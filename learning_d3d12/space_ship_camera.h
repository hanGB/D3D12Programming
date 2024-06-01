#pragma once
#include "d3d12_camera.h"

class SpaceShipCamera : public D3D12Camera {
public:
	SpaceShipCamera(D3D12Camera* camera);
	virtual ~SpaceShipCamera();

	virtual void Rotate(float pitch = 0.0f, float yaw = 0.0f, float roll = 0.0f);

private:
	void UpdateWithRotateMatrix(XMMATRIX& rotateMat);
};