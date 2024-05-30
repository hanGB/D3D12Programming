#include "stdafx.h"
#include "per_object.h"

PERObject::PERObject(PhysicsComponent* physics, GraphicsComponent* graphics)
	: m_physics(physics), m_graphics(graphics)
{
	m_physics->SetOwner(this);
	m_graphics->SetOwner(this);
}

PERObject::~PERObject()
{
	delete m_graphics;
	delete m_physics;
}

GraphicsComponent& PERObject::GetGraphics()
{
	return *m_graphics;
}

PhysicsComponent& PERObject::GetPhysics()
{
	return *m_physics;
}

XMFLOAT3 PERObject::GetPosition() const
{
	return m_position;
}

XMFLOAT3 PERObject::GetScale() const
{
	return m_scale;
}

XMFLOAT3 PERObject::GetRotation() const
{
	return m_rotation;
}

XMFLOAT4X4 PERObject::GetModelTransform()
{
	XMFLOAT4X4 finalModel;
	m_modelTransform = Matrix4x4::Identity();
	XMFLOAT3 radianRotation = XMFLOAT3(XMConvertToRadians(m_rotation.x), XMConvertToRadians(m_rotation.y), XMConvertToRadians(m_rotation.z));

	// 위치, 회전, 크기 변환 행렬 계산
	XMMATRIX tranMatrix = XMMatrixTranslationFromVector(XMLoadFloat3(&m_position));
	XMMATRIX rotateMatrix = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&radianRotation));
	XMMATRIX scaleMatrix = XMMatrixScalingFromVector(XMLoadFloat3(&m_scale));

	// 계산
	finalModel = Matrix4x4::Multiply(m_modelTransform, scaleMatrix);
	finalModel = Matrix4x4::Multiply(finalModel, rotateMatrix);
	finalModel = Matrix4x4::Multiply(finalModel, tranMatrix);
	
	return finalModel;
}

void PERObject::SetPosition(XMFLOAT3 position)
{
	m_position = position;
}

void PERObject::SetScale(XMFLOAT3 scale)
{
	m_scale = scale;
}

void PERObject::SetRotation(XMFLOAT3 rotation)
{
	m_rotation = rotation;
}
