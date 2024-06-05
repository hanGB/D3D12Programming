#include "stdafx.h"
#include "camera_component.h"
#include "first_person_camera.h"
#include "third_person_camera.h"
#include "space_ship_camera.h"
#include "per_player.h"

CameraComponent::CameraComponent()
{
}

CameraComponent::~CameraComponent()
{
}

void CameraComponent::Initialize()
{
}

void CameraComponent::RotatePlayerAndCamera(float pitch, float yaw, float roll, float dTime)
{
	PERPlayer* player = dynamic_cast<PERPlayer*>(GetOwner());

	XMFLOAT3 rotation = player->GetRotation();
	DWORD cameraMode = m_camera->GetMode();

	if (cameraMode == FIRST_PERSON_CAMERA || cameraMode == THIRD_PERSON_CAMERA)
	{
		if (pitch != 0.0f)
		{
			// 끄덕임
			// x축 중심 회전 각을 -90도~ 90도 안으로 제한
			rotation.x += pitch * dTime;
			if (rotation.x > 89.f)
			{
				pitch = pitch * dTime - rotation.x + 89.f;
				pitch /= dTime;
				rotation.x = 89.f;
			}
			else if (rotation.x < -89.f)
			{
				pitch = pitch * dTime - rotation.x - 89.f;
				pitch /= dTime;
				rotation.x = -89.f;
			}
		}
		if (yaw != 0.0f)
		{
			// 몸통 회전
			rotation.y += yaw * dTime;
			if (rotation.y > 360.0f) rotation.y -= 360.0f;
			if (rotation.y < 0.0f) rotation.y += 360.0f;
		}
		if (roll != 0.0f)
		{
			// 기울임
			// z축 중심 회전 각을 -20도~ 20도 안으로 제한
			rotation.z += roll * dTime;
			if (rotation.z > 19.f)
			{
				roll = roll * dTime - rotation.z + 19.f;
				roll /= dTime;
				rotation.z = 19.f;
			}
			else if (rotation.x < -19.f)
			{
				roll = roll * dTime - rotation.z - 19.f;
				roll /= dTime;
				rotation.z = -19.f;
			}
		}
		// rotation 저장
		player->SetRotation(rotation);

		// 카메라 회전
		m_camera->Rotate(pitch, yaw, roll, dTime);

		// 플레이어 회전
		if (yaw != 0.0f)
		{
			XMFLOAT3 up = player->GetUpVector();
			XMMATRIX rotate = XMMatrixRotationAxis(XMLoadFloat3(&up), XMConvertToRadians(yaw));
			XMFLOAT3 look = player->GetLookVector();
			player->SetLookVector(Vector3::TransformNormal(look, rotate));
			XMFLOAT3 right = GetOwner()->GetRightVector();
			player->SetRightVector(Vector3::TransformNormal(right, rotate));
		}
	}
	else if (cameraMode == SPACE_SHIP_CAMERA)
	{
		// 스페이스 쉽 카메라는 회전에 제한이 없음
		m_camera->Rotate(pitch, yaw, roll, dTime);

		if (pitch != 0.0f)
		{
			XMFLOAT3 right = player->GetRightVector();
			XMMATRIX rotate = XMMatrixRotationAxis(XMLoadFloat3(&right), XMConvertToRadians(pitch * dTime));
			XMFLOAT3 look = player->GetLookVector();
			player->SetLookVector(Vector3::TransformNormal(look, rotate));
			XMFLOAT3 up = GetOwner()->GetUpVector();
			player->SetRightVector(Vector3::TransformNormal(up, rotate));
		}
		if (yaw != 0.0f)
		{
			XMFLOAT3 up = GetOwner()->GetUpVector();
			XMMATRIX rotate = XMMatrixRotationAxis(XMLoadFloat3(&up), XMConvertToRadians(yaw * dTime));
			XMFLOAT3 look = player->GetLookVector();
			player->SetLookVector(Vector3::TransformNormal(look, rotate));
			XMFLOAT3 right = GetOwner()->GetRightVector();
			player->SetRightVector(Vector3::TransformNormal(right, rotate));
		}
		if (roll != 0.0f)
		{
			XMFLOAT3 look = player->GetLookVector();
			XMMATRIX rotate = XMMatrixRotationAxis(XMLoadFloat3(&look), XMConvertToRadians(roll * dTime));
			XMFLOAT3 up = player->GetUpVector();
			player->SetLookVector(Vector3::TransformNormal(up, rotate));
			XMFLOAT3 right = GetOwner()->GetRightVector();
			player->SetRightVector(Vector3::TransformNormal(right, rotate));
		}
	}

	// 회전으로 인해 플레이어의 로컬 축 들이 서로 직교하지 않을 수 있으므로 z축 기준으로 서로 직교하는 단위 벡터로 변경
	XMFLOAT3 look = player->GetLookVector();
	XMFLOAT3 right = player->GetRightVector();
	XMFLOAT3 up = player->GetUpVector();

	look = Vector3::Normalize(look);
	right = Vector3::CrossProduct(up, look, true);
	up = Vector3::CrossProduct(look, right, true);

	player->SetLookVector(look);
	player->SetRightVector(right);
	player->SetUpVector(up);
}

D3D12Camera* CameraComponent::OnChangeCamera(DWORD newCameraMode, DWORD currentCameraMode)
{
	// 카메라 새로 생성
	D3D12Camera* newCamera = NULL;
	switch (newCameraMode)
	{
	case FIRST_PERSON_CAMERA: newCamera = new FirstPersonCamera(m_camera);
		break;
	case THIRD_PERSON_CAMERA: newCamera = new ThirdPersonCamera(m_camera);
		break;
	case SPACE_SHIP_CAMERA: newCamera = new SpaceShipCamera(m_camera);
		break;
	}

	PERPlayer* player = dynamic_cast<PERPlayer*>(GetOwner());

	// 현재 카메라의 모드가 스페이스쉽 카메라일 경우 플레이어의 up벡터를 월드좌표계의 y축 방향으로 설정
	// 다른 카메라의 경우 y축 이동이 불가능하므로 y좌표가 되어야 되므로 right와 look의 y을 0으로 변경
	if (currentCameraMode == SPACE_SHIP_CAMERA)
	{
		XMFLOAT3 look = player->GetLookVector();
		XMFLOAT3 right = player->GetRightVector();
		XMFLOAT3 rotation = player->GetRotation();
		
		XMFLOAT3 up = XMFLOAT3(0.f, 1.f, 0.f);
		up = Vector3::Normalize(up);
		right = XMFLOAT3(right.x, 0.f, right.z);
		right = Vector3::Normalize(right);
		look = XMFLOAT3(look.x, 0.f, look.z);
		look = Vector3::Normalize(look);

		rotation.x = 0.0f;
		rotation.z = 0.0f;

		// look 벡터와 월드 좌표계의 z축이 이루는 각도를 플레이어의 y축의 회전 각도로 설정
		XMFLOAT3 zAxis = XMFLOAT3(0.f, 0.f, 1.0f);
		rotation.y = Vector3::Angle(zAxis, look);
		if (look.x < 0.0f) rotation.y = -rotation.y;

		player->SetLookVector(look);
		player->SetRightVector(right);
		player->SetUpVector(up);
		player->SetRotation(rotation);
	}
	// 새로운 카메라 모드가 스페이스 쉽 모드일 경우 플레이어 로컬 축을 현재 카메라의 로컬 축과 같게 설정
	else if (newCameraMode == SPACE_SHIP_CAMERA && m_camera)
	{
		player->SetLookVector(m_camera->GetLookVector());
		player->SetRightVector(m_camera->GetRightVector());
		player->SetUpVector(m_camera->GetUpVector());
	}

	if (newCamera)
	{
		newCamera->SetMode(newCameraMode);
		newCamera->SetPlayer(player);
	}
	if (m_camera) delete m_camera;

	return newCamera;
}

void CameraComponent::SetCamera(D3D12Camera* camera)
{
	m_camera = camera;
}

D3D12Camera* CameraComponent::GetCamera()
{
	return m_camera;
}

D3D12Camera* CameraComponent::ChangeCamera(DWORD newCameraMode, float deltaTime)
{
	PERLog::Logger().Info("카메라 변경");

	DWORD currentCameraMode = (m_camera) ? m_camera->GetMode() : 0x00;
	if (currentCameraMode == newCameraMode) return m_camera;

	switch (newCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		GetOwner()->GetPhysics().SetFriction(200.0f);
		GetOwner()->GetPhysics().SetGravity(XMFLOAT3(0.f, 0.f, 0.f));
		GetOwner()->GetPhysics().SetMaxVelocity(125.0f, 400.0f);
		m_camera = OnChangeCamera(FIRST_PERSON_CAMERA, currentCameraMode);
		m_camera->SetTimeLag(0.0f);
		m_camera->SetOffSet(XMFLOAT3(0.0f, 20.0f, 0.0f));
		m_camera->GenerateProjectionMatrix(60.f, (float)PER_DEFAULT_WINDOW_WIDTH / (float)PER_DEFAULT_WINDOW_HEIGHT, 1.0f, 5000.0f);
		m_camera->SetViewport(0, 0, PER_DEFAULT_WINDOW_WIDTH, PER_DEFAULT_WINDOW_HEIGHT);
		m_camera->SetScissorRect(0, 0, PER_DEFAULT_WINDOW_WIDTH, PER_DEFAULT_WINDOW_HEIGHT);
		break;
	case SPACE_SHIP_CAMERA:
		GetOwner()->GetPhysics().SetFriction(150.0f);
		GetOwner()->GetPhysics().SetGravity(XMFLOAT3(0.f, 0.f, 0.f));
		GetOwner()->GetPhysics().SetMaxVelocity(400.0f, 400.0f);
		m_camera = OnChangeCamera(SPACE_SHIP_CAMERA, currentCameraMode);
		m_camera->SetTimeLag(0.0f);
		m_camera->SetOffSet(XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_camera->GenerateProjectionMatrix(60.f, (float)PER_DEFAULT_WINDOW_WIDTH / (float)PER_DEFAULT_WINDOW_HEIGHT, 1.0f, 5000.0f);
		m_camera->SetViewport(0, 0, PER_DEFAULT_WINDOW_WIDTH, PER_DEFAULT_WINDOW_HEIGHT);
		m_camera->SetScissorRect(0, 0, PER_DEFAULT_WINDOW_WIDTH, PER_DEFAULT_WINDOW_HEIGHT);
		break;
	case THIRD_PERSON_CAMERA:
		GetOwner()->GetPhysics().SetFriction(250.0f);
		GetOwner()->GetPhysics().SetGravity(XMFLOAT3(0.f, 0.f, 0.f));
		GetOwner()->GetPhysics().SetMaxVelocity(125.0f, 400.0f);
		m_camera = OnChangeCamera(THIRD_PERSON_CAMERA, currentCameraMode);
		m_camera->SetTimeLag(0.25f);
		m_camera->SetOffSet(XMFLOAT3(0.0f, 20.0f, -50.0f));
		m_camera->GenerateProjectionMatrix(60.f, (float)PER_DEFAULT_WINDOW_WIDTH / (float)PER_DEFAULT_WINDOW_HEIGHT, 1.0f, 5000.0f);
		m_camera->SetViewport(0, 0, PER_DEFAULT_WINDOW_WIDTH, PER_DEFAULT_WINDOW_HEIGHT);
		m_camera->SetScissorRect(0, 0, PER_DEFAULT_WINDOW_WIDTH, PER_DEFAULT_WINDOW_HEIGHT);
		break;
	}

	m_camera->SetPosition(Vector3::Add(GetOwner()->GetPosition(), m_camera->GetOffSet()));

	return m_camera;
}
