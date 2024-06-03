#include "stdafx.h"
#include "third_person_camera.h"
#include "per_player.h"

ThirdPersonCamera::ThirdPersonCamera(D3D12Camera* camera)
	: D3D12Camera(camera)
{
	m_viewMode = THIRD_PERSON_CAMERA;
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

ThirdPersonCamera::~ThirdPersonCamera()
{
}

void ThirdPersonCamera::Update(XMFLOAT3& lookAt, float timeElapsed)
{
	if (!m_player) return;

	// 플레이어의 회전 행렬 얻기
	XMFLOAT3 playerRotate = m_player->GetRotation();
	XMFLOAT3 rotateInRadian = XMFLOAT3(XMConvertToRadians(playerRotate.x), XMConvertToRadians(playerRotate.y), XMConvertToRadians(playerRotate.z));
	XMMATRIX roatateMat = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotateInRadian));
	XMFLOAT4X4 rotateMatrix = Matrix4x4::Identity();
	rotateMatrix = Matrix4x4::Multiply(roatateMat, rotateMatrix);
	m_calculatedPlayerUp = XMFLOAT3(rotateMatrix._21, rotateMatrix._22, rotateMatrix._23);

	// 카메라 오프셋 회전
	XMFLOAT3 offSet = Vector3::TransformCoord(m_offSet, roatateMat);

	// 플레이어에 오프셋을 더해 카메라 위치 계산
	XMFLOAT3 playerPos = m_player->GetPosition();
	XMFLOAT3 currentPos = Vector3::Add(playerPos, offSet);
	// 현재 위치와 이전 위치로 계산된 방향으로 카메라 랙 적용
	XMFLOAT3 direction = Vector3::Subtract(currentPos, m_position);
	float length = Vector3::Length(direction);
	direction = Vector3::Normalize(direction);
	// 랙 계산
	float timeLagScale = (m_timeLag) ? timeElapsed * (1.0f / m_timeLag) : 1.0f;
	float distance = length * timeLagScale;
	if (distance > length) distance = length;
	if (length < 0.01f) distance = length;
	// 회전 적용을 하지 않고 위치만 적용
	m_position = Vector3::Add(m_position, direction, distance);
	SetLookAt(lookAt);
}

void ThirdPersonCamera::SetLookAt(XMFLOAT3& lookAt)
{
	// 현재 카메라의 위치에서 플레이어를 바라보기 위한 카메라 변환 행렬 생성
	XMFLOAT4X4 lookAtMat = Matrix4x4::LookAtLH(m_position, lookAt, m_calculatedPlayerUp);
	// 카메라 변환 행렬에서 카메라의 x, y, z축을 구한다.
	m_right = XMFLOAT3(lookAtMat._11, lookAtMat._21, lookAtMat._31);
	m_up = XMFLOAT3(lookAtMat._12, lookAtMat._22, lookAtMat._32);
	m_look = XMFLOAT3(lookAtMat._13, lookAtMat._23, lookAtMat._33);
}
