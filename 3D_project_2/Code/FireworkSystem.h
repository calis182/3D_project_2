#ifndef FIREWORK_SYSTEM_H
#define FIREWORK_SYSTEM_H

#include "BaseParticleSystem.h"
#include "stdafx.h"

class FireworkSystem : public BaseParticleSystem
{
public:
	FireworkSystem(ID3D11Device *device, ID3D11DeviceContext *deviceContext, D3DXVECTOR3 emit, int maxParticle, char* textureFilename);
	~FireworkSystem();

	virtual void render(D3DXMATRIX& world, D3DXMATRIX& view, D3DXMATRIX& proj);
	virtual void update(float dt, float frames, Camera& cam);

	void updateBuffers(float frames);
	void emitParticles(float time);
	void killParticles(float time, float dt);

private:


};

#endif