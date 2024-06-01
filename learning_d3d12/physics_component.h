#pragma once
#include "per_component.h"
#include "d3d12_mesh.h"
#include "d3d12_shader.h"
#include "d3d12_camera.h"

class PhysicsComponent : public PERComponent {
public:
	PhysicsComponent();
	virtual ~PhysicsComponent();
	virtual void Initialize();

	virtual void Update(float dTime);

private:
	// 상대적 변화
	// 로컬 축으로 이동
	void MoveStafe(float distance);
	void MoveUp(float distance);
	void MoveForward(float distance);
	// 상대적 회전
	void Rotate(float pitch, float yaw, float roll);
};