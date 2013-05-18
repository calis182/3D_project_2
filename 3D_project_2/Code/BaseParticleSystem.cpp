#include "BaseParticleSystem.h"

BaseParticleSystem::BaseParticleSystem(ID3D11Device *device, ID3D11DeviceContext *deviceContext, D3DXVECTOR3 emit, int maxParticle, char* textureFilename)
{
	this->device = device;
	this->deviceContext = deviceContext;
	emitter = emit;

	this->maxParticle = maxParticle;
	nrOfParticles = 0;

	texture = NULL;
	if(strcmp(textureFilename, "NULL"))
	{
		texture = new TextureClass();
		texture->init(device, textureFilename);
	}
}

BaseParticleSystem::~BaseParticleSystem()
{
	if(texture != NULL)
	{
		texture->shutdown();
		delete texture;
	}

	for(int i = 0; i < particles.size(); i++)
		delete particles.at(i);

	delete shader;
	
	vbs[0]->Release();
	vbs[1]->Release();
}