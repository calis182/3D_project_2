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
	
	int getNumberOfParticles() { return nrOfParticles; }

protected:
	struct Vertex
	{
		D3DXVECTOR3 pos;
		float pad1;
		D3DXVECTOR3 normal;
		float pad2;
		D3DXVECTOR2 uv;
		float padding3[2];

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

	struct InstancedData
	{
		D3DXMATRIX matrix;
	};

	std::vector<BaseParticle*> particles;

	int maxParticle;
	int nrOfParticles;

	D3DXVECTOR3 emitter;

	Shader* shader;
	ID3D11Buffer* vbs[2];

	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;

	TextureClass* texture;

	Camera* camera;

};

#endif