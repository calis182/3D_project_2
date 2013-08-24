#include "Texture.h"

Texture::Texture()
{
	texture = NULL;
	SRV = NULL;
	RTV = NULL;
	UAV = NULL;
}

Texture::~Texture()
{
	SAFE_RELEASE(texture);
	SAFE_RELEASE(SRV);
	SAFE_RELEASE(RTV);
	SAFE_RELEASE(UAV);
}

bool Texture::init(ID3D11Device* device, int x, int y, UINT textureFlags)
{
	UINT bindFlags = 0;

	//set flags
	if(textureFlags & SHADER_RESOURCE)
		bindFlags |= D3D11_BIND_SHADER_RESOURCE;
	if(textureFlags & UNORDERED_RESOURCE)
		bindFlags |= D3D11_BIND_UNORDERED_ACCESS;
	if(textureFlags & RENDER_TARGET)
		bindFlags |= D3D11_BIND_RENDER_TARGET;

	D3D11_TEXTURE2D_DESC texDesc;	
	texDesc.Width				= x;
	texDesc.Height				= y;
	texDesc.MipLevels			= 1;
	texDesc.ArraySize			= 1;
	texDesc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count	= 1;
	texDesc.SampleDesc.Quality	= 0;
	texDesc.Usage				= D3D11_USAGE_DEFAULT;
	texDesc.BindFlags			= bindFlags;
	texDesc.CPUAccessFlags		= 0;
	texDesc.MiscFlags			= 0;

	//Create Texture2D
	if(FAILED(device->CreateTexture2D(&texDesc, NULL, &texture)))
		return false;

	//Create Shader resource view
	if(textureFlags & SHADER_RESOURCE)
	{
		if(FAILED(device->CreateShaderResourceView(texture, NULL, &SRV)))
			return false;
	}

	//Create Unordered access view
	if(textureFlags & UNORDERED_RESOURCE)
	{
		if(FAILED(device->CreateUnorderedAccessView(texture, NULL, &UAV)))
			return false;
	}

	//Create Render target view
	if(textureFlags & RENDER_TARGET)
	{
		if(FAILED(device->CreateRenderTargetView(texture, NULL, &RTV)))
			return false;
	}

	return true;
}

bool Texture::init(ID3D11Device* device, char* filename, UINT textureFlags)
{
	if(FAILED(D3DX11CreateShaderResourceViewFromFile(device, filename, NULL, NULL, &SRV, NULL)))
	{
		MessageBox(NULL, "Could not load texture", NULL, MB_OK);
		return false;
	}

	return true;
}

ID3D11Texture2D* Texture::getTexture()
{
	return texture;
}

ID3D11ShaderResourceView** Texture::getSRV()
{
	return &SRV;
}

ID3D11UnorderedAccessView** Texture::getUAV()
{
	return &UAV;
}

ID3D11RenderTargetView** Texture::getRTV()
{
	return &RTV;
}