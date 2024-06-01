#pragma once
#include "graphics_component.h"
#include "physics_component.h"

class PERObject {
public:
	PERObject(PhysicsComponent* physics, GraphicsComponent* graphics);
	~PERObject();

	void Initialize();

	GraphicsComponent& GetGraphics();
	PhysicsComponent& GetPhysics();

	// 기본 정보 getter, setter
	XMFLOAT3 GetPosition() const;
	XMFLOAT3 GetScale() const;
	XMFLOAT3 GetRotation() const;
	void SetPosition(XMFLOAT3 position);
	void SetScale(XMFLOAT3 scale);
	void SetRotation(XMFLOAT3 rotation);

	// 로컬 축 정보
	XMFLOAT3 GetLookVector();
	XMFLOAT3 GetUpVector();
	XMFLOAT3 GetRightVector();

	XMFLOAT4X4 GetWorldTransform();

protected:
	GraphicsComponent* m_graphics = nullptr;
	PhysicsComponent* m_physics = nullptr;

	// 크기 변환이 제외된 월드 변환(실제 렌더링 할 때 크기 변환을 곱해서 넘김)
	XMFLOAT4X4 m_worldTransform = Matrix4x4::Identity();
	
	XMFLOAT3 m_position = { 0.f, 0.f, 0.f };
	XMFLOAT3 m_scale = { 1.f, 1.f, 1.f };
	XMFLOAT3 m_rotation = { 0.f, 0.f, 0.f };
};