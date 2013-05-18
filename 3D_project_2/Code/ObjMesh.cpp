#include "ObjMesh.h"

ObjMesh::ObjMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	this->device = device;
	this->deviceContext = deviceContext;
	shader = NULL;
	texture = NULL;
	D3DXMatrixIdentity(&world);
	pos = D3DXVECTOR3(0, 0, 0);
}

ObjMesh::~ObjMesh()
{
	texture->shutdown();

	delete vertexBuffer;
	delete indexBuffer;
	delete data;
	delete shader;
	delete texture;
}

bool ObjMesh::initiate(char* filename)
{
	data = new ObjData;

	if(!data->load(filename))
		return false;

	if(!createBuffers())
		return false;

	if(!initShader())
		return false;

	texture = new TextureClass();

	std::string path = "..\\Shaders\\obj_mesh\\" + data->getTexturePath();
	
	texture->init(device, path);

	return true;
}

bool ObjMesh::initShader()
{
	D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	shader = new Shader();

	if(FAILED(shader->Init(device, deviceContext, "..\\shaders\\object.fx", inputDesc, 3)))
	{
		delete shader;
		return false;
	}

	return true;
}

bool ObjMesh::createBuffers()
{
	int size = data->getNumberFaces() * 3;
	Vertex* vertices = new Vertex[size];
	int* indices = new int[size];

	//Create all vertices
	for(int i = 0, k = 0; k < size-3; i++, k+=3)
	{
		Triangle face = data->getFace(i);

		for(int j = 0; j < 3; j++)
		{
			indices[k+j] = (int)face.tr[j][0];
			vertices[k+j].pos = data->getVertex((int)face.tr[j][0]);
			vertices[k+j].tex = data->getTex((int)face.tr[j][1]);
			vertices[k+j].normal = data->getNormal((int)face.tr[j][2]);
		}
	}

	BUFFER_INIT_DESC bd;
	bd.ElementSize = sizeof(Vertex);
	bd.InitData = vertices;
	bd.NumElements = size;
	bd.Type = VERTEX_BUFFER;
	bd.Usage = BUFFER_DEFAULT;

	vertexBuffer = new Buffer();

	if(FAILED(vertexBuffer->Init(device, deviceContext, bd)))
	{
		delete vertices;
		delete indices;

		return false;
	}

	BUFFER_INIT_DESC bd2;
	bd2.ElementSize = sizeof(int);
	bd2.InitData = indices;
	bd2.NumElements = size;
	bd2.Type = INDEX_BUFFER;
	bd2.Usage = BUFFER_DEFAULT;

	indexBuffer = new Buffer();

	if(FAILED(indexBuffer->Init(device, deviceContext, bd2)))
	{
		delete[] vertices;
		delete[] indices;

		return false;
	}

	delete[] vertices;
	delete[] indices;

	vertices = NULL;
	indices = NULL;

	return true;
}

void ObjMesh::render(D3DXMATRIX& view, D3DXMATRIX& proj, D3DXVECTOR3& cameraPos, PointLight& light, ID3D11ShaderResourceView* cubeMap)
{
	shader->SetResource("cubeMap", cubeMap);
	shader->SetMatrix("gW", world);
	shader->SetMatrix("gV", view);
	shader->SetMatrix("gP", proj);
	shader->SetFloat4("eyePos", D3DXVECTOR4(cameraPos, 1));
	shader->SetRawData("light", &light, sizeof(light));
	
	shader->SetResource("texture1", texture->getTexture());


	shader->Apply(0);

	vertexBuffer->Apply(0);
	indexBuffer->Apply(0);

	deviceContext->Draw(data->getNumberFaces() * 3, 0);
}

void ObjMesh::billboard(const D3DXVECTOR3& cameraPos)
{
	// Calculate the rotation that needs to be applied to the billboard model to face the current camera position using the arc tangent function.
	float angle = atan2(pos.x - cameraPos.x, pos.z - cameraPos.z) * (float)(180.0 / D3DX_PI);

	// Convert rotation into radians.
	float rotation = (float)angle * 0.0174532925f;

	D3DXMatrixRotationY(&world, rotation);
	D3DXMatrixTranslation(&translate, pos.x, pos.y, pos.z);
	D3DXMatrixMultiply(&world, &world, &translate);
}