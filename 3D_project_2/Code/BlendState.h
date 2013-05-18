#ifndef BLENDSTATE_H
#define BLENDSTATE_H

#include "stdafx.h"

class BlendState
{
public:
	BlendState();
	~BlendState();
	void createBlendState(ID3D11Device* mDevice);
	void setState(int state, ID3D11DeviceContext* mDeviceContext);


private:

	ID3D11BlendState* transparentBS[2];
	D3D11_BLEND_DESC blend;
	float blendFactor[4];
};



#endif