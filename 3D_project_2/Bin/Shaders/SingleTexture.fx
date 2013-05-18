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
	float3 normal : NORMAL;
	float2 uv : UV;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 normal : NORMALS;
    float2 tex : TEXCOORD0;
};

SamplerState SampleType
{
	AddressU	= WRAP;
	AddressV = WRAP;
};

Texture2D Texture;

//-----------------------------------------------------------------------------------------
// State Structures
//-----------------------------------------------------------------------------------------
RasterizerState NoCulling
{
	CullMode = NONE;
	//FillMode = wireframe;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType ParticleVertexShader(VertexInputType input)
{
    PixelInputType output = (PixelInputType)0;
    
    output.position = mul(float4(input.position, 1), worldMatrix);
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
    float4 finalColor = Texture.Sample(SampleType, input.tex);//float4(input.color, 1);
    return finalColor;
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