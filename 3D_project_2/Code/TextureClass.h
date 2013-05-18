#ifndef TEXTURECLASS_H
#define TEXTURECLASS_H

#include "stdafx.h"
#include <string.h>

class TextureClass
{
public:
	TextureClass();
	~TextureClass();

	bool init(ID3D11Device *device, std::string filename);

	void shutdown();

	ID3D11ShaderResourceView* getTexture();

private:
	
	ID3D11ShaderResourceView* texture;



};

#endif