#include "SnowSystem.h"

SnowSystem::SnowSystem(ID3D11Device *device, ID3D11DeviceContext *deviceContext, D3DXVECTOR3 emit, int maxParticle, char* textureFilename) : BaseParticleSystem(device, deviceContext, emit, maxParticle, textureFilename)
{
	D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};

	shader = new Shader();
	if(FAILED(shader->Init(device, deviceContext, "../Shaders/ParticleColored.fx", inputDesc, 7)))
	{
		exit(0);
	}

	//Init buffers
	D3DXVECTOR3 color = D3DXVECTOR3(1, 1, 1);
	float sizeX = 0.1;
	float sizeZ = 0.1;
	int index = 0;
	Vertex vertices[6];
	vertices[index].pos = D3DXVECTOR3(0 - sizeX, 0 + 0.25, 0 - sizeZ);
	vertices[index].normal = color;
	vertices[index].uv = D3DXVECTOR2(0, 0);
	index++;

	vertices[index].pos = D3DXVECTOR3(0 + sizeX, 0 + 0.25, 0 + sizeZ);
	vertices[index].normal = color;
	vertices[index].uv = D3DXVECTOR2(1, 0);
	index++;

	vertices[index].pos = D3DXVECTOR3(0 + sizeX, 0 - 0.25, 0 + sizeZ);
	vertices[index].normal = color;
	vertices[index].uv = D3DXVECTOR2(1, 1);
	index++;

	vertices[index].pos = D3DXVECTOR3(0 - sizeX, 0 + 0.25, 0 - sizeZ);
	vertices[index].normal = color;
	vertices[index].uv = D3DXVECTOR2(0, 0);
	index++;

	vertices[index].pos = D3DXVECTOR3(0 - sizeX, 0 - 0.25, 0 - sizeZ);
	vertices[index].normal = color;
	vertices[index].uv = D3DXVECTOR2(0, 1);
	index++;

	vertices[index].pos = D3DXVECTOR3(0 + sizeX, 0 - 0.25, 0 + sizeZ);
	vertices[index].normal = color;
	vertices[index].uv = D3DXVECTOR2(1, 1);
	index++;

	//Vertex buffer
	D3D11_BUFFER_DESC vbd1;
	vbd1.Usage = D3D11_USAGE_IMMUTABLE;
	vbd1.ByteWidth = sizeof(Vertex) * 6;
	vbd1.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd1.CPUAccessFlags = 0;
	vbd1.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices;
	device->CreateBuffer(&vbd1, &vinitData, &vbs[0]);

	//Instanced buffer
	D3D11_BUFFER_DESC vbd2;
	vbd2.Usage = D3D11_USAGE_DYNAMIC;
	vbd2.ByteWidth = sizeof(InstancedData) * maxParticle;
	vbd2.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd2.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd2.MiscFlags = 0;
	vbd2.StructureByteStride = 0;

	device->CreateBuffer(&vbd2, NULL, &vbs[1]);

	//Create particles
	for(int i = 0; i < maxParticle; i++)
		particles.push_back(new Snow(D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(0, 0, 0), 0, NULL));
}

SnowSystem::~SnowSystem()
{
}

void SnowSystem::render(ID3D11DeviceContext* deviceContext, D3DXMATRIX &world, D3DXMATRIX &view, D3DXMATRIX &proj)
{
	shader->SetMatrix("worldMatrix", world);
	shader->SetMatrix("viewMatrix", view);
	shader->SetMatrix("projectionMatrix", proj);
	
	shader->Apply(0);

	UINT stride[2] = {sizeof(Vertex), sizeof(InstancedData)};
	UINT asd[2] = {0, 0};

	deviceContext->IASetVertexBuffers(0, 2, vbs, stride, asd);
	deviceContext->DrawInstanced(6, nrOfParticles, 0, 0);
}

void SnowSystem::update(float dt, float frames, Camera& cam)
{
	for(int i = 0; i < nrOfParticles; i++)
		particles.at(i)->update(dt);

	//Ta bort partiklar
	killParticles(frames);

	//lägg till nya partiklar
	emitParticles(frames);

	//uppdatera partiklar
	updateBuffers(cam);
}

void SnowSystem::updateBuffers(Camera& cam)
{
	//Update instancedData
	D3D11_MAPPED_SUBRESOURCE mappedData;
	deviceContext->Map(vbs[1], 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mappedData);

	InstancedData* dataView = reinterpret_cast<InstancedData*>(mappedData.pData);

	for(int i = 0; i < nrOfParticles; i++)
		dataView[i].matrix = particles.at(i)->getMatrix();

	deviceContext->Unmap(vbs[1], 0);
}

void SnowSystem::emitParticles(float time)
{
	if(nrOfParticles < maxParticle)
	{
		particles.at(nrOfParticles)->init(D3DXVECTOR3(rand() % 256 - 128, 200, rand() % 256 - 128), D3DXVECTOR3(0, -1, 0), 50.0f);
		nrOfParticles++;
	}
}

void SnowSystem::killParticles(float time)
{
	for(int i = 0; i < nrOfParticles; i++)
	{
		if(particles.at(i)->getPosition().y < 0)
		{
			nrOfParticles--;
			BaseParticle* temp = particles.at(i);
			particles.at(i) = particles.at(nrOfParticles);
			particles.at(nrOfParticles) = temp;
		}
	}
}