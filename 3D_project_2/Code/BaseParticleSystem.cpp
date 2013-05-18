#include "BaseParticleSystem.h"

BaseParticleSystem::BaseParticleSystem(ID3D11Device *device, ID3D11DeviceContext *deviceContext, D3DXVECTOR3 emit, int maxParticle, char* textureFilename)
{
	nrOfParticle = 0;
	this->device = device;
	this->deviceContext = deviceContext;
	emitter = emit;

	texture = NULL;
	if(strcmp(textureFilename, "NULL"))
	{
		texture = new TextureClass();
		texture->init(device, textureFilename);
	}

	vertices = new Vertex[maxParticle*6];

	BUFFER_INIT_DESC vertexBufferDesc;

	vertexBufferDesc.ElementSize = sizeof(Vertex);
	vertexBufferDesc.InitData = vertices;
	vertexBufferDesc.NumElements = maxParticle*6;
	vertexBufferDesc.Type = VERTEX_BUFFER;
	vertexBufferDesc.Usage = BUFFER_CPU_WRITE_DISCARD;

	buffer = new Buffer();
	buffer->Init(device, deviceContext, vertexBufferDesc);
}

BaseParticleSystem::~BaseParticleSystem()
{
	if(texture != NULL)
	{
		texture->shutdown();
		delete texture;
	}

	for(int i = 0; i < particles.size(); i++)
	{
		delete particles.at(i);
	}

	delete[] vertices;

	delete buffer;
	delete shader;
}