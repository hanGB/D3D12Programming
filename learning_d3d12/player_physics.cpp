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
	PhysicsComponent::Update(dTime);


	CameraComponent* cameraComponent = GetOwner()->GetComponentWithType<CameraComponent>();
	if (!cameraComponent) return;
	
	D3D12Camera* camera = cameraComponent->GetCamera();
	XMFLOAT3 pos = GetOwner()->GetPosition();
	if (camera->GetMode() == THIRD_PERSON_CAMERA) camera->Update(pos, dTime);
	if (camera->GetMode() == THIRD_PERSON_CAMERA) camera->SetLookAt(pos);

	camera->RegenerateViewMatrix();
}

void PlayerPhysics::MoveLocalAxis(float xDistance, float yDistance, float zDistance)
{
	XMFLOAT3 look = GetOwner()->GetLookVector();
	XMFLOAT3 right = GetOwner()->GetRightVector();
	XMFLOAT3 up = GetOwner()->GetUpVector();

	PERLog::Logger().InfoWithFormat("%f, %f, %f", xDistance, yDistance, zDistance);
	XMFLOAT3 shift = XMFLOAT3(0.f, 0.f, 0.f);

	shift = Vector3::Add(shift, look, zDistance);
	shift = Vector3::Add(shift, right, xDistance);
	shift = Vector3::Add(shift, up, yDistance);

	PERLog::Logger().InfoWithFormat("%f, %f, %f", shift.x, shift.y, shift.z);

	MoveWorldAxis(shift.x, shift.y, shift.z);
	CameraComponent* cameraComponent = GetOwner()->GetComponentWithType<CameraComponent>();
	cameraComponent->GetCamera()->Move(shift);
}

void PlayerPhysics::Rotate(float pitch, float yaw, float roll)
{
	CameraComponent* cameraComponent = GetOwner()->GetComponentWithType<CameraComponent>();
	if (cameraComponent) cameraComponent->RotatePlayerAndCamera(pitch, yaw, roll);
}
