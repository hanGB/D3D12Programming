#include "stdafx.h"
#include "camera_component.h"

CameraComponent::CameraComponent()
{
}

CameraComponent::~CameraComponent()
{
}

void CameraComponent::Initialize()
{
}

void CameraComponent::SetCamera(D3D12Camera* camera)
{
	m_camera = camera;
}

D3D12Camera* CameraComponent::GetCamera()
{
	return m_camera;
}
