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
	
	MatchCamera(dTime);
}

void PlayerPhysics::MoveLocalAxis(XMFLOAT3& shift, float dTime)
{

	XMFLOAT3 look = GetOwner()->GetLookVector();
	XMFLOAT3 right = GetOwner()->GetRightVector();
	XMFLOAT3 up = GetOwner()->GetUpVector();

	XMFLOAT3 worldShift = XMFLOAT3(0.f, 0.f, 0.f);

	worldShift = Vector3::Add(worldShift, look, shift.z);
	worldShift = Vector3::Add(worldShift, right, shift.x);
	worldShift = Vector3::Add(worldShift, up, shift.y);

	MoveWorldAxis(worldShift, dTime);
	CameraComponent* cameraComponent = GetOwner()->GetComponentWithType<CameraComponent>();
	cameraComponent->GetCamera()->Move(worldShift, dTime);
}

void PlayerPhysics::Rotate(float pitch, float yaw, float roll, float dTime)
{

}

void PlayerPhysics::MatchCamera(float dTime)
{
	// 변경된 물리값과 카메라 맞춤
	CameraComponent* cameraComponent = GetOwner()->GetComponentWithType<CameraComponent>();
	if (!cameraComponent) return;

	D3D12Camera* camera = cameraComponent->GetCamera();
	XMFLOAT3 pos = GetOwner()->GetPosition();
	if (camera->GetMode() == THIRD_PERSON_CAMERA) camera->Update(pos, dTime);
	if (camera->GetMode() == THIRD_PERSON_CAMERA) camera->SetLookAt(pos);

	camera->RegenerateViewMatrix();
}


