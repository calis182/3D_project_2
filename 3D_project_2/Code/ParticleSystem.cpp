#include "ParticleSystem.h"

ParticleSystem::ParticleSystem()
{

}

ParticleSystem::~ParticleSystem()
{
	for(int i = 0; i < (int)systems.size(); i++)
	{
		delete systems.at(i);
	}
}

void ParticleSystem::Init(ID3D11Device *device, ID3D11DeviceContext *deviceContext)
{
	//skapa particle systemen som man vill ha
	systems.push_back(new SnowSystem(device, deviceContext, D3DXVECTOR3(0, 0, 0), 10000, "NULL"));

	systems.push_back(new FireworkSystem(device, deviceContext, D3DXVECTOR3(-64, 10, -64), 10000, "NULL"));
	systems.push_back(new FireworkSystem(device, deviceContext, D3DXVECTOR3(64, 10, 64), 10000, "NULL"));
	systems.push_back(new FireworkSystem(device, deviceContext, D3DXVECTOR3(64, 10, -64), 10000, "NULL"));
	systems.push_back(new FireworkSystem(device, deviceContext, D3DXVECTOR3(-64, 10, 64), 10000, "NULL"));

	systems.push_back(new TestSystem(device, deviceContext, D3DXVECTOR3(0, 10, 0), 10000, "..\\Shaders\\hearth.png"));
}

void ParticleSystem::Update(float dt, float frames, Camera& cam)
{
	for(int i = 0; i < (int)systems.size(); i++)
		systems.at(i)->update(dt, frames, cam);
}

void ParticleSystem::Draw(ID3D11DeviceContext* dc, D3DXMATRIX &world, D3DXMATRIX &view, D3DXMATRIX &proj)
{
	for(int i = 0; i < (int)systems.size(); i++)
		systems.at(i)->render(dc, world, view, proj);
}