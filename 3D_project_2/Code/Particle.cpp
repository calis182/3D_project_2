#include "Particle.h"

BaseParticle::BaseParticle(D3DXVECTOR3 m_position, D3DXVECTOR3 m_direction, float m_velocity, TextureClass* m_texture)
{
	this->m_position = m_position;
	this->m_direction = m_direction;
	this->m_velocity = m_velocity;
	this->m_texture = m_texture;
}

void BaseParticle::setPositionX(float X)
{
	this->m_position.x = X;
}

void BaseParticle::setPositionY(float Y)
{
	this->m_position.y = Y;
}

void BaseParticle::setPositionZ(float Z)
{
	this->m_position.z = Z;
}

void BaseParticle::setDirectionX(float X)
{
	this->m_direction.x = X;
}

void BaseParticle::setDirectionY(float Y)
{
	this->m_direction.y = Y;
}

void BaseParticle::setDirectionZ(float Z)
{
	this->m_direction.z = Z;
}

void BaseParticle::update(float dt)
{
	m_position.x += m_direction.x * m_velocity * dt;
	m_position.y += m_direction.y * m_velocity * dt;
	m_position.z += m_direction.z * m_velocity * dt;
}

Fire::Fire(D3DXVECTOR3 m_position, D3DXVECTOR3 m_direction, float m_velocity, TextureClass* m_texture, float age)
	:BaseParticle(m_position, m_direction, m_velocity, m_texture)
{
	this->age = age;
}

float Fire::getAge()
{
	return age;
}

void Fire::update(float dt, float frames)
{
	float x = rand() % 3-1, z = rand() % 3-1;

	x += m_direction.x;
	z += m_direction.z;

	if(frames - age <= 20)
	{
		x *= 3;
		z *= 3;
	}

	m_position.x += x * m_velocity * dt;
	m_position.y += m_direction.y * m_velocity * dt;
	m_position.z += z * m_velocity * dt;
}

D3DXVECTOR3 BaseParticle::getPosition()
{
	return this->m_position;
}

D3DXVECTOR3 BaseParticle::getDirection()
{
	return this->m_direction;
}

float BaseParticle::getVelocity()
{
	return this->m_velocity;
}

Snow::Snow(D3DXVECTOR3 m_position, D3DXVECTOR3 m_direction, float m_velocity, TextureClass* m_texture) 
	: BaseParticle(m_position, m_direction, m_velocity, m_texture)
{
}