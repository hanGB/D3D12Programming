#pragma once
#include "per_component.h"
#include "d3d12_camera.h"

class CameraComponent : public PERComponent {
public:
	CameraComponent();
	virtual ~CameraComponent();
	virtual void Initialize();

	void SetCamera(D3D12Camera* camera);
	D3D12Camera* GetCamera();

private:
	D3D12Camera* m_camera = nullptr;
};