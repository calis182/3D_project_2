#include "Particle.h"

BaseParticle::BaseParticle(D3DXVECTOR3 m_position, D3DXVECTOR3 m_direction, float m_velocity, TextureClass* m_texture)
{
	this->m_position = m_position;
	this->m_direction = m_direction;
	this->m_velocity = m_velocity;
	this->m_texture = m_texture;
	D3DXMatrixTranslation(&matrix, m_position.x, m_position.y, m_position.z);
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

void BaseParticle::init(D3DXVECTOR3& m_position, D3DXVECTOR3& m_direction, float m_velocity)
{
	this->m_position = m_position;
	this->m_direction = m_direction;
	this->m_velocity = m_velocity;
	D3DXMatrixTranslation(&matrix, m_position.x, m_position.y, m_position.z);
}

void BaseParticle::update(float dt)
{
	float velo = m_velocity * dt;
	m_position.x += m_direction.x * velo;
	m_position.y += m_direction.y * velo;
	m_position.z += m_direction.z * velo;

	D3DXMatrixTranslation(&matrix, m_position.x, m_position.y, m_position.z);
}

D3DXVECTOR3& BaseParticle::getPosition()
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

	float velo = m_velocity * dt;

	m_position.x += x * velo;
	m_position.y += m_direction.y * velo;
	m_position.z += z * velo;

	D3DXMatrixTranslation(&matrix, m_position.x, m_position.y, m_position.z);
}

void Fire::init(D3DXVECTOR3& m_position, D3DXVECTOR3& m_direction, float m_velocity, float age)
{
	BaseParticle::init(m_position, m_direction, m_velocity);
	this->age = age;
}

Snow::Snow(D3DXVECTOR3 m_position, D3DXVECTOR3 m_direction, float m_velocity, TextureClass* m_texture) 
	: BaseParticle(m_position, m_direction, m_velocity, m_texture)
{
}