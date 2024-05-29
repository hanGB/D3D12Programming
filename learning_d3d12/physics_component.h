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

	XMMATRIX Rotate(XMFLOAT3 axis, float angle);
};