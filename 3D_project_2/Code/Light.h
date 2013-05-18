#ifndef LIGHT_H
#define LIGHT_H

#include "stdafx.h"

class PointLight
{
public:
	PointLight(D3DXVECTOR4 amb, D3DXVECTOR4 spec, D3DXVECTOR4 diff, D3DXVECTOR3 pos, float range);
	~PointLight();

private:
	D3DXVECTOR4 ambient, specular, diffuse;
	D3DXVECTOR3 position;
	float range;

};

#endif