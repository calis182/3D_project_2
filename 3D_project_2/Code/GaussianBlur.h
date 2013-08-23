#ifndef GAUSSIAN_BLUR_H
#define GAUSSIAN_BLUR_H

#include "stdafx.h"

class GaussianBlur
{
public:
	GaussianBlur();
	~GaussianBlur();

	bool init(ID3D11Device* device, ID3D11DeviceContext* deviceContext);

	void blur(ID3D11DeviceContext* deviceContext, int blurPasses, ID3D11ShaderResourceView* mainSRV, ID3D11UnorderedAccessView* mainUAV);

	ID3D11Texture2D* blurTexture;
	ID3D11RenderTargetView* blurRTV;
	ID3D11ShaderResourceView* blurSRV;
	ID3D11UnorderedAccessView* blurUAV;
	ID3D11ComputeShader* bruteForceGaussian;
	ID3D11ComputeShader* verticalGaussian;
	ID3D11ComputeShader* horizontalGaussian;

private:
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;

};

#endif