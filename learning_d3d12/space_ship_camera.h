#pragma once
#include "d3d12_camera.h"

class SpaceShipCamera : public D3D12Camera {
public:
	SpaceShipCamera(D3D12Camera* camera);
	virtual ~SpaceShipCamera();

	virtual void Rotate(float pitch, float yaw, float roll, float dTime);

private:
	void RotateCameraLocalAxisAndPosition(XMMATRIX& rotateMat);
};