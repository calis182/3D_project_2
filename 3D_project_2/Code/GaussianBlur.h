#ifndef GAUSSIAN_BLUR_H
#define GAUSSIAN_BLUR_H

#include "stdafx.h"
#include "Texture.h"

class GaussianBlur
{
public:
	GaussianBlur();
	~GaussianBlur();

	bool init(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	void blur(ID3D11DeviceContext* deviceContext, int blurPasses, ID3D11ShaderResourceView* mainSRV, ID3D11UnorderedAccessView* mainUAV);

private:
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;

	Texture* texture;

	ID3D11ComputeShader* verticalGaussian;
	ID3D11ComputeShader* horizontalGaussian;
};

#endif