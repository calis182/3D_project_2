#include "TextureClass.h"

TextureClass::TextureClass()
{
}

TextureClass::~TextureClass()
{

}

bool TextureClass::init(ID3D11Device *device, std::string filename)
{
	if(FAILED(D3DX11CreateShaderResourceViewFromFile(device, filename.c_str(), NULL, NULL, &texture, NULL)))
	{
		MessageBox(NULL, "Could not load texture", NULL, MB_OK);
		return false;
	}
	return true;
}

void TextureClass::shutdown()
{
	if(texture)
		texture->Release();
}

ID3D11ShaderResourceView* TextureClass::getTexture()
{
	return texture;
}