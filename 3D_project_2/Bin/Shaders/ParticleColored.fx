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

//The two input types both have a color component so that the particle can have an individual color that is added to the texture base color.
struct VertexInputType
{
    float3 position : POSITION;
	float3 color : COLOR;
	float2 uv : TEX;

	row_major float4x4 world : WORLD;
	uint instancedID : SV_InstanceID;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 color : COLORS;
    float2 tex : TEXCOORD0;
};

SamplerState SampleType;

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

    output.color = input.color;

    return output;
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 ParticlePixelShader(PixelInputType input) : SV_TARGET
{
    return float4(input.color, 1);
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