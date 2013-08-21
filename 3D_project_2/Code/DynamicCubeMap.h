#ifndef DYNAMIC_CUBE_MAP_H
#define DYNAMIC_CUBE_MAP_H

#include "stdafx.h"

class DynamicCubeMap
{
public:
	DynamicCubeMap(ID3D11Device* device);
	~DynamicCubeMap();

	void setUpCameras(D3DXVECTOR3 pos);
	void init();

	void setPosition(D3DXVECTOR3 pos) { position = pos; }

	Camera getCamera(int index) { return cubeMapCamera[index]; }
	D3D11_VIEWPORT getViewPort(int res) { return cubeMapViewport[res]; }
	ID3D11ShaderResourceView* getCubeMap(int res) { return dynamicCubeMapSRV[res]; }
	ID3D11DepthStencilView* getDepthStencilView(int res) { return dynamicCubeMapDSV[res]; }
	ID3D11RenderTargetView* getRenderTargetView(int index, int res) { return dynamicCubeMapRTV[index][res]; }
	D3DXVECTOR3 getPosition() { return position; };

private:
	ID3D11DepthStencilView* dynamicCubeMapDSV[4];
	ID3D11RenderTargetView* dynamicCubeMapRTV[6][4];
	ID3D11ShaderResourceView* dynamicCubeMapSRV[4];
	
	//D3D11_VIEWPORT cubeMapViewport;
	D3D11_VIEWPORT cubeMapViewport[4];

	ID3D11Device* device;

	Camera cubeMapCamera[6];
	D3DXVECTOR3 position;

	static const int maxCubeMapSize = 1024;
	static const int minCubeMapSize = 128;

	int cubeMapSize[4];

};

#endif