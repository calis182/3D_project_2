#ifndef TERRAIN_H
#define TERRAIN_H

#include "stdafx.h"
#include "HeightMap.h"
#include "TextureClass.h"
#include "BlendState.h"
#include "Light.h"


class Terrain
{
public:
	Terrain();
	~Terrain();

	HRESULT initShader(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	bool init(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	void shutdown();
	void render(ID3D11DeviceContext* deviceContext, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX proj, D3DXVECTOR3 cam, PointLight& light, ID3D11ShaderResourceView* cubeMap, float tessFactor, D3DXVECTOR4* frustrumPlaneEquation);

	int getIndexCount();

	float getY(float x, float z);

	int getWidth();
	int getHeight();

	bool isInBounds(int x, int z);

private:
	
	struct Vertex
	{
		D3DXVECTOR3 pos;
		D3DXVECTOR3 normal;
		D3DXVECTOR2 uv;
	};

	Buffer *mesh, *index, *fence;
	TextureClass texture1, texture2, texture3, blendMap, texture4, hm;
	Shader*	g_Shader, *g_Fence;
	HeightMap *heightMap;

	int width, height;
	int numOfStartingVertex;
	int vertexCount, indexCount;
};

#endif