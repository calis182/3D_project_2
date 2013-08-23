Texture2D<float4> inputMap : register(t0);
RWTexture2D<float4> outputMap : register(u0);

#define size_x 32
#define size_y 32

static const float filter[7][7] = {
	0.000904706, 0.003157733, 0.00668492, 0.008583607, 0.00668492, 0.003157733, 0.000904706,
	0.003157733, 0.01102157, 0.023332663, 0.029959733, 0.023332663, 0.01102157, 0.003157733,
	0.00668492, 0.023332663, 0.049395249, 0.063424755, 0.049395249, 0.023332663, 0.00668492,
	0.008583607, 0.029959733, 0.063424755, 0.081438997, 0.063424755, 0.029959733, 0.008583607,
	0.00668492, 0.023332663, 0.049395249, 0.063424755, 0.049395249, 0.023332663, 0.00668492,
	0.003157733, 0.01102157, 0.023332663, 0.029959733, 0.023332663, 0.01102157, 0.003157733,
	0.000904706, 0.003157733, 0.00668492, 0.008583607, 0.00668492, 0.003157733, 0.000904706
};

[numthreads(size_x, size_y, 1)]

void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	int3 textureLocation = dispatchThreadID - int3(3, 3, 0);

	float4 color = float4(0.0, 0.0, 0.0, 0.0);

	for(int x = 0; x < 7; x++)
	{
		for(int y = 0; y < 7; y++)
		{
			color += inputMap.Load(textureLocation + int3(x, y, 0)) * filter[x][y];
		}
	}

	outputMap[dispatchThreadID.xy] = color;
}

static const float filterX[7] = {
	0.030078323, 0.104983664, 0.222250419, 0.285375187, 0.222250419, 0.104983664, 0.030078323
};

#define width 1024

groupshared float4 horizontalpoints[3+width+3];
[numthreads(width, 1, 1)]

void CSMainX(uint3 groupID : SV_GroupID, uint3 dispatchThreadID : SV_DispatchThreadID, uint3 groupThreadID : SV_GroupThreadID, uint groupIndex : SV_GroupIndex)
{
	float4 data = inputMap.Load(dispatchThreadID);

	horizontalpoints[dispatchThreadID.x+3] = data;

	if(groupIndex == 0)
	{
		horizontalpoints[0] = inputMap.Load(dispatchThreadID - int3(3, 0, 0));
		horizontalpoints[1] = inputMap.Load(dispatchThreadID - int3(2, 0, 0));
		horizontalpoints[2] = inputMap.Load(dispatchThreadID - int3(1, 0, 0));
	}

	if(groupIndex == width-1)
	{
		horizontalpoints[3+width+0] = inputMap.Load(dispatchThreadID + int3(1, 0, 0));
		horizontalpoints[3+width+1] = inputMap.Load(dispatchThreadID + int3(2, 0, 0));
		horizontalpoints[3+width+2] = inputMap.Load(dispatchThreadID + int3(3, 0, 0));
	}

	GroupMemoryBarrierWithGroupSync();
	
	int textureLocation = groupThreadID.x;
	float4 color = float4(0.0, 0.0, 0.0, 0.0);

	for(int x = 0; x < 7; x++)
	{
		color += horizontalpoints[textureLocation + x] * filterX[x];
	}

	outputMap[dispatchThreadID.xy] = color;
}

#define height 768

groupshared float4 verticalpoints[3+height+3];
[numthreads(1, height, 1)]

void CSMainY(uint3 groupID : SV_GroupID, uint3 dispatchThreadID : SV_DispatchThreadID, uint3 groupThreadID : SV_GroupThreadID, uint groupIndex : SV_GroupIndex)
{
	float4 data = inputMap.Load(dispatchThreadID);

	verticalpoints[dispatchThreadID.y+3] = data;

	if(groupIndex == 0)
	{
		verticalpoints[0] = inputMap.Load(dispatchThreadID - int3(0, 3, 0));
		verticalpoints[1] = inputMap.Load(dispatchThreadID - int3(0, 2, 0));
		verticalpoints[2] = inputMap.Load(dispatchThreadID - int3(0, 1, 0));
	}

	if(groupIndex == height-1)
	{
		verticalpoints[3+height+0] = inputMap.Load(dispatchThreadID + int3(0, 1, 0));
		verticalpoints[3+height+1] = inputMap.Load(dispatchThreadID + int3(0, 2, 0));
		verticalpoints[3+height+2] = inputMap.Load(dispatchThreadID + int3(0, 3, 0));
	}

	GroupMemoryBarrierWithGroupSync();
	
	int textureLocation = groupThreadID.y;

	float4 color = float4(0.0, 0.0, 0.0, 0.0);

	for(int y = 0; y < 7; y++)
	{
		color += verticalpoints[textureLocation + y] * filterX[y];
	}

	outputMap[dispatchThreadID.xy] = color;
}