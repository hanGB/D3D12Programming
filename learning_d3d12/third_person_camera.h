#pragma once
#include "d3d12_camera.h"

class ThridPersonCamera : public D3D12Camera {
public:
	ThridPersonCamera(D3D12Camera* camera);
	virtual ~ThridPersonCamera();

	virtual void Update(XMFLOAT3& lookAt, float timeElapsed);
	virtual void SetLookAt(XMFLOAT3& lookAt);

private:
	XMFLOAT3 m_calculatedPlayerUp;
};