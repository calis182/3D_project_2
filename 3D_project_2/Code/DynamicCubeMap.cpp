#include "DynamicCubeMap.h"

DynamicCubeMap::DynamicCubeMap(ID3D11Device* device)
{
	this->device = device;
}

DynamicCubeMap::~DynamicCubeMap()
{
	dynamicCubeMapSRV->Release();
	dynamicCubeMapDSV->Release();
	for(int i = 0; i < 6; i++)
		dynamicCubeMapRTV[i]->Release();
}

void DynamicCubeMap::init()
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = cubeMapSize;
	texDesc.Height = cubeMapSize;
	texDesc.MipLevels = 0;
	texDesc.ArraySize = 6;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE;

	ID3D11Texture2D* cubeTex = 0;
	device->CreateTexture2D(&texDesc, NULL, &cubeTex);

	//Create all 6 render target views
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.MipSlice = 0;
	rtvDesc.Texture2DArray.ArraySize = 1;

	for(int i = 0; i < 6; i++)
	{
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		device->CreateRenderTargetView(cubeTex, &rtvDesc, &dynamicCubeMapRTV[i]);
	}

	//Create shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = -1;

	device->CreateShaderResourceView(cubeTex, &srvDesc, &dynamicCubeMapSRV);

	cubeTex->Release();

	//Set up Depth stencil view
	D3D11_TEXTURE2D_DESC depthTexDesc;
	depthTexDesc.Width = cubeMapSize;
	depthTexDesc.Height = cubeMapSize;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.ArraySize = 1;
	depthTexDesc.SampleDesc.Count = 1;
	depthTexDesc.SampleDesc.Quality = 0;
	depthTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthTexDesc.CPUAccessFlags = 0;
	depthTexDesc.MiscFlags = 0;

	ID3D11Texture2D* depthTex = 0;
	device->CreateTexture2D(&depthTexDesc, NULL, &depthTex);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = depthTexDesc.Format;
	dsvDesc.Flags = 0;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	device->CreateDepthStencilView(depthTex, &dsvDesc, &dynamicCubeMapDSV);

	depthTex->Release();

	//Set up a new viewport
	cubeMapViewport.TopLeftX = 0.0f;
	cubeMapViewport.TopLeftY = 0.0f;
	cubeMapViewport.Width = cubeMapSize;
	cubeMapViewport.Height = cubeMapSize;
	cubeMapViewport.MinDepth = 0.0f;
	cubeMapViewport.MaxDepth = 1.0f;
}

void DynamicCubeMap::setUpCameras(D3DXVECTOR3 pos)
{
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
		
		cubeMapCamera[i].UpdateViewMatrix();
		cubeMapCamera[i].SetLens(PI * 0.5f, 1.0f, 0.1f, 1000.0f);
	}
}