#include "GaussianBlur.h"

GaussianBlur::GaussianBlur()
{
	texture = NULL;
}

GaussianBlur::~GaussianBlur()
{
	SAFE_DELETE(texture);

	verticalGaussian->Release();
	horizontalGaussian->Release();
}

bool GaussianBlur::init(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	HRESULT hr;
	this->device = device;
	this->deviceContext = deviceContext;

	ID3DBlob* pBlob = NULL;
	//Vertical
	hr = CompileShaderFromFile("..\\Shaders\\GaussianFilter.fx", "CSMainY", "cs_5_0", &pBlob);
	if(FAILED(hr))
		return false;

	hr = device->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &verticalGaussian);
	SAFE_RELEASE(pBlob);
	if(FAILED(hr))
		return false;

	//Horizontal
	hr = CompileShaderFromFile("..\\Shaders\\GaussianFilter.fx", "CSMainX", "cs_5_0", &pBlob);
	if(FAILED(hr))
		return false;

	hr = device->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &horizontalGaussian);
	SAFE_RELEASE(pBlob);
	if(FAILED(hr))
		return false;

	texture = new Texture();
	texture->init(device, SCREEN_WIDTH, SCREEN_HEIGHT, SHADER_RESOURCE | UNORDERED_RESOURCE);

	/*D3D11_TEXTURE2D_DESC texDesc;	
	texDesc.Width				= SCREEN_WIDTH;
	texDesc.Height				= SCREEN_HEIGHT;
	texDesc.MipLevels			= 1;
	texDesc.ArraySize			= 1;
	texDesc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count	= 1;
	texDesc.SampleDesc.Quality	= 0;
	texDesc.Usage				= D3D11_USAGE_DEFAULT;
	texDesc.BindFlags			= D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags		= 0;
	texDesc.MiscFlags			= 0;

	if(FAILED(device->CreateTexture2D(&texDesc, NULL, &blurTexture)))
		return false;

	if(FAILED(device->CreateShaderResourceView(blurTexture, NULL, &blurSRV)))
		return false;

	if(FAILED(device->CreateUnorderedAccessView(blurTexture, NULL, &blurUAV)))
		return false;

	SAFE_RELEASE(blurTexture);
	*/
	return true;
}

void GaussianBlur::blur(ID3D11DeviceContext* deviceContext, int blurPasses, ID3D11ShaderResourceView* mainSRV, ID3D11UnorderedAccessView* mainUAV)
{
	ID3D11ShaderResourceView* nullSRV[1] = { 0 };
	ID3D11UnorderedAccessView* nullUAV[1] = { 0 };
	for(int i = 0; i < blurPasses; i++)
	{
		//Horizontal pass
		deviceContext->CSSetShaderResources(0, 1, &mainSRV);
		deviceContext->CSSetUnorderedAccessViews(0, 1, texture->getUAV(), 0);
		deviceContext->CSSetShader(horizontalGaussian, NULL, 0);
		deviceContext->Dispatch((UINT)ceil(SCREEN_WIDTH/1024.0f), SCREEN_HEIGHT, 1);

		deviceContext->CSSetShaderResources( 0, 1, nullSRV);
		deviceContext->CSSetUnorderedAccessViews( 0, 1, nullUAV, 0);

		//Vertical pass
		deviceContext->CSSetShaderResources(0, 1, texture->getSRV());
		deviceContext->CSSetUnorderedAccessViews(0, 1, &mainUAV, 0);
		deviceContext->CSSetShader(verticalGaussian, NULL, 0);
		deviceContext->Dispatch(SCREEN_WIDTH, (UINT)ceil(SCREEN_HEIGHT/768.0f), 1);
		
		deviceContext->CSSetShaderResources( 0, 1, nullSRV);
		deviceContext->CSSetUnorderedAccessViews( 0, 1, nullUAV, 0);
	}
}