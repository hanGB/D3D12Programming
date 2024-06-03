#pragma once
#include "d3d12_camera.h"

class ThirdPersonCamera : public D3D12Camera {
public:
	ThirdPersonCamera(D3D12Camera* camera);
	virtual ~ThirdPersonCamera();

	virtual void Update(XMFLOAT3& lookAt, float timeElapsed);
	virtual void SetLookAt(XMFLOAT3& lookAt);

private:
	XMFLOAT3 m_calculatedPlayerUp;
};