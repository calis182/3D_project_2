/////////////
// GLOBALS //
/////////////

cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

//////////////
// TYPEDEFS //
//////////////

struct VertexInputType
{
    float3 position : POSITION;
	float3 normal : NORMAL;
	float2 uv : UV;

	row_major float4x4 world : WORLD;
	uint instancedID : SV_InstanceID;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 normal : NORMALS;
    float2 tex : TEXCOORD0;
};

SamplerState SampleType;

Texture2D Texture;

//-----------------------------------------------------------------------------------------
// State Structures
//-----------------------------------------------------------------------------------------
RasterizerState NoCulling
{
	CullMode = NONE;
	FillMode = Solid;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType ParticleVertexShader(VertexInputType input)
{
    PixelInputType output = (PixelInputType)0;
    
    output.position = mul(float4(input.position, 1), input.world);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    output.tex = input.uv;

    output.normal = input.normal;

    return output;
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 ParticlePixelShader(PixelInputType input) : SV_TARGET
{
    return Texture.Sample(SampleType, input.tex);
}

technique11 BasicTech
{
    pass p0
    {
		// Set VS, GS, and PS
        SetVertexShader( CompileShader( vs_4_0, ParticleVertexShader() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, ParticlePixelShader() ) );
	    
	    SetRasterizerState( NoCulling );
    }  
}