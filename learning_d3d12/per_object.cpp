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

XMFLOAT4X4 PERObject::GetModelTransform() const
{
	return m_modelTransform;
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

void PERObject::SetModelTransform(XMFLOAT4X4 modelTransform)
{
	m_modelTransform = modelTransform;
}
