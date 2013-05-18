#ifndef CAMERA__H
#define CAMERA__H

#include "stdafx.h"

class Camera
{
private:

	D3DXVECTOR3 m_position;
	D3DXVECTOR3 mRight;
	D3DXVECTOR3 mUp;
	D3DXVECTOR3 mLook;

	float mNearZ;
	float mFarZ;
	float mAspect;
	float mFovY;

	D3DXMATRIX mView;
	D3DXMATRIX mProj;

public:
	Camera();
	virtual ~Camera();

	void SetPosition(const D3DXVECTOR3& v);

	D3DXVECTOR3 GetPosition()const;

	D3DXVECTOR3 GetRight()const;
	D3DXVECTOR3 GetUp()const;
	D3DXVECTOR3 GetLook()const;

	float GetNearZ()const;
	float GetFarZ()const;
	float GetAspect()const;
	
	D3DXVECTOR3 CrossMatrix(const D3DXVECTOR3& v, const D3DXMATRIX& m);

	void SetLens(float fovY, float aspect, float zn, float zf);

	void LookAt(D3DXVECTOR3 pos, D3DXVECTOR3 target, D3DXVECTOR3 worldUp);

	void setLook(D3DXVECTOR3 look) { mLook = look; }
	void setUp(D3DXVECTOR3 up) { mUp = up; }
	void setRight(D3DXVECTOR3 right) { mRight = right; }

	D3DXMATRIX View()const;
	D3DXMATRIX Proj()const;
	D3DXMATRIX ViewsProj()const;

	void Walk(float dist);
	void Strafe(float dist);

	void Pitch(float angle);
	void Yaw(float angle);

	void UpdateViewMatrix();
};
#endif