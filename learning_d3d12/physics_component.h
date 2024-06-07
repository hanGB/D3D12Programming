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
	virtual void MoveWorldAxis(XMFLOAT3& shift, float dTime);

	// 로컬 축으로 이동
	virtual void MoveLocalAxis(XMFLOAT3& shift, float dTime);

	void MoveStafe(float distance);
	void MoveUp(float distance);
	void MoveForward(float distance);

	// 상대적 회전
	virtual void Rotate(float pitch, float yaw, float roll, float dTime);

	// 속도 사용
	virtual void UseVelocity(float dTime);

	// 각종 물리 값 적용
	XMFLOAT3 GetVelocityByApplyingVariousPhysicalValues(float dTime);

	// 중력
	XMFLOAT3 m_gravity;
	// 마찰력
	float m_friction;
	// 각 축에 대한 최고 속도
	float m_maxVelocityXZ;
	float m_maxVelocityY;
};