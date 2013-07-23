#include "BlendState.h"

BlendState* BlendState::blendState = NULL;
BlendState::BlendState()
{
	for(int i=0; i<4; i++)
	{
		blendFactor[i]=1.0f;
	}
}

BlendState::~BlendState()
{
	//transparentBS[0]->Release();
	//transparentBS[1]->Release();
}

void BlendState::shutdown()
{
	delete blendState;
}

BlendState* BlendState::getInstance()
{
	if(blendState == NULL)
	{
		blendState = new BlendState();
	}
	return blendState;
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

	D3D11_BLEND_DESC blend2;

	blend2.AlphaToCoverageEnable = false;
	blend2.IndependentBlendEnable = true;

	blend2.RenderTarget[0].BlendEnable = false;
	blend2.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blend2.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blend2.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend2.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend2.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blend2.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend2.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	
	mDevice->CreateBlendState(&blend2, &transparentBS[1]);

/*
	blend.RenderTarget[1].BlendEnable = true;
	blend.RenderTarget[1].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blend.RenderTarget[1].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blend.RenderTarget[1].BlendOp = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[1].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[1].DestBlendAlpha = D3D11_BLEND_ZERO;
	blend.RenderTarget[1].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[1].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	*/
}

void BlendState::setState(int state, ID3D11DeviceContext* mDeviceContext)
{
	
	mDeviceContext->OMSetBlendState(transparentBS[state], blendFactor, 0xffffffff);
}

