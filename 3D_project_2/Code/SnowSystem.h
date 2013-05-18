#ifndef SNOW_SYSTEM_H
#define SNOW_SYSTEM_H

#include "BaseParticleSystem.h"
#include "stdafx.h"

class SnowSystem : public BaseParticleSystem
{
public:
	SnowSystem(ID3D11Device *device, ID3D11DeviceContext *deviceContext, D3DXVECTOR3 emit, int maxParticle, char* textureFilename);
	~SnowSystem();

	virtual void render(D3DXMATRIX& world, D3DXMATRIX& view, D3DXMATRIX& proj);
	virtual void update(float dt, float frames, Camera& cam);

	void updateBuffers();
	void emitParticles(float time);
	void killParticles(float time);

private:


};

#endif