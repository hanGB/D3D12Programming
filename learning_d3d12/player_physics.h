#pragma once
#include "physics_component.h"

class PlayerPhysics: public PhysicsComponent{
public:
	PlayerPhysics();
	virtual ~PlayerPhysics();
	virtual void Initialize();

	virtual void Update(float dTime);

protected:
	virtual void MoveLocalAxis(float xDistance, float yDistance, float zDistance);
	// 상대적 회전
	virtual void Rotate(float pitch, float yaw, float roll);
};