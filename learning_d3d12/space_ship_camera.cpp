#include "stdafx.h"
#include "space_ship_camera.h"
#include "per_object.h"

SpaceShipCamera::SpaceShipCamera(D3D12Camera* camera)
	: D3D12Camera(camera)
{
	m_viewMode = SPACE_SHIP_CAMERA;
}

SpaceShipCamera::~SpaceShipCamera()
{
}

void SpaceShipCamera::Rotate(float pitch, float yaw, float roll)
{
	if (!m_player) return;

	if (pitch != 0.0f)
	{
		// 플레이어의 로컬 x축에 대한 pitch 회전 행렬 계산
		XMFLOAT3 right = m_player->GetRightVector();
		XMMATRIX rotateMat = XMMatrixRotationAxis(XMLoadFloat3(&right), XMConvertToRadians(pitch));
		// 회전 변환 행렬로 카메라 값 업데이트
		UpdateWithRotateMatrix(rotateMat);
	}
	if (yaw != 0.0f)
	{
		// 플레이어의 로컬 y축에 대한 yaw 회전 행렬 계산
		XMFLOAT3 up = m_player->GetUpVector();
		XMMATRIX rotateMat = XMMatrixRotationAxis(XMLoadFloat3(&up), XMConvertToRadians(yaw));
		// 회전 변환 행렬로 카메라 값 업데이트
		UpdateWithRotateMatrix(rotateMat);
	}
	if (roll != 0.0f)
	{
		// 플레이어의 로컬 z축에 대한 roll 회전 행렬 계산
		XMFLOAT3 look = m_player->GetLookVector();
		XMMATRIX rotateMat = XMMatrixRotationAxis(XMLoadFloat3(&look), XMConvertToRadians(roll));
		// 회전 변환 행렬로 카메라 값 업데이트
		UpdateWithRotateMatrix(rotateMat);
	}
}

void SpaceShipCamera::UpdateWithRotateMatrix(XMMATRIX& rotateMat)
{
	XMFLOAT3 playerPos = m_player->GetPosition();

	// 카메라의 로컬 x, y, z축 회전
	m_right = Vector3::TransformNormal(m_right, rotateMat);
	m_up = Vector3::TransformNormal(m_up, rotateMat);
	m_look = Vector3::TransformNormal(m_look, rotateMat);
	// 카메라 위치에서 플레이어 위치를 빼 플레이어 위치를 원점으로한 카메라 위치를 얻음
	m_position = Vector3::Subtract(m_position, playerPos);
	// 플레이어 중심으로 카메라 위치 벡터 회전
	m_position = Vector3::TransformCoord(m_position, rotateMat);
	// 회전시킨 카메라의 위치 벡터에 플레이어 위치를 더해서 카메라의 위치 벡터 계산
	m_position = Vector3::Add(m_position, playerPos);
}
