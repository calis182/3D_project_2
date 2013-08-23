#include "DynamicCubeMap.h"

DynamicCubeMap::DynamicCubeMap(ID3D11Device* device)
{
	this->device = device;
}

DynamicCubeMap::~DynamicCubeMap()
{
	for(int j = 0; j < 4; j++)
	{
		dynamicCubeMapSRV[j]->Release();
		dynamicCubeMapDSV[j]->Release();
		for(int i = 0; i < 6; i++)
			dynamicCubeMapRTV[i][j]->Release();
	}
}

void DynamicCubeMap::init()
{
	int size = maxCubeMapSize / (maxCubeMapSize / minCubeMapSize);
	for(int i = 0; i < 4; i++)
	{
		cubeMapSize[i] = size * (i+1);
	}

	D3D11_TEXTURE2D_DESC texDesc[4];
	for(int i = 0; i < 4; i++)
	{
		texDesc[i].Width = cubeMapSize[i];
		texDesc[i].Height = cubeMapSize[i];
		texDesc[i].MipLevels = 0;
		texDesc[i].ArraySize = 6;
		texDesc[i].SampleDesc.Count = 1;
		texDesc[i].SampleDesc.Quality = 0;
		texDesc[i].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc[i].Usage = D3D11_USAGE_DEFAULT;
		texDesc[i].BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texDesc[i].CPUAccessFlags = 0;
		texDesc[i].MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE;
	}

	ID3D11Texture2D* cubeTex[4] = {0};
	for(int i = 0; i < 4; i++)
	{
		device->CreateTexture2D(&texDesc[i], NULL, &cubeTex[i]);
	}

	//Create all 6 render target views
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = texDesc[0].Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.MipSlice = 0;
	rtvDesc.Texture2DArray.ArraySize = 1;

	for(int j = 0; j < 4; j++)
	{
		for(int i = 0; i < 6; i++)
		{
			rtvDesc.Texture2DArray.FirstArraySlice = i;
			device->CreateRenderTargetView(cubeTex[j], &rtvDesc, &dynamicCubeMapRTV[i][j]);
		}
	}

	//Create shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc[0].Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = -1;
	
	for(int i = 0; i < 4; i++)
	{
		device->CreateShaderResourceView(cubeTex[i], &srvDesc, &dynamicCubeMapSRV[i]);
	}

	for(int i = 0; i < 4; i++)
	{
		cubeTex[i]->Release();
	}

	//Set up Depth stencil view
	D3D11_TEXTURE2D_DESC depthTexDesc[4];
	for(int i = 0; i < 4; i++)
	{
		depthTexDesc[i].Width = cubeMapSize[i];
		depthTexDesc[i].Height = cubeMapSize[i];
		depthTexDesc[i].MipLevels = 1;
		depthTexDesc[i].ArraySize = 1;
		depthTexDesc[i].SampleDesc.Count = 1;
		depthTexDesc[i].SampleDesc.Quality = 0;
		depthTexDesc[i].Format = DXGI_FORMAT_D16_UNORM;
		depthTexDesc[i].Usage = D3D11_USAGE_DEFAULT;
		depthTexDesc[i].BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthTexDesc[i].CPUAccessFlags = 0;
		depthTexDesc[i].MiscFlags = 0;
	}

	ID3D11Texture2D* depthTex[4] = {0};
	for(int i = 0; i < 4; i++)
	{
		device->CreateTexture2D(&depthTexDesc[i], NULL, &depthTex[i]);
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = depthTexDesc[0].Format;
	dsvDesc.Flags = 0;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	for(int i = 0; i < 4; i++)
	{
		device->CreateDepthStencilView(depthTex[i], &dsvDesc, &dynamicCubeMapDSV[i]);
	}

	for(int i = 0; i < 4; i++)
	{
		depthTex[i]->Release();
	}

	for(int i = 0; i < 4; i++)
	{
		cubeMapViewport[i].TopLeftX = 0.0f;
		cubeMapViewport[i].TopLeftY = 0.0f;
		cubeMapViewport[i].Width = cubeMapSize[i];
		cubeMapViewport[i].Height = cubeMapSize[i];
		cubeMapViewport[i].MinDepth = 0.0f;
		cubeMapViewport[i].MaxDepth = 1.0f;
	}
}

void DynamicCubeMap::setUpCameras(D3DXVECTOR3 pos)
{
	setPosition(pos);
	D3DXVECTOR3 worldUp(0, 1, 0);

	D3DXVECTOR3 lookAt[6] = {
		D3DXVECTOR3(1.0f, 0, 0), 
		D3DXVECTOR3(-1.0f, 0, 0), 
		D3DXVECTOR3(0, 1.0f, 0), 
		D3DXVECTOR3(0, -1.0f, 0), 
		D3DXVECTOR3(0, 0, 1.0f), 
		D3DXVECTOR3(0, 0, -1.0f), 
	};

	D3DXVECTOR3 ups[6] = {
		D3DXVECTOR3(0.0f, 1.0f, 0.0f),
		D3DXVECTOR3(0.0f, 1.0f, 0.0f),
		D3DXVECTOR3(0.0f, 0.0f, -1.0f),
		D3DXVECTOR3(0.0f, 0.0f, 1.0f),
		D3DXVECTOR3(0.0f, 1.0f, 0.0f),
		D3DXVECTOR3(0.0f, 1.0f, 0.0f),
	};

	D3DXVECTOR3 right[6] = {
		D3DXVECTOR3(0.0f, 0.0f, -1.0f),
		D3DXVECTOR3(0.0f, 0.0f, 1.0f),
		D3DXVECTOR3(1.0f, 0.0f, 0.0f),
		D3DXVECTOR3(1.0f, 0.0f, 0.0f),
		D3DXVECTOR3(1.0f, 0.0f, 0.0f),
		D3DXVECTOR3(-1.0f, 0.0f, 0.0f),
	};

	for(int i = 0; i < 6; i++)
	{
		cubeMapCamera[i].setLook(lookAt[i]);
		cubeMapCamera[i].setUp(ups[i]);
		cubeMapCamera[i].setRight(right[i]);
		cubeMapCamera[i].SetPosition(pos);
		
		cubeMapCamera[i].SetLens(PI * 0.5f, 1.0f, 0.1f, 1000.0f);
		cubeMapCamera[i].UpdateViewMatrix();
	}
}