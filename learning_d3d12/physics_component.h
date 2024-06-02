#pragma once
#include "per_component.h"

class PhysicsComponent : public PERComponent {
public:
	PhysicsComponent();
	virtual ~PhysicsComponent();
	virtual void Initialize();

	virtual void Update(float dTime);

private:
	// 상대적 이동
	// 월드 축으로 이동
	void MoveWorldAxis(float xDistance, float yDistance, float zDistance);
	// 로컬 축으로 이동
	void MoveStafe(float distance);
	void MoveUp(float distance);
	void MoveForward(float distance);

	// 상대적 회전
	void Rotate(float pitch, float yaw, float roll);

	// 중력
	XMFLOAT3 m_gravity;
	// 마찰력
	float m_friction;
	// 각 축에 대한 최고 속도
	float m_fMaxVelocityXZ;
	float m_fMaxVelocityY;
};