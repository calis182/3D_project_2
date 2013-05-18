#ifndef BASE_PARTICLE_SYSTEM_H
#define BASE_PARTICLE_SYSTEM_H

#include "stdafx.h"
#include "Particle.h"


class BaseParticleSystem
{
public:
	BaseParticleSystem(ID3D11Device *device, ID3D11DeviceContext *deviceContext, D3DXVECTOR3 emit, int maxParticle, char* textureFilename);
	~BaseParticleSystem();

	virtual void render(D3DXMATRIX& world, D3DXMATRIX& view, D3DXMATRIX& proj) = 0;
	virtual void update(float dt, float frames, Camera& cam) = 0;

protected:
	struct Vertex
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 normal;
	D3DXVECTOR2 uv;

	Vertex()
	{
		pos = D3DXVECTOR3(0, 0, 0);
		normal = D3DXVECTOR3(0, 0, 0);
		uv = D3DXVECTOR2(0, 0);
	}
	
	Vertex(D3DXVECTOR3 p, D3DXVECTOR3 n, D3DXVECTOR2 tex)
	{
		pos = p;
		normal = n;
		uv = tex;
	}
};

	std::vector<BaseParticle*> particles;
	Vertex* vertices;

	int maxParticle;
	int nrOfParticle;

	D3DXVECTOR3 emitter;

	Shader* shader;
	Buffer* buffer;

	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;

	TextureClass* texture;

	Camera* camera;

};

#endif