#include "FireworkSystem.h"

FireworkSystem::FireworkSystem(ID3D11Device *device, ID3D11DeviceContext *deviceContext, D3DXVECTOR3 emit, int maxParticle, char* textureFilename) : BaseParticleSystem(device, deviceContext, emit, maxParticle, textureFilename)
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

FireworkSystem::~FireworkSystem()
{
	delete shader;
}

void FireworkSystem::render(D3DXMATRIX &world, D3DXMATRIX &view, D3DXMATRIX &proj)
{
	shader->SetMatrix("worldMatrix", world);
	shader->SetMatrix("viewMatrix", view);
	shader->SetMatrix("projectionMatrix", proj);
	
	shader->Apply(0);
	buffer->Apply(0);
	deviceContext->Draw(nrOfParticle * 6, 0);
}

void FireworkSystem::update(float dt, float frames, Camera& cam)
{
	for(int i = 0; i < particles.size(); i++)
		static_cast<Fire*>(particles.at(i))->update(dt, frames);

	//Ta bort partiklar
	killParticles(frames, dt);


	//uppdatera partiklar
	updateBuffers(frames);

	static bool done = false;

	if(emitter.y < 120)
		emitter.y += 100 * dt;
	else	//lägg till nya partiklar
	{
		if(!done)
		{
			for(int i = 0; i < 1000; i++)
			{
				BaseParticle* temp = new Fire(emitter, D3DXVECTOR3(sin((float)i) * (rand() % 10 - 5), 0, sin((float)i) * (rand() % 10 - 5)), 10.0f, NULL, frames);
				particles.push_back(temp);
				nrOfParticle++;
			}
		}
		done = true;

		emitter.y = -300;
		done = false;
	}

	if(!done)
		emitParticles(frames);
}

void FireworkSystem::updateBuffers(float frames)
{
	Vertex* vertexPtr;

	D3DXVECTOR3 color = D3DXVECTOR3((rand() % 100) * 0.01, (rand() % 100) * 0.01, (rand() % 100) * 0.01);
	
	float sizeX = 0.25;
	float sizeZ = 0.25;
	int index = 0;
	for(int i = 0; i < particles.size(); i++)
	{
		D3DXVECTOR3 pos = particles.at(i)->getPosition();

		if((int)pos.x % 2 == 0)
		{
			sizeX = 0.25;
			sizeZ = 0;
		}
		else
		{
			sizeX = 0;
			sizeZ = 0.25;
		}

		vertices[index].pos = D3DXVECTOR3(pos.x - sizeX, pos.y + 0.25, pos.z - sizeZ);
		vertices[index].normal = color;
		vertices[index].uv = D3DXVECTOR2(0, 0);
		index++;

		vertices[index].pos = D3DXVECTOR3(pos.x + sizeX, pos.y + 0.25, pos.z + sizeZ);
		vertices[index].normal = color;
		vertices[index].uv = D3DXVECTOR2(1, 0);
		index++;

		vertices[index].pos = D3DXVECTOR3(pos.x + sizeX, pos.y - 0.25, pos.z + sizeZ);
		vertices[index].normal = color;
		vertices[index].uv = D3DXVECTOR2(1, 1);
		index++;

		vertices[index].pos = D3DXVECTOR3(pos.x - sizeX, pos.y + 0.25, pos.z - sizeZ);
		vertices[index].normal = color;
		vertices[index].uv = D3DXVECTOR2(0, 0);
		index++;

		vertices[index].pos = D3DXVECTOR3(pos.x - sizeX, pos.y - 0.25, pos.z - sizeZ);
		vertices[index].normal = color;
		vertices[index].uv = D3DXVECTOR2(0, 1);
		index++;

		vertices[index].pos = D3DXVECTOR3(pos.x + sizeX, pos.y - 0.25, pos.z + sizeZ);
		vertices[index].normal = color;
		vertices[index].uv = D3DXVECTOR2(1, 1);
		index++;
	}
	
	vertexPtr = (Vertex*)buffer->Map();

	memcpy(vertexPtr, (void*)vertices, (sizeof(Vertex) * index));

	buffer->Unmap();
}

void FireworkSystem::emitParticles(float time)
{
	if(particles.size() < 9000)
	{
		BaseParticle* temp = new Fire(emitter, D3DXVECTOR3(0, 0, 0), 20.0f, NULL, time);
		particles.push_back(temp);
		nrOfParticle++;
	}
}

void FireworkSystem::killParticles(float time, float dt)
{
	int deathTime = dt * (rand() % 50500 + 300);
	int size = particles.size();
	for(int i = 0; i < size; i++)
	{
		if(time - static_cast<Fire*>(particles.at(i))->getAge() > deathTime)
		{
			BaseParticle* temp = particles.at(i);
			particles.erase(particles.begin() + i);
			nrOfParticle--;
			size--;
			delete temp;
		}
	}
}