#pragma once
#include "d3d12_camera.h"

class FirstPersonCamera : public D3D12Camera {
public:
	FirstPersonCamera(D3D12Camera* camera);
	virtual ~FirstPersonCamera();

	virtual void Rotate(float pitch = 0.0f, float yaw = 0.0f, float roll = 0.0f);

private:
	void RotateCameraLocalAxis(XMMATRIX& rotateMat);
};