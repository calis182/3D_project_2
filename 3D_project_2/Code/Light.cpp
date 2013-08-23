#include "Light.h"

PointLight::PointLight(D3DXVECTOR4 amb, D3DXVECTOR4 spec, D3DXVECTOR4 diff, D3DXVECTOR3 pos, float range)
{
	ambient = amb;
	specular = spec;
	diffuse = diff;
	position = pos;
	this->range = range;
}

PointLight::~PointLight()
{
}

D3DXVECTOR4 PointLight::getAmbient()const
{
	return this->ambient;
}

D3DXVECTOR4 PointLight::getDiffuse()const
{
	return this->diffuse;
}

D3DXVECTOR3 PointLight::getPosition()const
{
	return this->position;
}