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
	texture1.shutdown();
	texture2.shutdown();
	texture3.shutdown();
	blendMap.shutdown();
	texture4.shutdown();
}

HRESULT Terrain::initShader(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{

	blendState.createBlendState(device);

	D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	g_Shader = new Shader();
	if(FAILED(g_Shader->Init(device, deviceContext, "../Shaders/Basic.fx", inputDesc, 3)))
	{
		return E_FAIL;
	}

	g_Fence = new Shader();
	if(FAILED(g_Fence->Init(device, deviceContext, "../Shaders/SingleTexture.fx", inputDesc, 3)))
	{
		return E_FAIL;
	}

	texture1.init(device, "..\\Shaders\\stone.jpg");
	texture2.init(device, "..\\Shaders\\grassafterrain.jpg");
	texture3.init(device, "..\\Shaders\\water.jpg");
	blendMap.init(device, "..\\Shaders\\blendmap.jpg");
	texture4.init(device, "..\\Shaders\\wire.png");

	


	return S_OK;

}

bool Terrain::init(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	BUFFER_INIT_DESC vertexBufferDesc, indexBufferDesc;
	Vertex* vertices = NULL;
	unsigned long* indices = NULL;
	float posX = 0, posZ = 0;
	
	width = 256;
	height = 256;
	
	initShader(device, deviceContext);

	vertexCount = width*height;
	indexCount = (width-1) * (height-1) * 6;

	vertices = new Vertex[vertexCount];
	if(!vertices)
		return false;

	indices = new unsigned long[indexCount];
	if(!indices)
		return false;
	
	unsigned long ind = 0;
	
	heightMap = new HeightMap();
	heightMap->loadRaw(width,height, "../Shaders/hm.raw", 0.3f, 0);

	float** heightData = heightMap->getHeightMapData();

	float repeat = 1.0f;

	float x, y, z;
	float halfWidth = -(width*0.5f), halfHeight = (height*0.5f);
	for(int i = 0; i < width; i++)
	{
		x = halfWidth+i;
		for(int j = 0; j < height; j++)
		{
			int in = i * width + j;
			z = halfHeight-j;
			y = heightData[i][j];
			vertices[in].pos = D3DXVECTOR3(x, y, z);
			vertices[in].normal = D3DXVECTOR3(0,1,0);
			vertices[in].uv = D3DXVECTOR2(i/(width/repeat),j/(height/repeat));
		}
	}

	D3DXVECTOR3 vec1, vec2, vec3, vec4, vec5;

	for(int i = 0; i < width-1; i++)
	{
		for(int j = 0; j < height-1; j++)
		{
			vec1 = vertices[i * width + j].pos;
			vec2 = vertices[i * width + (j+1)].pos;
			vec3 = vertices[(i+1) * width + (j+1)].pos;

			vec4 = vec2 - vec1;
			vec5 = vec3 - vec1;

			D3DXVECTOR3 temp = vec5;

			vec5.x = (vec4.y * temp.z) - (vec4.z * temp.y);
			vec5.y = (vec4.z * temp.x) - (vec4.x * temp.z);
			vec5.z = (vec4.x * temp.y) - (vec4.y * temp.x);

			float length = sqrtf((vec5.x * vec5.x) + (vec5.y * vec5.y) + (vec5.z * vec5.z));

			vec5.x /= length;
			vec5.y /= length;
			vec5.z /= length;

			vertices[i * width + j].normal = vec5;
		}
	}

	ind = 0;
	
	for(int i = 0; i < width-1; i++)
	{
		for(int j = 0; j < height-1; j++)
		{
			indices[ind] = i * width + j;
			indices[ind+1] = i * width + j+1;
			indices[ind+2] = (i+1) * width + j+1;

			indices[ind+3] = i * width + j;
			indices[ind+4] = (i+1) * width + j+1;
			indices[ind+5] = (i+1) * width + j;

			ind += 6;
		}
	}

	vertexBufferDesc.ElementSize = sizeof(Vertex);
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

void Terrain::render(ID3D11DeviceContext* deviceContext, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX proj, D3DXVECTOR3 cam, PointLight &light)
{
	mesh->Unmap();
	index->Unmap();
	fence->Unmap();


	
	g_Shader->SetResource("gTexture1", texture1.getTexture());
	g_Shader->SetResource("gTexture2", texture2.getTexture());
	g_Shader->SetResource("gTexture3", texture3.getTexture());
	g_Shader->SetResource("gBlendMap", blendMap.getTexture());
	g_Shader->SetMatrix("gW", world);
	g_Shader->SetMatrix("gV", view);
	g_Shader->SetMatrix("gP", proj);
	g_Shader->SetFloat4("eyePos", D3DXVECTOR4(cam, 1));
	g_Shader->SetRawData("light", &light, sizeof(light));

	mesh->Apply(0);
	index->Apply(0);
	g_Shader->Apply(0);
	blendState.setState(1, deviceContext);
	deviceContext->DrawIndexed(indexCount,0,0);

	g_Fence->SetResource("Texture", texture4.getTexture());
	g_Fence->SetMatrix("worldMatrix", world);
	g_Fence->SetMatrix("viewMatrix", view);
	g_Fence->SetMatrix("projectionMatrix", proj);
	
	blendState.setState(0, deviceContext);
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
