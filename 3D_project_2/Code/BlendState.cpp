#include "BlendState.h"

BlendState::BlendState()
{
	for(int i=0; i<4; i++)
	{
		blendFactor[i]=1.0f;
	}

	
	
}

BlendState::~BlendState()
{
}

void BlendState::createBlendState(ID3D11Device* mDevice)
{
	
			blend.AlphaToCoverageEnable = true;
			blend.IndependentBlendEnable = false;

			blend.RenderTarget[0].BlendEnable = true;
			blend.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blend.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		
			mDevice->CreateBlendState(&blend, &transparentBS[0]);

			blend.AlphaToCoverageEnable = false;
			blend.IndependentBlendEnable = false;

			blend.RenderTarget[0].BlendEnable = false;
			blend.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blend.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			mDevice->CreateBlendState(&blend, &transparentBS[1]);
}

void BlendState::setState(int state, ID3D11DeviceContext* mDeviceContext)
{
	
	mDeviceContext->OMSetBlendState(transparentBS[state], blendFactor, 0xffffffff);
}

