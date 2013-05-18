#include "SnowSystem.h"

SnowSystem::SnowSystem(ID3D11Device *device, ID3D11DeviceContext *deviceContext, D3DXVECTOR3 emit, int maxParticle, char* textureFilename) : BaseParticleSystem(device, deviceContext, emit, maxParticle, textureFilename)
{
	D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	shader = new Shader();
	if(FAILED(shader->Init(device, deviceContext, "../Shaders/ParticleColored.fx", inputDesc, 3)))
	{
		exit(0);
	}
}

SnowSystem::~SnowSystem()
{
	delete shader;
}

void SnowSystem::render(D3DXMATRIX &world, D3DXMATRIX &view, D3DXMATRIX &proj)
{
	shader->SetMatrix("worldMatrix", world);
	shader->SetMatrix("viewMatrix", view);
	shader->SetMatrix("projectionMatrix", proj);
	
	shader->Apply(0);
	buffer->Apply(0);

	deviceContext->Draw(nrOfParticle * 6, 0);
}

void SnowSystem::update(float dt, float frames, Camera& cam)
{
	for(int i = 0; i < particles.size(); i++)
		particles.at(i)->update(dt);

	//Ta bort partiklar
	killParticles(frames);

	//lägg till nya partiklar
	emitParticles(frames);

	//uppdatera partiklar
	updateBuffers();
}

void SnowSystem::updateBuffers()
{
	Vertex* vertexPtr;

	float sizeX = 0.25;
	float sizeZ = 0.25;
	int index = 0;
	for(int i = 0; i < particles.size(); i++)
	{
		D3DXVECTOR3 pos = particles.at(i)->getPosition();

		if((int)pos.x % 2 == 0)
		{
			sizeX = 0.10;
			sizeZ = 0;
		}
		else
		{
			sizeX = 0;
			sizeZ = 0.10;
		}

		vertices[index].pos = D3DXVECTOR3(pos.x - sizeX, pos.y + 0.10, pos.z - sizeZ);
		vertices[index].normal = D3DXVECTOR3(1, 1, 1);
		vertices[index].uv = D3DXVECTOR2(0, 0);
		index++;

		vertices[index].pos = D3DXVECTOR3(pos.x + sizeX, pos.y + 0.10, pos.z + sizeZ);
		vertices[index].normal = D3DXVECTOR3(1, 1, 1);
		vertices[index].uv = D3DXVECTOR2(1, 0);
		index++;

		vertices[index].pos = D3DXVECTOR3(pos.x + sizeX, pos.y - 0.10, pos.z + sizeZ);
		vertices[index].normal = D3DXVECTOR3(1, 1 , 1);
		vertices[index].uv = D3DXVECTOR2(1, 1);
		index++;

		vertices[index].pos = D3DXVECTOR3(pos.x - sizeX, pos.y + 0.10, pos.z - sizeZ);
		vertices[index].normal = D3DXVECTOR3(1, 1, 1);
		vertices[index].uv = D3DXVECTOR2(0, 0);
		index++;

		vertices[index].pos = D3DXVECTOR3(pos.x - sizeX, pos.y - 0.10, pos.z - sizeZ);
		vertices[index].normal = D3DXVECTOR3(1, 1, 1);
		vertices[index].uv = D3DXVECTOR2(0, 1);
		index++;

		vertices[index].pos = D3DXVECTOR3(pos.x + sizeX, pos.y - 0.10, pos.z + sizeZ);
		vertices[index].normal = D3DXVECTOR3(1, 1 , 1);
		vertices[index].uv = D3DXVECTOR2(1, 1);
		index++;
	}
	
	vertexPtr = (Vertex*)buffer->Map();

	memcpy(vertexPtr, (void*)vertices, (sizeof(Vertex) * index));

	buffer->Unmap();
}

void SnowSystem::emitParticles(float time)
{
	if(particles.size() < 9000)
	{
		BaseParticle* temp = new Snow(D3DXVECTOR3(rand() % 256 - 128, 200, rand() % 256 - 128), D3DXVECTOR3(0, -1, 0), 50.0f, NULL);
		particles.push_back(temp);
		nrOfParticle++;
	}
}

void SnowSystem::killParticles(float time)
{
	int size = particles.size();
	for(int i = 0; i < size; i++)
	{
		if(particles.at(i)->getPosition().y < 0)
		{
			BaseParticle* temp = particles.at(i);
			particles.erase(particles.begin() + i);
			nrOfParticle--;
			size--;
			delete temp;
		}
	}
}