
//-----------------------------------------------------------------------------------------
// Input and Output Structures
//-----------------------------------------------------------------------------------------

struct PointLight
{
	float4 ambient;
	float4 specular;
	float4 diffuse;

	float3 pos;
	float range;
};

cbuffer EveryFrame
{
	PointLight light;

	matrix gW;
	matrix gV;
	matrix gP;
	matrix gVP;
	matrix gWVP;

	float tessFactor;

	float4 eyePos;

	float4 frustrumPlaneEquation[4];
};

struct VSIn
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : UV;
};

struct VSOut
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : UV;
	float3 posW : POSITION1;
};

struct PSSceneIn
{
	float4 pos  : SV_Position;
	float3 normal : NORMAL;
	float2 uv : UV;
	float3 posW : POSITION;
};

struct HSDataOutput
{
	float edges[3] : SV_TessFactor;
	float inside : SV_InsideTessFactor;
};

Texture2D gTexture1;
Texture2D gTexture2;
Texture2D gTexture3;
Texture2D gBlendMap;
Texture2D gHeightMap;

SamplerState ss
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 1;
	AddressU = WRAP;
	AddressV = WRAP;
};

//-----------------------------------------------------------------------------------------
// State Structures
//-----------------------------------------------------------------------------------------
RasterizerState NoCulling
{
	CullMode = none;
	//FillMode = wireframe;
};

//-----------------------------------------------------------------------------------------
// VertexShader: VSScene
//-----------------------------------------------------------------------------------------
VSOut VSScene(VSIn input)
{
	VSOut output = (VSOut)0;

	output.pos = mul(float4(input.pos, 1), gW).xyz;
	//output.pos = mul(float4(output.pos.xyz, 1), gV);
	//output.pos = mul(float4(output.pos.xyz, 1), gP);

	output.uv = input.uv;
	output.normal = (mul(float4(input.normal, 1), gW)).xyz;

	output.posW = mul(float4(input.pos, 1), gW).xyz;

	return output;
}

float distanceFromPlane(float3 f3Position, float4 f4PlaneEquation)
{
	float distance = dot(float4(f3Position, 1.0f), f4PlaneEquation);
	return distance;
}

//-----------------------------------------------------------------------------------------
// ConstantHullShader: ConstantHS
//-----------------------------------------------------------------------------------------

HSDataOutput ConstantHS(InputPatch<VSOut, 3> ip, uint PatchID : SV_PrimitiveID)
{
	HSDataOutput output = (HSDataOutput)0;

	//	Back face culling, not very usable for the terrain.
	/*
	float3 edge0 = ip[1].pos - ip[0].pos;
	float3 edge1 = ip[2].pos - ip[0].pos;

	float3 faceNormal = normalize(cross(edge1, edge0));
	float3 view = normalize(ip[0].pos - eyePos.xyz);

	if(dot(view, faceNormal) > 0.10)
	{
		for(int i = 0; i < 3; i++)
			output.edges[i] = 0;
		output.inside = 0;
		return output;
	}
	*/
	for(int i = 0; i < 3; i++)
		output.edges[i] = tessFactor;
	output.inside = tessFactor;


	/****************************************************
					Frustrum culling
	****************************************************/
	bool viewFrustrumCull = true;

	float f4PlaneTest[4];
	float heightScale = -10.0f;

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

//-----------------------------------------------------------------------------------------
// HullShader: HS
//-----------------------------------------------------------------------------------------

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
	output.normal = p[i].normal;
	output.uv = p[i].uv;
	output.posW = p[i].posW;

	return output;
}

//-----------------------------------------------------------------------------------------
// DomainShader: DS
//-----------------------------------------------------------------------------------------

[domain("tri")]
PSSceneIn DS(HSDataOutput input, float3 UVW : SV_DomainLocation, const OutputPatch<VSOut, 3> tri)
{
	PSSceneIn output = (PSSceneIn)0;

	float3 finalPos = UVW.x * tri[0].pos
					+ UVW.y * tri[1].pos
					+ UVW.z * tri[2].pos;

	output.posW = UVW.x * tri[0].posW
				+ UVW.y * tri[1].posW
				+ UVW.z * tri[2].posW;

	output.normal = UVW.x * tri[0].normal
				  + UVW.y * tri[1].normal
				  + UVW.z * tri[2].normal;
	output.normal = normalize(output.normal);

	output.uv = UVW.x * tri[0].uv
			  + UVW.y * tri[1].uv
			  + UVW.z * tri[2].uv;

	float mipLevel = clamp((distance(finalPos, eyePos.xyz) - 100.0f) / 30.0f, 0.0f, 3.0f);
	float h = gHeightMap.SampleLevel(ss, output.uv, mipLevel).r;

	finalPos = finalPos + output.normal * h * 0.5 * 128;

	output.pos = mul(float4(finalPos, 1), gVP);

	return output;
}

//-----------------------------------------------------------------------------------------
// PixelShader: PSSceneMain
//-----------------------------------------------------------------------------------------
float4 PSScene(PSSceneIn input) : SV_Target
{
	//Blendmap
	int repeat = 20;
	float4 c0 = gTexture2.Sample(ss, input.uv*repeat);
	float4 c1 = gTexture1.Sample(ss, input.uv*repeat);
	float4 c2 = gTexture2.Sample(ss, input.uv*repeat);
	float4 c3 = gTexture3.Sample(ss, input.uv*repeat);
	//float4 c4 = gBlendMap.Sample(ss, input.uv*repeat);

	float4 t = gBlendMap.Sample(ss, input.uv);

	float4 texColor = c0;
	texColor = lerp(texColor, c1, t.r);
	texColor = lerp(texColor, c2, t.g);
	texColor = lerp(texColor, c3, t.b);
	//texColor = lerp(texColor, c4, t.a);

	//Pointlight
	float3 toEye = normalize(eyePos.xyz - input.posW);

	float3 lightVec = light.pos - input.posW;
	float d = length(lightVec);

	if(d > light.range)
		return texColor;

	lightVec /= d;

	float diffuseFactor = dot(lightVec, input.normal);

	float4 diffuse;
	float4 spec;

	if(diffuseFactor > 0.0f)
	{
		float3 v = reflect(-lightVec, input.normal);
		float specFactor = (max(dot(v, toEye), 0.0f));

		diffuse = diffuseFactor * light.diffuse;
		spec = specFactor * light.specular;
	}

	texColor += diffuse + light.ambient + spec;

	return texColor;
}

//-----------------------------------------------------------------------------------------
// Technique: RenderTextured  
//-----------------------------------------------------------------------------------------
technique11 BasicTech
{
    pass p0
    {
		// Set VS, GS, and PS
        SetVertexShader(CompileShader(vs_5_0, VSScene()));
		SetHullShader(CompileShader(hs_5_0, HS()));
		SetDomainShader(CompileShader(ds_5_0, DS()));
		SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PSScene()));
	    
	    SetRasterizerState( NoCulling );
    }  
}
