#ifndef WATER_SIMULATION_H
#define WATER_SIMULATION_H

#include "stdafx.h"
#include "Texture.h"

class WaterSimulation
{
public:
	WaterSimulation();
	~WaterSimulation();

	bool init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, D3DXVECTOR3 pos, int sizeX, int sizeY, int calcX, int calcY, int detailed);
	bool initShaders(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	bool initResources(ID3D11Device* device, ID3D11DeviceContext* deviceContext, D3DXVECTOR3 pos, int sizeX, int sizeY, int calcX, int calcY, int detailed);

	void update(ID3D11DeviceContext* deviceContext, float dt);
	void render(ID3D11DeviceContext* deviceContext, D3DXMATRIX wvp, float tessFactor, D3DXVECTOR4* frustrumPlaneEquation);

	void setPosition(D3DXVECTOR3 pos);

private:
	struct WaterData
	{
		float height;
		D3DXVECTOR4 pipes;

		WaterData()
		{
			height = 0;
			pipes = D3DXVECTOR4(0, 0, 0, 0);
		}
	};

	struct PosUV
	{
		D3DXVECTOR3 pos;
		float padding;
		D3DXVECTOR2 uv;
	};

	struct ComputeShaderConstantBuffer
	{
		D3DXVECTOR4 timeFactors;
		D3DXVECTOR4 dispatchSize;
	};

	ID3D11Buffer* buffers[2];
	ID3D11ShaderResourceView* SRV[2];
	ID3D11UnorderedAccessView* UAV[2];

	ID3D11Buffer* constantBuffer;
	ComputeShaderConstantBuffer cBuffer;

	ID3D11ComputeShader* shader;
	Shader* renderShader;

	Texture* waterTexture;

	Buffer* tessMesh;
	Buffer* tessIndex;

	int tessVertexCount;
	int tessIndexCount;
	
	int threadGroupX, threadGroupY;

	int sizeX, sizeY;
	int calcX, calcY;
	int totalCalcPoints;

	D3DXMATRIX position;
};

#endif