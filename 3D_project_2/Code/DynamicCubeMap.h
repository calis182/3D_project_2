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
	D3D11_VIEWPORT getViewPort() { return cubeMapViewport; }
	ID3D11ShaderResourceView* getCubeMap() { return dynamicCubeMapSRV; }
	ID3D11DepthStencilView* getDepthStencilView() { return dynamicCubeMapDSV; }
	ID3D11RenderTargetView* getRenderTargetView(int index) { return dynamicCubeMapRTV[index]; }
	D3DXVECTOR3 getPosition() { return position; };

private:
	ID3D11DepthStencilView* dynamicCubeMapDSV;
	ID3D11RenderTargetView* dynamicCubeMapRTV[6];
	ID3D11ShaderResourceView* dynamicCubeMapSRV;
	D3D11_VIEWPORT cubeMapViewport;

	ID3D11Device* device;

	Camera cubeMapCamera[6];
	D3DXVECTOR3 position;

	static const int cubeMapSize = 256;

};

#endif