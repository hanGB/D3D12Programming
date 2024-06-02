#include "stdafx.h"
#include "per_object.h"

PERObject::PERObject(InputComponent* input, AiComponent* ai, PhysicsComponent* physics, GraphicsComponent* graphics)
	: m_input(input), m_ai(ai), m_physics(physics), m_graphics(graphics)
{
	m_input->SetOwner(this);
	m_ai->SetOwner(this);
	m_physics->SetOwner(this);
	m_graphics->SetOwner(this);
}

PERObject::~PERObject()
{
	delete m_input;
	delete m_ai;
	delete m_graphics;
	delete m_physics;
}

void PERObject::Initialize()
{
	m_worldTransform = Matrix4x4::Identity();

	SetPosition(XMFLOAT3(0.f, 0.f, 0.f));
	SetScale(XMFLOAT3(1.f, 1.f, 1.f));
	SetRotation(XMFLOAT3(0.f, 0.f, 0.f));

	m_input->Initialize();
	m_ai->Initialize();
	m_physics->Initialize();
	m_graphics->Initialize();
}

InputComponent& PERObject::GetInput()
{
	return *m_input;
}

AiComponent& PERObject::GetAi()
{
	return *m_ai;
}

GraphicsComponent& PERObject::GetGraphics()
{
	return *m_graphics;
}

PhysicsComponent& PERObject::GetPhysics()
{
	return *m_physics;
}

void PERObject::AddComponent(PERComponent* component)
{
	if (!m_component) 
	{
		m_component = component;
		return;
	}

	// 이미 컨포넌트가 있을 경우 그 컨포넌트의 다음으로 설정
	while (true) 
	{
		PERComponent* next = m_component;
		if (!next)
		{
			next = component;
			return;
		}
		next = next->GetNext();
	}
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

XMFLOAT3 PERObject::GetLocalAsixForce() const
{
	return m_localAsixForce;
}

XMFLOAT3 PERObject::GetWorldAsixForce() const
{
	return m_worldAsixForce;
}

XMFLOAT3 PERObject::GetRotateForce() const
{
	return m_rotateForce;
}

void PERObject::SetPosition(XMFLOAT3 position)
{
	m_position = position;
	m_worldTransform._41 = position.x;
	m_worldTransform._42 = position.y;
	m_worldTransform._43 = position.z;
}

void PERObject::SetScale(XMFLOAT3 scale)
{
	m_scale = scale;
	// 충돌 계산 제외 아무런 영향이 없음으로
	// 월드 변환 변경 필요 없음
}

void PERObject::SetRotation(XMFLOAT3 rotation)
{
	m_rotation = rotation;

	// 회전 값이 변경되었으므로 로컬 축이 전체 변경됨
	// 월드 변환 초기화
	m_worldTransform = Matrix4x4::Identity();

	// 회전 변환 적용
	// 라디안으로 변환
	XMFLOAT3 radianRotation = XMFLOAT3(XMConvertToRadians(m_rotation.x), XMConvertToRadians(m_rotation.y), XMConvertToRadians(m_rotation.z));
	XMMATRIX rotateMatrix = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&radianRotation));
	m_worldTransform = Matrix4x4::Multiply(m_worldTransform, rotateMatrix);

	// 위치 적용
	SetPosition(m_position);
}

void PERObject::SetLocalAsixForce(XMFLOAT3 force)
{
	m_localAsixForce = force;
}

void PERObject::SetWorldAsixForce(XMFLOAT3 force)
{
	m_worldAsixForce = force;
}

void PERObject::SetRotateForce(XMFLOAT3 force)
{
	m_rotateForce = force;
}

XMFLOAT3 PERObject::GetLookVector()
{
	XMFLOAT3 look = XMFLOAT3(m_worldTransform._31, m_worldTransform._32, m_worldTransform._33);
	return Vector3::Normalize(look);
}

XMFLOAT3 PERObject::GetUpVector()
{
	XMFLOAT3 up = XMFLOAT3(m_worldTransform._21, m_worldTransform._22, m_worldTransform._23);
	return Vector3::Normalize(up);
}

XMFLOAT3 PERObject::GetRightVector()
{
	XMFLOAT3 look = XMFLOAT3(m_worldTransform._11, m_worldTransform._12, m_worldTransform._13);
	return Vector3::Normalize(look);
}

XMFLOAT4X4 PERObject::GetWorldTransform()
{
	// 최종적으로 크기 변환을 추가	
	XMMATRIX scaleMatrix = XMMatrixScalingFromVector(XMLoadFloat3(&m_scale));

	XMFLOAT4X4 finalWorld = Matrix4x4::Multiply(m_worldTransform, scaleMatrix);
	
	return finalWorld;
}
