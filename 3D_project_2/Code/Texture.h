#ifndef TEXTURE_H
#define TEXTURE_H

#include "stdafx.h"

enum BindFlags
{
	RENDER_TARGET = 0x01,
	SHADER_RESOURCE = 0x02,
	UNORDERED_RESOURCE = 0x04,
};

class Texture
{
public:
	Texture();
	~Texture();

	bool init(ID3D11Device* device, int x, int y, UINT textureFlags);
	bool init(ID3D11Device* device, char* filename, UINT textureFlags);

	ID3D11Texture2D* getTexture();
	ID3D11ShaderResourceView** getSRV();
	ID3D11UnorderedAccessView** getUAV();
	ID3D11RenderTargetView** getRTV();

private:
	ID3D11Texture2D* texture;
	ID3D11ShaderResourceView* SRV;
	ID3D11UnorderedAccessView* UAV;
	ID3D11RenderTargetView* RTV;

};

#endif