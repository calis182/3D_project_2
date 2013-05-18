#ifndef TEST_SYSTEM_H
#define TEST_SYSTEM_H

#include "BaseParticleSystem.h"
#include "stdafx.h"

class TestSystem : public BaseParticleSystem
{
public:
	TestSystem(ID3D11Device *device, ID3D11DeviceContext *deviceContext, D3DXVECTOR3 emit, int maxParticle, char* textureFilename);
	~TestSystem();

	virtual void render(D3DXMATRIX& world, D3DXMATRIX& view, D3DXMATRIX& proj);
	virtual void update(float dt, float frames, Camera& cam);
	
	void updateBuffers(Camera& cam);
	void emitParticles(float time);
	void killParticles(float time, float dt);

private:


};

#endif