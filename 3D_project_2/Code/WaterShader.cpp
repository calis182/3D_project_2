#include "WaterShader.h"

WaterShader::WaterShader()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_sampleState = 0;

}

WaterShader::WaterShader(const WaterShader& other)
{
}

WaterShader::~WaterShader()
{
}

bool WaterShader::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd)
{

	Vertex waterModel[] = {
	{D3DXVECTOR3(-128, 5, 128), D3DXVECTOR3(0, 1, 0), D3DXVECTOR2(0, 0)},
	{D3DXVECTOR3(128, 5, -128), D3DXVECTOR3(0, 1, 0), D3DXVECTOR2(1, 1)},
	{D3DXVECTOR3(-128, 5, -128), D3DXVECTOR3(0, 1, 0), D3DXVECTOR2(0,1)},
		
	{D3DXVECTOR3(128, 5, 128), D3DXVECTOR3(0, 1, 0), D3DXVECTOR2(1, 0)},
	{D3DXVECTOR3(128, 5, -128), D3DXVECTOR3(0, 1, 0), D3DXVECTOR2(1,1)},
	{D3DXVECTOR3(-128, 5, 128), D3DXVECTOR3(0, 1, 0), D3DXVECTOR2(0,0)}
	};
	
	BUFFER_INIT_DESC waterDesc;
	waterDesc.ElementSize = sizeof(Vertex);
	waterDesc.InitData = waterModel;
	waterDesc.NumElements = 6;
	waterDesc.Type = VERTEX_BUFFER;
	waterDesc.Usage = BUFFER_DEFAULT;

	refraction = new Buffer();
	if(FAILED(refraction->Init(device, deviceContext, waterDesc)))
	{
		return false;
	}
	
	reflection = new Buffer();
	if(FAILED(reflection->Init(device, deviceContext, waterDesc)))
	{
		return false;
	}

	water = new Buffer();
	if(FAILED(water->Init(device, deviceContext, waterDesc)))
	{
		return false;
	}
	return true;
}

void WaterShader::Shutdown()
{
	ShutdownShader();
}

bool WaterShader::InitializeShader(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	refractionShader = new Shader();
	if(FAILED(refractionShader->Init(device, deviceContext, "../Shaders/Refraction.fx", inputDesc,3)))
	{
		return false;
	}

	reflectionShader = new Shader();
	if(FAILED(reflectionShader->Init(device, deviceContext, "../Shaders/Reflection.fx", inputDesc, 3)))
	{
		return false;
	}

	D3D11_INPUT_ELEMENT_DESC waterInputDesc[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	waterShader = new Shader();
	if(FAILED(waterShader->Init(device, deviceContext, "../Shaders/Water.fx", waterInputDesc, 2)))
	{
		return false;
	}

		
	normalMap = new TextureClass();
	normalMap->init(device, "..\\Shaders\\water01.dds");



	return true;
}

void WaterShader::ShutdownShader()
{
	
	// Release the sampler state.
	if(m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = 0;
	}

	// Release the layout.
	if(m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	// Release the pixel shader.
	if(m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	// Release the vertex shader.
	if(m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}

	if(refraction)
	{
		delete refraction;
		refraction = NULL;
	}

	if(reflection)
	{
		delete reflection;
		reflection = NULL;
	}

	if(water)
	{
		delete water;
		water = NULL;
	}

	if(refractionShader)
	{
		 delete refractionShader;
		 refractionShader = NULL;
	}

	if(reflectionShader)
	{
		delete reflectionShader;
		reflectionShader = NULL;
	}

	if(waterShader)
	{
		delete waterShader;
		waterShader = NULL;
	}

	delete normalMap;
	normalMap = NULL;

}

void WaterShader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, char* shaderFilename)
{
	char* compileErrors;
	unsigned long bufferSize, i;
	ofstream fout;

	//Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	//Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	//Open a file to write the to.
	fout.open("shader-error.txt");

	//Wtite out the error message.
	for(i = 0; i < bufferSize; i++)
	{
		fout<<compileErrors[i];
	}

	//close the file.
	fout.close();

	//Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	//Pop a message up on the to notify the user to check the text file for complie errors.
	MessageBox(hwnd, "Error compiling shader. Check shader-error.tx for message.", shaderFilename, MB_OK);
}

bool WaterShader::SetRefractionParameters(D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX proj, D3DXVECTOR4 clipPlane, D3DXVECTOR4 ambient, D3DXVECTOR4 diffuse, D3DXVECTOR3 lightPos)
{
	this->world = world;
	this->view = view;
	this->proj = proj;
	this->clipPlane = clipPlane;
	this->ambient = ambient;
	this->diffuse = diffuse;
	this->lightPos = lightPos;
	return true;
}

bool WaterShader::SetReflectionParameters(D3DXMATRIX world, D3DXMATRIX reflectionMatrix, D3DXMATRIX proj, D3DXVECTOR4 ambient, D3DXVECTOR4 diffuse, D3DXVECTOR3 lightPos)
{
	this->world = world;
	this->reflectionMatrix = reflectionMatrix;
	this->proj = proj;
	this->ambient = ambient;
	this->diffuse = diffuse;
	this->lightPos = lightPos;
	return true;
}

bool WaterShader::SetWaterParameters(D3DXMATRIX world, D3DXMATRIX reflectionMatrix, D3DXMATRIX view, D3DXMATRIX proj, D3DXVECTOR4 ambient, D3DXVECTOR4 diffuse, D3DXVECTOR3 lightPos, float waterTranslation, float reflectRefractScale)
{
	this->world = world;
	this->reflectionMatrix = reflectionMatrix;
	this->view = view;
	this->proj = proj;
	this->ambient = ambient;
	this->diffuse = diffuse;
	this->lightPos = lightPos;
	this->waterTranslation = waterTranslation;
	this->reflectRefractScale = reflectRefractScale;
	return true;
}

void WaterShader::RenderRefraction(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* texture)
{
	
	D3DXVECTOR4 lightPos4 = D3DXVECTOR4(lightPos,0);
	refractionShader->SetResource("shaderTexture", texture);
	refractionShader->SetMatrix("worldMatrix", world);
	refractionShader->SetMatrix("viewMatrix", view);
	refractionShader->SetMatrix("projectionMatrix", proj);
	refractionShader->SetFloat4("clipPlane", clipPlane);
	refractionShader->SetFloat4("ambientColor", ambient);
	refractionShader->SetFloat4("diffuseColor", diffuse);
	refractionShader->SetFloat4("lightPos", lightPos4);


	refraction->Apply(0);
	refractionShader->Apply(0);
	//Render the triangles.
	deviceContext->Draw(6, 0);
}

void WaterShader::RenderReflection(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* texture)
{
	
	D3DXVECTOR4 lightPos4 = D3DXVECTOR4(lightPos,0);
	reflectionShader->SetResource("shaderTexture", texture);
	reflectionShader->SetMatrix("worldMatrix", world);
	reflectionShader->SetMatrix("reflectionViewMatrix", reflectionMatrix);
	reflectionShader->SetMatrix("projectionMatrix", proj);
	reflectionShader->SetFloat4("ambientColor", ambient);
	reflectionShader->SetFloat4("diffuseColor", diffuse);
	reflectionShader->SetFloat4("lightPos", lightPos4);

	
	reflection->Apply(0);
	reflectionShader->Apply(0);
	deviceContext->Draw(6, 0);

}
void WaterShader::RenderWater(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* refractionTexture, ID3D11ShaderResourceView* reflectionTexture)
{

	D3DXVECTOR4 lightPos4 = D3DXVECTOR4(lightPos,0);
	waterShader->SetResource("normalTexture", normalMap->getTexture());
	waterShader->SetResource("refractionTexture", refractionTexture);
	waterShader->SetResource("reflectionTexture", reflectionTexture);
	waterShader->SetMatrix("worldMatrix", world);
	waterShader->SetMatrix("reflectionMatrix", reflectionMatrix);
	waterShader->SetMatrix("projectionMatrix", proj);
	waterShader->SetMatrix("viewMatrix", view);
	waterShader->SetFloat("waterTranslation", waterTranslation);
	waterShader->SetFloat("reflectRefractScale", reflectRefractScale);

	
	water->Apply(0);
	waterShader->Apply(0);
	

	// Render the triangles.
	deviceContext->Draw(6, 0);
}
