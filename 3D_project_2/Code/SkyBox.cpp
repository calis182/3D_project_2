#include "SkyBox.h"

SkyBox::SkyBox(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	this->device = device;
	this->deviceContext = deviceContext;
	shader = NULL;
	texture = NULL;

	D3DXMatrixIdentity(&world);
}

SkyBox::~SkyBox()
{
	if(texture)
	{
		texture->shutdown();
		delete texture;
	}

	if(shader)
		delete shader;

	delete vertexBuffer;
	delete indexBuffer;
}

bool SkyBox::init()
{
	if(!initShader())
		return false;

	if(!createBox())
		return false;

	texture = new TextureClass();
	texture->init(device, "..\\Shaders\\skybox.dds");

	return true;
}

bool SkyBox::initShader()
{
	D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	shader = new Shader();
	if(FAILED(shader->Init(device, deviceContext, "..\\Shaders\\CubeMap.fx", inputDesc, 1)))
		return false;

	return true;
}

bool SkyBox::createBox()
{
	Vertex* vertices = new Vertex[12];

	float size = 100;
	float w2 = 0.5f*size;
	float h2 = 0.5f*size;
	float d2 = 0.5f*size;

	vertices[0] = Vertex(-w2, h2, d2);
	vertices[1] = Vertex(-w2, -h2, d2);
	vertices[2] = Vertex(-w2, h2, -d2);
	vertices[3] = Vertex(-w2, -h2, -d2);

	vertices[4] = Vertex(w2, h2, -d2);
	vertices[5] = Vertex(w2, -h2, -d2);
	vertices[6] = Vertex(w2, h2, d2);
	vertices[7] = Vertex(w2, -h2, d2);
	
	vertices[8] = Vertex(-w2, h2, d2);
	vertices[9] = Vertex(-w2, -h2, d2);
	vertices[10] = Vertex(w2, h2, d2);
	vertices[11] = Vertex(w2, -h2, d2);


	BUFFER_INIT_DESC bd;
	bd.ElementSize = sizeof(Vertex);
	bd.InitData = vertices;
	bd.NumElements = 12;
	bd.Type = VERTEX_BUFFER;
	bd.Usage = BUFFER_DEFAULT;

	vertexBuffer = new Buffer();
	if(FAILED(vertexBuffer->Init(device, deviceContext, bd)))
		return false;

	int* ind = new int[36];

	//left
	ind[0] = 0; ind[1] = 2; ind[2] = 3;
	ind[3] = 0; ind[4] = 1; ind[5] = 3;

	//front
	ind[6] = 2; ind[7] = 4; ind[8] = 5;
	ind[9] = 2; ind[10] = 3; ind[11] = 5;
	
	//right
	ind[12] = 4; ind[13] = 6; ind[14] = 7;
	ind[15] = 4; ind[16] = 5; ind[17] = 7;
	
	//back
	ind[18] = 6; ind[19] = 0; ind[20] = 1;
	ind[21] = 6; ind[22] = 7; ind[23] = 1;

	//top
	ind[24] = 0; ind[25] = 2; ind[26] = 4;
	ind[27] = 0; ind[28] = 6; ind[29] = 4;
	
	//bottom
	ind[30] = 1; ind[31] = 3; ind[32] = 5;
	ind[33] = 1; ind[34] = 7; ind[35] = 5;

	
	BUFFER_INIT_DESC bd2;
	bd2.ElementSize = sizeof(int);
	bd2.InitData = ind;
	bd2.NumElements = 36;
	bd2.Type = INDEX_BUFFER;
	bd2.Usage = BUFFER_DEFAULT;

	indexBuffer = new Buffer();
	if(FAILED(indexBuffer->Init(device, deviceContext, bd2)))
	{
		delete[] vertices;
		delete[] ind;
		return false;
	}

	delete[] vertices;
	delete[] ind;

	return true;
}

void SkyBox::render(D3DXMATRIX vp, ID3D11ShaderResourceView* cubeMap)
{
	shader->SetResource("gCubeMap", cubeMap);
	shader->SetMatrix("gWVP", world*vp);
	
	vertexBuffer->Apply(0);
	indexBuffer->Apply(0);

	shader->Apply(0);

	deviceContext->DrawIndexed(36, 0, 0);
}

void SkyBox::update(D3DXVECTOR3 cameraPos)
{
	D3DXMATRIX scale;
	D3DXMatrixScaling(&scale, 10, 10, 10);
	D3DXMatrixTranslation(&translation, cameraPos.x, cameraPos.y, cameraPos.z);
	D3DXMatrixIdentity(&world);
	world *= scale * translation;
}