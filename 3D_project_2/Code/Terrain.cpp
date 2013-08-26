#include "Terrain.h"

Terrain::Terrain()
{
	mesh = NULL;
	index = NULL;

	width = height = 0;
	vertexCount = indexCount = 0;
}

Terrain::~Terrain()
{
	delete heightMap;
	delete mesh;
	delete index;
	delete fence;
	texture1.shutdown();
	texture2.shutdown();
	texture3.shutdown();
	blendMap.shutdown();
	texture4.shutdown();

	delete g_Shader;
	delete g_Fence;
}

HRESULT Terrain::initShader(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	g_Shader = new Shader();
	if(FAILED(g_Shader->Init(device, deviceContext, "../Shaders/Terrain Tessellation.fx", inputDesc, 2)))
	{
		return E_FAIL;
	}

	D3D11_INPUT_ELEMENT_DESC inputDesc2[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	g_Fence = new Shader();
	if(FAILED(g_Fence->Init(device, deviceContext, "../Shaders/SingleTexture.fx", inputDesc2, 3)))
	{
		return E_FAIL;
	}

	texture1.init(device, "..\\Shaders\\stone.jpg");
	texture2.init(device, "..\\Shaders\\grassafterrain.jpg");
	texture3.init(device, "..\\Shaders\\brickwork-texture.jpg");
	blendMap.init(device, "..\\Shaders\\blendmap.jpg");
	texture4.init(device, "..\\Shaders\\wire.png");
	hm.init(device, "..\\Shaders\\hm.png");

	return S_OK;
}

bool Terrain::init(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	BUFFER_INIT_DESC vertexBufferDesc, indexBufferDesc;
	Vertex2* vertices = NULL;
	unsigned long* indices = NULL;

	width = 256;
	height = 256;
	numOfStartingVertex = 8;

	initShader(device, deviceContext);

	vertexCount = numOfStartingVertex*numOfStartingVertex;
	indexCount = (numOfStartingVertex-1) * (numOfStartingVertex-1) * 6;

	vertices = new Vertex2[vertexCount];
	if(!vertices)
		return false;

	indices = new unsigned long[indexCount];
	if(!indices)
		return false;

	unsigned long ind = 0;

	heightMap = new HeightMap();
	heightMap->loadRaw(width,height, "../Shaders/hm.raw", 0.3f, 0);

	float repeat = 1.0f;

	float x = 0, y = 0, z = 0;
	float halfWidth = -(width*0.5f), halfHeight = (height*0.5f);
	float triangleSize = (float)width / (numOfStartingVertex-1);
	float uvSize = 1.0f / (numOfStartingVertex-1);
	for(int i = 0; i < numOfStartingVertex; i++)
	{
		x = halfWidth+i*triangleSize;
		for(int j = 0; j < numOfStartingVertex; j++)
		{
			int in = i * numOfStartingVertex + j;
			z = halfHeight-j*triangleSize;
			//y = heightMap->getData(x + 128, z + 128);
			vertices[in].pos = D3DXVECTOR3(x, y, z);
			//vertices[in].normal = D3DXVECTOR3(0,1,0);
			//i/(width/repeat),j/(height/repeat));
			vertices[in].uv = D3DXVECTOR2(i * uvSize, j * uvSize);
		}
	}

	D3DXVECTOR3 vec1, vec2, vec3, vec4, vec5;

	ind = 0;
	for(int i = 0; i < numOfStartingVertex-1; i++)
	{
		for(int j = 0; j < numOfStartingVertex-1; j++)
		{
			indices[ind] = i * numOfStartingVertex + j;
			indices[ind+1] = i * numOfStartingVertex + j+1;
			indices[ind+2] = (i+1) * numOfStartingVertex + j+1;

			indices[ind+3] = i * numOfStartingVertex + j;
			indices[ind+4] = (i+1) * numOfStartingVertex + j+1;
			indices[ind+5] = (i+1) * numOfStartingVertex + j;

			ind += 6;
		}
	}

	vertexBufferDesc.ElementSize = sizeof(Vertex2);
	vertexBufferDesc.InitData = vertices;
	vertexBufferDesc.NumElements = vertexCount;
	vertexBufferDesc.Type = VERTEX_BUFFER;
	vertexBufferDesc.Usage = BUFFER_DEFAULT;


	mesh = new Buffer();
	if(FAILED(mesh->Init(device, deviceContext, vertexBufferDesc)))
	{
		return false;
	}

	indexBufferDesc.ElementSize = sizeof(unsigned long);
	indexBufferDesc.InitData = indices;
	indexBufferDesc.NumElements = indexCount;
	indexBufferDesc.Type = INDEX_BUFFER;
	indexBufferDesc.Usage = BUFFER_DEFAULT;

	index = new Buffer();
	if(FAILED(index->Init(device, deviceContext, indexBufferDesc)))
	{
		return false;
	}
	float fenceRepeat = 16;
	Vertex mesh2[] = {
	{D3DXVECTOR3(-128, 0, -128),D3DXVECTOR3(1, 0, 1), D3DXVECTOR2(0, fenceRepeat/4)},
	{D3DXVECTOR3(-128, 64, -128),D3DXVECTOR3(1, 0, 1), D3DXVECTOR2(0,0)},
	{D3DXVECTOR3(-128, 0, 128),D3DXVECTOR3(1, 0, 1),D3DXVECTOR2(fenceRepeat,fenceRepeat/4)},
		
	{D3DXVECTOR3(-128, 64, -128),D3DXVECTOR3(1, 0, 1),D3DXVECTOR2(0, 0)},
	{D3DXVECTOR3(-128, 64, 128),D3DXVECTOR3(1, 0, 1),D3DXVECTOR2(fenceRepeat,0)},
	{D3DXVECTOR3(-128, 0, 128),D3DXVECTOR3(1,0, 1),D3DXVECTOR2(fenceRepeat,fenceRepeat/4)},

	{D3DXVECTOR3(-128, 0, 128),D3DXVECTOR3(0, 0, -1), D3DXVECTOR2(0, fenceRepeat/4)},
	{D3DXVECTOR3(-128, 64, 128),D3DXVECTOR3(0, 0, -1), D3DXVECTOR2(0,0)},
	{D3DXVECTOR3(128, 0, 128),D3DXVECTOR3(0, 0, -1),D3DXVECTOR2(fenceRepeat,fenceRepeat/4)},

	{D3DXVECTOR3(-128, 64, 128),D3DXVECTOR3(0, 0, -1),D3DXVECTOR2(0, 0)},
	{D3DXVECTOR3(128, 64, 128),D3DXVECTOR3(0, 0, -1),D3DXVECTOR2(fenceRepeat,0)},
	{D3DXVECTOR3(128, 0, 128),D3DXVECTOR3(0, 0, -1),D3DXVECTOR2(fenceRepeat,fenceRepeat/4)},

	{D3DXVECTOR3(128, 0, 128),D3DXVECTOR3(-1, 0, 0), D3DXVECTOR2(0, fenceRepeat/4)},
	{D3DXVECTOR3(128, 64, 128),D3DXVECTOR3(-1, 0, 0), D3DXVECTOR2(0,0)},
	{D3DXVECTOR3(128, 0, -128),D3DXVECTOR3(-1, 0, 0),D3DXVECTOR2(fenceRepeat,fenceRepeat/4)},

	{D3DXVECTOR3(128, 64, 128),D3DXVECTOR3(-1, 0, 0),D3DXVECTOR2(0, 0)},
	{D3DXVECTOR3(128, 64, -128),D3DXVECTOR3(-1, 0, 0),D3DXVECTOR2(fenceRepeat,0)},
	{D3DXVECTOR3(128, 0, -128),D3DXVECTOR3(-1, 0, 0),D3DXVECTOR2(fenceRepeat,fenceRepeat/4)},

	{D3DXVECTOR3(128, 0, -128),D3DXVECTOR3(-1, 0, 0), D3DXVECTOR2(0, fenceRepeat/4)},
	{D3DXVECTOR3(128, 64, -128),D3DXVECTOR3(-1, 0, 0), D3DXVECTOR2(0,0)},
	{D3DXVECTOR3(-128, 0, -128),D3DXVECTOR3(-1, 0, 0),D3DXVECTOR2(fenceRepeat,fenceRepeat/4)},

	{D3DXVECTOR3(128, 64, -128),D3DXVECTOR3(-1, 0, 0),D3DXVECTOR2(0, 0)},
	{D3DXVECTOR3(-128, 64, -128),D3DXVECTOR3(-1, 0, 0),D3DXVECTOR2(fenceRepeat,0)},
	{D3DXVECTOR3(-128, 0, -128),D3DXVECTOR3(-1, 0, 0),D3DXVECTOR2(fenceRepeat,fenceRepeat/4)},
	};


	BUFFER_INIT_DESC bufferDesc;
	bufferDesc.ElementSize = sizeof(Vertex);
	bufferDesc.InitData = mesh2;
	bufferDesc.NumElements = 24;
	bufferDesc.Type = VERTEX_BUFFER;
	bufferDesc.Usage = BUFFER_DEFAULT;

	fence = new Buffer();
	if(FAILED(fence->Init(device, deviceContext, bufferDesc)))
	{
		return false;
	}

	delete[] vertices;
	vertices = NULL;

	delete[] indices;
	indices = 0;
	
	return true;
}

void Terrain::shutdown()
{
	if(index->GetBufferPointer())
	{
		delete index;
		index = NULL;
	}

	if(mesh->GetBufferPointer())
	{
		delete mesh;
		mesh = NULL;
	}

	if(fence->GetBufferPointer())
	{
		delete fence;
		fence = NULL;
	}

	delete g_Shader;
	delete g_Fence;
}

void Terrain::render(ID3D11DeviceContext* deviceContext, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX proj, D3DXVECTOR3 cam, PointLight& light, ID3D11ShaderResourceView* cubeMap, float tessFactor, D3DXVECTOR4* frustrumPlaneEquation)
{
	//g_Shader->SetResource("cubeMap", cubeMap);
	g_Shader->SetResource("gTexture1", texture1.getTexture());
	g_Shader->SetResource("gTexture2", texture2.getTexture());
	g_Shader->SetResource("gTexture3", texture3.getTexture());
	g_Shader->SetResource("gBlendMap", blendMap.getTexture());
	g_Shader->SetResource("gHeightMap", hm.getTexture());
	g_Shader->SetMatrix("gW", world);
	g_Shader->SetMatrix("gV", view);
	g_Shader->SetMatrix("gP", proj);
	g_Shader->SetMatrix("gVP", view * proj);
	g_Shader->SetMatrix("gWVP", world * view * proj);
	g_Shader->SetFloat4("eyePos", D3DXVECTOR4(cam, 1));
	g_Shader->SetRawData("light", &light, sizeof(light));
	g_Shader->SetFloat("tessFactor", tessFactor);
	g_Shader->SetRawData("frustrumPlaneEquation", frustrumPlaneEquation, sizeof(D3DXVECTOR4) * 4);

	if(GetAsyncKeyState('N'))
	{
		g_Shader->SetBool("normals", true);
	}
	else
	{
		g_Shader->SetBool("normals", false);
	}

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

	mesh->Apply(0);
	index->Apply(0);
	g_Shader->Apply(0);
	deviceContext->DrawIndexed(indexCount,0,0);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	g_Fence->SetResource("Texture", texture4.getTexture());
	g_Fence->SetMatrix("worldMatrix", world);
	g_Fence->SetMatrix("viewMatrix", view);
	g_Fence->SetMatrix("projectionMatrix", proj);

	BlendState::getInstance()->setState(0, deviceContext);
	fence->Apply(0);
	g_Fence->Apply(0);
	deviceContext->Draw(24,0);
}

int Terrain::getIndexCount()
{
	return indexCount;
}

float Terrain::getY(float x, float z)
{
	float X = x + (width * 0.5);
	float Z = (height * 0.5) - z;

	if(X >= 0 && X < width && Z >= 0 && Z < width)
		return heightMap->getData(X, Z);
	return 0;
}

int Terrain::getWidth()
{ 
	return width;
}

int Terrain::getHeight()
{ 
	return height;
}

bool Terrain::isInBounds(int x, int z)
{
	x = x + (width * 0.5);
	z = (height * 0.5) - z;

	if(x >= 3 && x < width-3 && z >= 3 && z < height-3)
		return true;
	return false;
}