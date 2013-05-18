//--------------------------------------------------------------------------------------
// Basic.fx
// Direct3D 11 Shader Model 4.0 Demo
// Copyright (c) Stefan Petersson, 2011
//--------------------------------------------------------------------------------------

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

	float4 eyePos;
};

struct VSIn
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : UV;
};

struct PSSceneIn
{
	float4 pos  : SV_Position;
	float3 normal : NORMAL;
	float2 uv : UV;
	float3 posW : POSITION;
};

SamplerState ss
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 1;
	AddressU = WRAP;
	AddressV = WRAP;
};

Texture2D texture1;

//-----------------------------------------------------------------------------------------
// State Structures
//-----------------------------------------------------------------------------------------
RasterizerState NoCulling
{
	CullMode = NONE;
	//FillMode = wireframe;
};

//-----------------------------------------------------------------------------------------
// VertexShader: VSScene
//-----------------------------------------------------------------------------------------
PSSceneIn VSScene(VSIn input)
{
	PSSceneIn output = (PSSceneIn)0;

	output.pos = mul(float4(input.pos, 1), gW);
	output.pos = mul(float4(output.pos.xyz, 1), gV);
	output.pos = mul(float4(output.pos.xyz, 1), gP);

	output.posW = mul(float4(input.pos, 1), gW).xyz;
	
	output.uv = input.uv;

	output.normal = mul(float4(input.normal, 1), gW).xyz;

	return output;
}

[maxvertexcount(3)]
void GSScene(triangle PSSceneIn input[3], inout TriangleStream<PSSceneIn> OutputStream)
{

	float3 normalTri;

	normalTri = normalize((input[0].normal + input[1].normal +input[2].normal)/3);

	float3 toEye = normalize(eyePos.xyz - input[0].posW);
	float angle = acos(toEye*normalTri);

	if(angle < 3.14 && angle > 0)
	{
		for(int i = 0; i<3; i++)
		{
		
			OutputStream.Append( input[i] );
		
		}
	}
}

//-----------------------------------------------------------------------------------------
// PixelShader: PSSceneMain
//-----------------------------------------------------------------------------------------
float4 PSScene(PSSceneIn input) : SV_Target
{
	float4 textureColor = texture1.Sample(ss, input.uv);

	float3 toEye = normalize(eyePos.xyz - input.posW);

	float3 lightVec = light.pos - input.posW;
	float d = length(lightVec);

	if(d > light.range)
		return textureColor;

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

	textureColor += diffuse + light.ambient + spec;

	return textureColor;
}

//-----------------------------------------------------------------------------------------
// Technique: RenderTextured  
//-----------------------------------------------------------------------------------------
technique11 BasicTech
{
    pass p0
    {
		// Set VS, GS, and PS
        SetVertexShader( CompileShader( vs_4_0, VSScene() ) );
		SetGeometryShader( NULL);
		SetGeometryShader( CompileShader( gs_4_0, GSScene()));
        SetPixelShader( CompileShader( ps_4_0, PSScene() ) );
	    
	    SetRasterizerState( NoCulling );
    }  
}