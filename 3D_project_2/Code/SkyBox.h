#ifndef SKYBOX_H
#define SKYBOX_H

#include "stdafx.h"


class SkyBox
{
public:
	SkyBox(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	~SkyBox();

	bool init();
	bool initShader();
	bool createBox();
	
	void render(D3DXMATRIX vp, ID3D11ShaderResourceView* cubeMap);
	void update(D3DXVECTOR3 cameraPos);
	
	ID3D11ShaderResourceView* getCubeMap() { return texture->getTexture(); }

private:
	struct Vertex
	{
		Vertex()
		{}

		Vertex(D3DXVECTOR3 p)
		{
			pos = p;
		}

		Vertex(float pX, float pY, float pZ)
		{
			pos.x = pX;
			pos.y = pY;
			pos.z = pZ;
		}

		D3DXVECTOR3 pos;
	};

	D3DXMATRIX world, translation, scale; 
	Shader* shader;

	Buffer* vertexBuffer, *indexBuffer;

	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;

	TextureClass* texture;

};

#endif