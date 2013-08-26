
struct WaterData
{
	float height;
	float4 flow;
};

StructuredBuffer<WaterData> currentState : register(t0);
RWStructuredBuffer<WaterData> newState : register(u0);

cbuffer TimeParameters
{
	float4 timeFactors;
	float4 dispatchSize;
};

#define size_x 16
#define size_y 16

#define padded_x (1 + size_x + 1)
#define padded_y (1 + size_y + 1)

groupshared WaterData loadedPoints[padded_x * padded_y];

[numthreads(padded_x, padded_y, 1)]
void CSMain(uint3 groupID : SV_GroupID, uint3 dispatchThreadID : SV_DispatchThreadID, uint3 groupThreadID : SV_GroupThreadID, uint groupIndex : SV_GroupIndex)
{
	int gridSize_x = dispatchSize.x;
	int gridSize_y = dispatchSize.y;

	int totalSize_x = dispatchSize.z;
	int totalSize_y = dispatchSize.w;

	//Load in data
	loadedPoints[groupIndex].height = 0.0f;
	loadedPoints[groupIndex].flow = float4(0.0f, 0.0f, 0.0f, 0.0f);

	int3 location = int3( 0, 0, 0 );
	location.x = groupID.x * size_x + (groupThreadID.x - 1);
	location.y = groupID.y * size_y + (groupThreadID.y - 1);
	int textureIndex = location.x + location.y * totalSize_x;

	loadedPoints[groupIndex] = currentState[textureIndex];

	GroupMemoryBarrierWithGroupSync();

	float4 newFlow = float4(0.0f, 0.0f, 0.0f, 0.0f);

	//Check for not right edge
	if((groupThreadID.x < padded_x - 1) && (location.x < totalSize_x - 1))
	{
		newFlow.x = (loadedPoints[groupIndex+1].height - loadedPoints[groupIndex].height);
		
		//Check for not bottom edge
		if((groupThreadID.y < padded_y - 1) && (location.y < totalSize_y - 1))
		{
			newFlow.y = (loadedPoints[(groupIndex + 1) + padded_x].height - loadedPoints[groupIndex].height);
		}
	}

	//Check for not bottom edge
	if( ( groupThreadID.y < padded_y - 1 ) && ( location.y < totalSize_y - 1 ) )
	{
		newFlow.z = ( loadedPoints[groupIndex+padded_x].height - loadedPoints[groupIndex].height );

		// Check for 'not' left edge
		if ( ( groupThreadID.x > 0 ) && ( location.x > 0 ) )
		{
			newFlow.w = ( loadedPoints[groupIndex + padded_x - 1].height - loadedPoints[groupIndex].height );
		}
	}
	
	const float TIME_STEP = 0.1f;
	const float PIPE_AREA = 0.0001f;
	const float GRAVITATION = 10.0f;
	const float PIPE_LENGTH = 0.2f;
	const float FLUID_DENSITY = 1.0f;
	const float COLUMN_AREA = 0.05f;
	const float DAMPING_FACTOR = 1;//0.9999f;
	
	float accelFactor = (min(9999.0f, TIME_STEP) * PIPE_AREA * GRAVITATION) / (PIPE_LENGTH * COLUMN_AREA);
	
	newFlow = (newFlow * accelFactor + loadedPoints[groupIndex].flow) * DAMPING_FACTOR;
	
	loadedPoints[groupIndex].flow = newFlow;
	
	GroupMemoryBarrierWithGroupSync();
	
	//Calculate the new height values for each column, then store the height and modified flow values.
	
	//Out of the current column...
	loadedPoints[groupIndex].height = loadedPoints[groupIndex].height + newFlow.x + newFlow.y + newFlow.z + newFlow.w;
	
	//From left columns
	loadedPoints[groupIndex].height = loadedPoints[groupIndex].height - loadedPoints[groupIndex-1].flow.x;
	
	//From upper left columns
	loadedPoints[groupIndex].height = loadedPoints[groupIndex].height - loadedPoints[groupIndex-padded_x-1].flow.y;
	
	// From top columns
	loadedPoints[groupIndex].height = loadedPoints[groupIndex].height - loadedPoints[groupIndex-padded_x].flow.z;

	// From top right columns
	loadedPoints[groupIndex].height = loadedPoints[groupIndex].height - loadedPoints[groupIndex-padded_x+1].flow.w;

	if((groupThreadID.x > 0) && (groupThreadID.x < padded_x - 1) && (groupThreadID.y > 0) && (groupThreadID.y < padded_y - 1))
	{
		newState[textureIndex] = loadedPoints[groupIndex];
	}
}