//
//	Simple texture shader
//

cbuffer EveryFrame
{
	matrix gWVP;
	matrix gVP;
	matrix gW;

	float tessFactor;
	
	float4 frustrumPlaneEquation[4];
	float dispatchSize;
};

struct WaterData
{
	float height;
	float4 flow;
};

struct VSIn
{
	float3 pos : POSITION;
	float2 uv : UV;
};

struct PSIn
{
	float4 pos : SV_POSITION;
	float3 posW : WORLD_POS;
	float2 uv : UV;
};

struct VSOut
{
	float4 pos : POSITION;
	float2 uv : UV;
};

struct HSDataOutput
{
	float edges[3] : SV_TessFactor;
	float inside : SV_InsideTessFactor;
};

StructuredBuffer<WaterData> currentState : register(t0);

Texture2D waterTexture;

SamplerState ss
{
	AddressU = WRAP;
	AddressV = WRAP;
};

RasterizerState NoCulling
{
	CullMode = NONE;
	FillMode = solid;
};

VSOut VSScene(VSIn input)
{
	VSOut output;

	output.pos = mul(float4(input.pos, 1), gW);

	output.uv = input.uv;

	return output;
}

float distanceFromPlane(float3 f3Position, float4 f4PlaneEquation)
{
	float distance = dot(float4(f3Position, 1.0f), f4PlaneEquation);
	return distance;
}

HSDataOutput ConstantHS(InputPatch<VSOut, 3> ip, uint PatchID : SV_PrimitiveID)
{
	HSDataOutput output = (HSDataOutput)0;

	output.edges[0] = tessFactor;
	output.edges[1] = tessFactor;
	output.edges[2] = tessFactor;
	output.inside = tessFactor;

	bool viewFrustrumCull = true;

	float f4PlaneTest[4];
	float heightScale = 0.0f;

	for(int j = 0; j < 4; j++)
	{
		f4PlaneTest[j] = ((distanceFromPlane(ip[0].pos, frustrumPlaneEquation[j]) > -heightScale) ? 1.0f : 0.0f) + 
						((distanceFromPlane(ip[1].pos, frustrumPlaneEquation[j]) > -heightScale) ? 1.0f : 0.0f) + 
						((distanceFromPlane(ip[2].pos, frustrumPlaneEquation[j]) > -heightScale) ? 1.0f : 0.0f);
		if(f4PlaneTest[j] > 0.0f)
		{
			viewFrustrumCull = false;
			break;
		}
	}

	if(viewFrustrumCull)
	{
		output.edges[0] = 0.0f;
		output.edges[1] = 0.0f;
		output.edges[2] = 0.0f;
		output.inside = 0.0f;
	}

	return output;
}

[domain("tri")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
VSOut HS(InputPatch<VSOut, 3> p, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID)
{
	VSOut output = (VSOut)0;

	output.pos = p[i].pos;
	output.uv = p[i].uv;
	
	return output;
}

float3 Sobel(float2 tc)
{
	// Useful aliases
	float2 pxSz = float2( 1.0f, 1.0f);

	// Compute the necessary offsets:
	float2 o00 = tc + float2( -pxSz.x, -pxSz.y );
	float2 o10 = tc + float2(    0.0f, -pxSz.y );
	float2 o20 = tc + float2(  pxSz.x, -pxSz.y );

	float2 o01 = tc + float2( -pxSz.x, 0.0f    );
	float2 o21 = tc + float2(  pxSz.x, 0.0f    );

	float2 o02 = tc + float2( -pxSz.x,  pxSz.y );
	float2 o12 = tc + float2(    0.0f,  pxSz.y );
	float2 o22 = tc + float2(  pxSz.x,  pxSz.y );

	int t00 = floor(o00.x) + floor(o00.y) * dispatchSize;
	int t10 = floor(o10.x) + floor(o10.y) * dispatchSize;
	int t20 = floor(o20.x) + floor(o20.y) * dispatchSize;

	int t01 = floor(o01.x) + floor(o01.y) * dispatchSize;
	int t21 = floor(o21.x) + floor(o21.y) * dispatchSize;

	int t02 = floor(o02.x) + floor(o02.y) * dispatchSize;
	int t12 = floor(o12.x) + floor(o12.y) * dispatchSize;
	int t22 = floor(o22.x) + floor(o22.y) * dispatchSize;

	// Use of the sobel filter requires the eight samples
	// surrounding the current pixel:
	float h00 = currentState[t00].height;
	float h10 = currentState[t10].height;
	float h20 = currentState[t20].height;

	float h01 = currentState[t01].height;
	float h21 = currentState[t21].height;

	float h02 = currentState[t02].height;
	float h12 = currentState[t12].height;
	float h22 = currentState[t22].height;

	// Evaluate the Sobel filters
	float Gx = h00 - h20 + 2.0f * h01 - 2.0f * h21 + h02 - h22;
	float Gy = h00 + 2.0f * h10 + h20 - h02 - 2.0f * h12 - h22;

	// Generate the missing Z
	float Gz = 0.01f * sqrt( max(0.0f, 1.0f - Gx * Gx - Gy * Gy ) );

	// Make sure the returned normal is of unit length
	return normalize( float3( 2.0f * Gx, Gz, 2.0f * Gy ) );
}

[domain("tri")]
PSIn DS(HSDataOutput input, float3 UVW : SV_DomainLocation, const OutputPatch<VSOut, 3> tri)
{
	PSIn output = (PSIn)0;

	float3 finalPos = UVW.x * tri[0].pos
					+ UVW.y * tri[1].pos
					+ UVW.z * tri[2].pos;

	output.posW = finalPos;

	float2 uv = UVW.x * tri[0].uv
			  + UVW.y * tri[1].uv
			  + UVW.z * tri[2].uv;

	int3 coords = int3(floor(uv.x), floor(uv.y), 0);
	int textureIndex = coords.x + coords.y * dispatchSize;

	finalPos.y += currentState[textureIndex].height;

	output.pos = mul(float4(finalPos, 1), gVP);

	output.uv = uv;

	return output;
}

float4 PSScene(PSIn input) : SV_Target
{
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 normal = Sobel(input.uv);

	float3 toEye = normalize(float3(gVP._41, gVP._42, gVP._43) - input.posW);

	float3 lightVec = -float3(0.0f, 0.0f, -1.0f);

	ambient = float4(1, 1, 1, 1);

	float diffuseFactor = dot(lightVec, normal);

	[flatten]
	if(diffuseFactor > 0.0f)
	{
		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), 0.5f);

		diffuse = diffuseFactor * float4(0.5f, 0.5f, 0.5f, 0.5f);
		specular = specFactor * float4(0.5f, 0.5f, 0.5f, 0.5f);
	}

	float4 textureColor = waterTexture.Sample(ss, input.uv/dispatchSize);

	//textureColor = textureColor * (ambient + diffuse) + specular;
	
	return textureColor;
	return float4(normal, 1);
}

technique11 BasicTech
{
	pass p0
	{
		SetVertexShader(CompileShader(vs_5_0, VSScene()));
		SetHullShader(CompileShader(hs_5_0, HS()));
		SetDomainShader(CompileShader(ds_5_0, DS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSScene()));

		SetRasterizerState(NoCulling);
	}
}