#pragma once
#include "physics_component.h"

class CameraComponent;

class PlayerPhysics: public PhysicsComponent{
public:
	PlayerPhysics();
	virtual ~PlayerPhysics();
	virtual void Initialize();

	virtual void Update(float dTime);

protected:
	virtual void MoveLocalAxis(XMFLOAT3& shift, float dTime);
	// 상대적 회전
	virtual void Rotate(float pitch, float yaw, float roll, float dTime);

	void MatchCamera(float dTime);

private:
	CameraComponent* m_cameraComponent;
};