#ifndef PARTICLE_H
#define PARTICLE_H

#include "stdafx.h"
#include "TextureClass.h"

class BaseParticle
{
protected:
	D3DXVECTOR3 m_position;
	D3DXVECTOR3 m_direction;
	float m_velocity;
	TextureClass* m_texture;

public:
	BaseParticle(D3DXVECTOR3 m_position, D3DXVECTOR3 m_direction, float m_velocity, TextureClass* m_texture);
	void setPositionX(float X);
	void setPositionY(float Y);
	void setPositionZ(float Z);
	void setDirectionX(float X);
	void setDirectionY(float Y);
	void setDirectionZ(float Z);

	D3DXVECTOR3 getPosition();
	D3DXVECTOR3 getDirection();
	float getVelocity();

	virtual void update(float dt);
};

class Fire : public BaseParticle
{
private:
	float age;
public:
	Fire(D3DXVECTOR3 m_position, D3DXVECTOR3 m_direction, float m_velocity, TextureClass* m_texture, float age);
	
	float getAge();

	virtual void update(float dt, float frames);
};

class Snow : public BaseParticle
{
public:
	Snow(D3DXVECTOR3 m_position, D3DXVECTOR3 m_direction, float m_velocity, TextureClass* m_texture);
	

};

#endif