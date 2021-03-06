#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include "Camera.h"
#include "Shader.h"

#include "stdafx.h"

#include "BaseParticleSystem.h"
#include "FireworkSystem.h"
#include "SnowSystem.h"
#include "TestSystem.h"

void updateParticles(void** param);

class ParticleSystem
{
private:
	std::vector<BaseParticleSystem*> systems;

public:
	ParticleSystem();
	~ParticleSystem();

	void Init(ID3D11Device *device, ID3D11DeviceContext *deviceContext);

	void Update(float dt, float frames, Camera& cam);
	void Draw(ID3D11DeviceContext* dc, D3DXMATRIX &world, D3DXMATRIX &view, D3DXMATRIX &proj);

	int getTotalNumOfParticles()
	{
		int total = 0;
		for(int i = 0; i < systems.size(); i++)
		{
			total += systems.at(i)->getNumberOfParticles();
		}
		return total;
	}

};

#endif