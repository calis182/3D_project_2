/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

Texture2D shaderTexture;
SamplerState SampleType;

cbuffer LightBuffer
{
    float4 ambientColor;
    float4 diffuseColor;
	float4 lightPos;
};

cbuffer ClipPlaneBuffer
{
	float4 clipPlane;
};

//-----------------------------------------------------------------------------------------
// State Structures
//-----------------------------------------------------------------------------------------
RasterizerState NoCulling
{
	CullMode = NONE;
	//FillMode = wireframe;
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
	float3 normal : NORMAL;
	float2 tex : TEXCOORD;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float clip : SV_ClipDistance0;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType RefractionVertexShader(VertexInputType input)
{
	PixelInputType output;

	//Change the position vector to be 4 units for proper matrix calculations.
	input.position.w = 1.0f;
	
	//Calculate the position of the vertex against the world, view and projection matrices.
	output.position = mul(input.position, worldMatrix);
	output.position = mul(input.position, viewMatrix);
	output.position = mul(input.position, projectionMatrix);

	//Strore the texture coordinates for the pixel shader.
	output.tex = input.tex;

	//Calculate the normal vector against the world matrix only.
	output.normal = mul(input.normal, (float3x3)worldMatrix);

	//Normalize the normal vector.
	output.normal = normalize(output.normal);

	//Set the clipping plane.
	output.clip = dot(mul(input.position, worldMatrix), clipPlane);

	return output;
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 RefractionPixelShader(PixelInputType input) : SV_TARGET
{
	float4 textureColor;
	float3 lightVec; 
	float lightIntensity;
	float4 color;

	//Sample the texture pixel at this location.
	textureColor = shaderTexture.Sample(SampleType, input.tex);

	//Set the default output color to the ambient light value for all pixels.
	color = ambientColor;

	lightVec = lightPos - input.position.w;
	float d = length(lightVec);
	lightVec /= d;

	//Caluculate the amount of light on this pixel.
	lightIntensity = saturate(dot(input.normal, lightVec));

	if(lightIntensity > 0.0f)
	{
		//Determine the final diffuse color based on the diffuse color and the amount of light intentsity.
		color += (diffuseColor * lightIntensity);
	}

	//Sature the final light color.
	color = saturate(color);

	//Multiply the texture pixel and the input color to get the final result.
	color = color * textureColor;

	return color;
}

//-----------------------------------------------------------------------------------------
// Technique: RenderTextured  
//-----------------------------------------------------------------------------------------
technique11 BasicTech
{
    pass p0
    {
	// Set VS and PS
        SetVertexShader(CompileShader(vs_4_0, RefractionVertexShader()));
        SetPixelShader(CompileShader(ps_4_0, RefractionPixelShader()));
	    
	SetRasterizerState( NoCulling );
    }  
}