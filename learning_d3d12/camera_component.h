#pragma once
#include "per_component.h"

class D3D12Camera;

class CameraComponent : public PERComponent {
public:
	CameraComponent();
	virtual ~CameraComponent();
	virtual void Initialize();

	// 플레이어와 카메라 회전
	void RotatePlayerAndCamera(float pitch, float yaw, float roll);

	void SetCamera(D3D12Camera* camera);
	D3D12Camera* GetCamera();

	D3D12Camera* ChangeCamera(DWORD newCameraMode, float deltaTime);

private:
	// 카메라 변경
	D3D12Camera* OnChangeCamera(DWORD newCameraMode, DWORD currentCameraMode);

	D3D12Camera* m_camera = nullptr;
};