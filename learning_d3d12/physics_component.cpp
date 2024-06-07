#include "stdafx.h"
#include "physics_component.h"
#include "per_object.h"

PhysicsComponent::PhysicsComponent()
{
}

PhysicsComponent::~PhysicsComponent()
{

}

void PhysicsComponent::Initialize()
{
	PERComponent::Initialize();
}

void PhysicsComponent::Update(float dTime)
{
	// 이동
	// 월드 좌표계
	XMFLOAT3 worldForce = GetOwner()->GetWorldAsixForce();
	MoveWorldAxis(worldForce, dTime);

	// 로컬 좌표계
	XMFLOAT3 localForce = GetOwner()->GetLocalAsixForce();
	MoveLocalAxis(localForce, dTime);

	// 회전
	XMFLOAT3 rotateForce = GetOwner()->GetRotateForce();
	Rotate(rotateForce.x, rotateForce.y, rotateForce.z, dTime);

	UseVelocity(dTime);

	if (GetNext()) dynamic_cast<PhysicsComponent*>(GetNext())->Update(dTime);
}

void PhysicsComponent::SetGravity(XMFLOAT3 gravity)
{
	m_gravity = gravity;
}

void PhysicsComponent::SetFriction(float friction)
{
	m_friction = friction;
}

void PhysicsComponent::SetMaxVelocity(float xz, float y)
{
	m_maxVelocityXZ = xz;
	m_maxVelocityY = y;
}

void PhysicsComponent::MoveWorldAxis(XMFLOAT3& shift, float dTime)
{
	XMFLOAT3 pos = GetOwner()->GetPosition();
	pos.x += shift.x * dTime;
	pos.y += shift.y * dTime;
	pos.z += shift.z * dTime;

	GetOwner()->SetPosition(pos);
}

void PhysicsComponent::MoveLocalAxis(XMFLOAT3& shift, float dTime)
{
	MoveForward(shift.x * dTime);
	MoveStafe(shift.y * dTime);
	MoveUp(shift.z * dTime);
}

void PhysicsComponent::MoveStafe(float distance)
{
	XMFLOAT3 pos = GetOwner()->GetPosition();
	XMFLOAT3 right = GetOwner()->GetRightVector();
	pos = Vector3::Add(pos, right, distance);

	GetOwner()->SetPosition(pos);
}

void PhysicsComponent::MoveUp(float distance)
{
	XMFLOAT3 pos = GetOwner()->GetPosition();
	XMFLOAT3 up = GetOwner()->GetUpVector();
	pos = Vector3::Add(pos, up, distance);

	GetOwner()->SetPosition(pos);
}

void PhysicsComponent::MoveForward(float distance)
{
	XMFLOAT3 pos = GetOwner()->GetPosition();
	XMFLOAT3 look = GetOwner()->GetLookVector();
	pos = Vector3::Add(pos, look, distance);

	GetOwner()->SetPosition(pos);
}

void PhysicsComponent::Rotate(float pitch, float yaw, float roll, float dTime)
{
	XMFLOAT3 rotation = GetOwner()->GetRotation();
	rotation.x += pitch * dTime;
	rotation.y += yaw * dTime;
	rotation.z += roll * dTime;

	GetOwner()->SetRotation(rotation);
}

void PhysicsComponent::UseVelocity(float dTime)
{
	XMFLOAT3 velocity = GetVelocityByApplyingVariousPhysicalValues(dTime);

	XMFLOAT3 shift = Vector3::ScalarProduct(velocity, dTime, false);
	MoveWorldAxis(velocity, dTime);

	GetOwner()->SetVelocity(velocity);
}

XMFLOAT3 PhysicsComponent::GetVelocityByApplyingVariousPhysicalValues(float dTime)
{
	XMFLOAT3 velocity = GetOwner()->GetVelocity();
	if (velocity.x == 0.0f && velocity.y == 0.0f && velocity.z == 0.0f) return velocity;

	// 중력 추가
	velocity = Vector3::Add(velocity, Vector3::ScalarProduct(m_gravity, dTime, false));

	// 마찰력에 의해 감속
	float length = Vector3::Length(velocity);
	float deceleration = (m_friction * dTime);
	if (deceleration > length) deceleration = length;
	velocity = Vector3::Add(velocity, Vector3::ScalarProduct(velocity, -deceleration, true));

	// xz축의 속도의 크기를 얻어 최대 속도 안으로 변경
	length = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);
	if (length > m_maxVelocityXZ)
	{
		velocity.x *= (m_maxVelocityXZ / length);
		velocity.z *= (m_maxVelocityXZ / length);
	}

	// y축의 속도의 크기를 얻어 최대 속도 안으로 변경
	float maxVelocityY = m_maxVelocityY * dTime;
	length = sqrtf(velocity.y * velocity.y);
	if (length > m_maxVelocityY)
	{
		velocity.y *= (maxVelocityY / length);
	}

	return velocity;
}
