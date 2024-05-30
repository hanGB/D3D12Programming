#pragma once
#include "graphics_component.h"
#include "physics_component.h"

class PERObject {
public:
	PERObject(PhysicsComponent* physics, GraphicsComponent* graphics);
	~PERObject();

	GraphicsComponent& GetGraphics();
	PhysicsComponent& GetPhysics();

	XMFLOAT3 GetPosition() const;
	XMFLOAT3 GetScale() const;
	XMFLOAT3 GetRotation() const;
	XMFLOAT4X4 GetModelTransform();

	void SetPosition(XMFLOAT3 position);
	void SetScale(XMFLOAT3 scale);
	void SetRotation(XMFLOAT3 rotation);

private:
	GraphicsComponent* m_graphics = nullptr;
	PhysicsComponent* m_physics = nullptr;

	XMFLOAT4X4 m_modelTransform = Matrix4x4::Identity();
	
	XMFLOAT3 m_position = { 0.f, 0.f, 0.f };
	XMFLOAT3 m_scale = { 1.f, 1.f, 1.f };
	XMFLOAT3 m_rotation = { 0.f, 0.f, 0.f };
};