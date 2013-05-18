#ifndef BLENDSTATE_H
#define BLENDSTATE_H

#include "stdafx.h"

class BlendState
{
public:
	void createBlendState(ID3D11Device* mDevice);
	void setState(int state, ID3D11DeviceContext* mDeviceContext);

	static BlendState* getInstance();

	void shutdown();

private:
	BlendState();
	~BlendState();

	
	static BlendState* blendState;
	ID3D11BlendState* transparentBS[2];
	D3D11_BLEND_DESC blend;
	float blendFactor[4];
};



#endif