#include "stdafx.h"
#include "first_person_camera.h"
#include "per_player.h"

FirstPersonCamera::FirstPersonCamera(D3D12Camera* camera)
	: D3D12Camera(camera)
{
	m_viewMode = FIRST_PERSON_CAMERA;
	if (camera)
	{
		if (camera->GetMode() == SPACE_SHIP_CAMERA)
		{
			m_up = XMFLOAT3(0.f, 1.f, 0.f);
			m_right.y = 0.f;
			m_look.y = 0.f;
			m_right = Vector3::Normalize(m_right);
			m_look = Vector3::Normalize(m_look);
		}
	}
}

FirstPersonCamera::~FirstPersonCamera()
{
}

void FirstPersonCamera::Rotate(float pitch, float yaw, float roll)
{
	if (pitch != 0.0f)
	{
		// 카메라의 로컬 x축으로 회전
		XMMATRIX rotateMat = XMMatrixRotationAxis(XMLoadFloat3(&m_right), XMConvertToRadians(pitch));
		RotateCameraLocalAxis(rotateMat);
	}
	if (!m_player) return;

	XMFLOAT3 playerRotate = m_player->GetRotation();
	XMFLOAT3 playerPos = m_player->GetPosition();

	if (yaw != 0.0f)
	{
		// 플레이어의 로컬 y축으로 회전
		XMFLOAT3 up = m_player->GetUpVector();
		XMMATRIX rotateMat = XMMatrixRotationAxis(XMLoadFloat3(&up), XMConvertToRadians(yaw));
		RotateCameraLocalAxis(rotateMat);
	}
	if (roll != 0.0f)
	{
	
		// 플레이어의 로컬 z축으로 회전
		XMFLOAT3 look = m_player->GetLookVector();
		XMMATRIX rotateMat = XMMatrixRotationAxis(XMLoadFloat3(&look), XMConvertToRadians(roll));
		// 카메라 위치에서 플레이어 위치를 빼 플레이어 위치를 원점으로한 카메라 위치를 얻음
		m_position = Vector3::Subtract(m_position, playerPos);
		// 플레이어 중심으로 카메라 위치 벡터 회전
		m_position = Vector3::TransformCoord(m_position, rotateMat);
		// 회전시킨 카메라의 위치 벡터에 플레이어 위치를 더해서 카메라의 위치 벡터 계산
		m_position = Vector3::Add(m_position, playerPos);
		
		RotateCameraLocalAxis(rotateMat);
	}
}

void FirstPersonCamera::RotateCameraLocalAxis(XMMATRIX& rotateMat)
{
	m_look = Vector3::TransformNormal(m_look, rotateMat);
	m_right = Vector3::TransformNormal(m_right, rotateMat);
	m_up = Vector3::TransformNormal(m_up, rotateMat);
}
