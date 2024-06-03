#pragma once
#include "per_component.h"

class PhysicsComponent : public PERComponent {
public:
	PhysicsComponent();
	virtual ~PhysicsComponent();
	virtual void Initialize();

	virtual void Update(float dTime);

	void SetGravity(XMFLOAT3 gravity);
	void SetFriction(float friction);
	void SetMaxVelocity(float xz, float y);

protected:
	// 상대적 이동
	// 월드 축으로 이동
	virtual void MoveWorldAxis(float xDistance, float yDistance, float zDistance);

	// 로컬 축으로 이동
	virtual void MoveLocalAxis(float xDistance, float yDistance, float zDistance);

	void MoveStafe(float distance);
	void MoveUp(float distance);
	void MoveForward(float distance);

	// 상대적 회전
	virtual void Rotate(float pitch, float yaw, float roll);

	// 중력
	XMFLOAT3 m_gravity;
	// 마찰력
	float m_friction;
	// 각 축에 대한 최고 속도
	float m_fMaxVelocityXZ;
	float m_fMaxVelocityY;
};