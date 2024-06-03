#pragma once
#include "d3d12_camera.h"

class FirstPersonCamera : public D3D12Camera {
public:
	FirstPersonCamera(D3D12Camera* camera);
	virtual ~FirstPersonCamera();

	virtual void Rotate(float pitch, float yaw, float roll, float dTime);

private:
	void RotateCameraLocalAxis(XMMATRIX& rotateMat);
};