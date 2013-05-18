#include "TestSystem.h"

TestSystem::TestSystem(ID3D11Device *device, ID3D11DeviceContext *deviceContext, D3DXVECTOR3 emit, int maxParticle, char* textureFilename) : BaseParticleSystem(device, deviceContext, emit, maxParticle, textureFilename)
{
	D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	shader = new Shader();
	if(FAILED(shader->Init(device, deviceContext, "../Shaders/SingleTexture.fx", inputDesc, 3)))
	{
		exit(0);
	}
}

TestSystem::~TestSystem()
{
	delete shader;
}

void TestSystem::render(D3DXMATRIX &world, D3DXMATRIX &view, D3DXMATRIX &proj)
{
	shader->SetResource("Texture", texture->getTexture());

	shader->SetMatrix("worldMatrix", world);
	shader->SetMatrix("viewMatrix", view);
	shader->SetMatrix("projectionMatrix", proj);
	
	shader->Apply(0);
	buffer->Apply(0);

	deviceContext->Draw(nrOfParticle * 6, 0);
}

void TestSystem::update(float dt, float frames, Camera& cam)
{
	for(int i = 0; i < particles.size(); i++)
		static_cast<Fire*>(particles.at(i))->update(dt, frames);

	emitter = cam.GetPosition();
	emitter += cam.GetLook() * 20;

	//Ta bort partiklar
	killParticles(frames, dt);

	//lägg till nya partiklar
	emitParticles(frames);

	//uppdatera partiklar
	updateBuffers(frames);
}

void TestSystem::updateBuffers(float frames)
{
	Vertex* vertexPtr;

	D3DXVECTOR3 color(1, 0, 0);
	
	float sizeX = 0.5;
	float sizeZ = 0.5;
	int index = 0;
	for(int i = 0; i < particles.size(); i++)
	{
		D3DXVECTOR3 pos = particles.at(i)->getPosition();

		vertices[index].pos = D3DXVECTOR3(pos.x - sizeX, pos.y + 0.5, pos.z - sizeZ);
		vertices[index].normal = color;
		vertices[index].uv = D3DXVECTOR2(0, 0);
		index++;

		vertices[index].pos = D3DXVECTOR3(pos.x + sizeX, pos.y + 0.5, pos.z + sizeZ);
		vertices[index].normal = color;
		vertices[index].uv = D3DXVECTOR2(1, 0);
		index++;

		vertices[index].pos = D3DXVECTOR3(pos.x + sizeX, pos.y - 0.5, pos.z + sizeZ);
		vertices[index].normal = color;
		vertices[index].uv = D3DXVECTOR2(1, 1);
		index++;

		vertices[index].pos = D3DXVECTOR3(pos.x - sizeX, pos.y + 0.5, pos.z - sizeZ);
		vertices[index].normal = color;
		vertices[index].uv = D3DXVECTOR2(0, 0);
		index++;

		vertices[index].pos = D3DXVECTOR3(pos.x - sizeX, pos.y - 0.5, pos.z - sizeZ);
		vertices[index].normal = color;
		vertices[index].uv = D3DXVECTOR2(0, 1);
		index++;

		vertices[index].pos = D3DXVECTOR3(pos.x + sizeX, pos.y - 0.5, pos.z + sizeZ);
		vertices[index].normal = color;
		vertices[index].uv = D3DXVECTOR2(1, 1);
		index++;
	}
	
	vertexPtr = (Vertex*)buffer->Map();

	memcpy(vertexPtr, (void*)vertices, (sizeof(Vertex) * index));

	buffer->Unmap();
}

void TestSystem::emitParticles(float time)
{
	if(particles.size() < 9000)
	{
		for(int i = 0; i < 3; i++)
		{
			BaseParticle* temp = new Fire(emitter, D3DXVECTOR3(rand() % 10 - 5, 15, rand() % 10 - 5), 1.0f, NULL, time);
			particles.push_back(temp);
			nrOfParticle++;
		}
	}
}

void TestSystem::killParticles(float time, float dt)
{
	int deathTime = dt * (rand() % 2000 + 1000);
	int size = (int)particles.size();
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