#include "stdafx.h"
#include "player_physics.h"
#include "per_player.h"
#include "camera_component.h"

PlayerPhysics::PlayerPhysics()
{
}

PlayerPhysics::~PlayerPhysics()
{
}

void PlayerPhysics::Initialize()
{
	PhysicsComponent::Initialize();
}

void PlayerPhysics::Update(float dTime)
{
	m_cameraComponent = GetOwner()->GetComponentWithType<CameraComponent>();

	PhysicsComponent::Update(dTime);
	
	MatchCamera(dTime);
}

void PlayerPhysics::MoveLocalAxis(XMFLOAT3& shift, float dTime)
{
	XMFLOAT3 look = GetOwner()->GetLookVector();
	XMFLOAT3 right = GetOwner()->GetRightVector();
	XMFLOAT3 up = GetOwner()->GetUpVector();

	XMFLOAT3 addVelocity = XMFLOAT3(0.f, 0.f, 0.f);

	addVelocity = Vector3::Add(addVelocity, look, shift.z);
	addVelocity = Vector3::Add(addVelocity, right, shift.x);
	addVelocity = Vector3::Add(addVelocity, up, shift.y);

	XMFLOAT3 velocity = GetOwner()->GetVelocity();
	velocity = Vector3::Add(velocity, Vector3::ScalarProduct(addVelocity, dTime, false));
	GetOwner()->SetVelocity(velocity);
}

void PlayerPhysics::Rotate(float pitch, float yaw, float roll, float dTime)
{
	if (m_cameraComponent) m_cameraComponent->RotatePlayerAndCamera(pitch, yaw, roll, dTime);
}

void PlayerPhysics::UseVelocity(float dTime)
{
	XMFLOAT3 velocity = GetVelocityByApplyingVariousPhysicalValues(dTime);

	XMFLOAT3 shift = Vector3::ScalarProduct(velocity, dTime, false);
	MoveWorldAxis(velocity, dTime);
	if (m_cameraComponent) m_cameraComponent->GetCamera()->Move(velocity, dTime);

	GetOwner()->SetVelocity(velocity);
}

void PlayerPhysics::MatchCamera(float dTime)
{
	// 변경된 물리값과 카메라 맞춤
	if (!m_cameraComponent) return;

	D3D12Camera* camera = m_cameraComponent->GetCamera();
	XMFLOAT3 pos = GetOwner()->GetPosition();
	if (camera->GetMode() == THIRD_PERSON_CAMERA) 
	{
		camera->Update(pos, dTime);
		camera->SetLookAt(pos);
	}

	camera->RegenerateViewMatrix();
}


