#pragma once

struct VS_CB_CAMERA_INFO
{
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
};

class D3D12Camera
{
public:
	D3D12Camera(int width, int height);
	~D3D12Camera();

	virtual void CreeateShaderVariables(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
	virtual void ReleaseShderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* commandList);

	void GenerateViewMatrix(XMFLOAT3 position, XMFLOAT3 lookAt, XMFLOAT3 up);
	void GenerateProjectionMatrix(float fovAngle, float aspectRatio, float nearPlaneDistance, float farPlaneDistance);

	void SetViewport(int xTopLeft, int yTopLeft, int width, int height, float minZ = 0.0f, float maxZ = 1.0f);
	void SetScissorRect(LONG left, LONG top, LONG right, LONG bottom);

	virtual void SetViewportsAndScissorRect(ID3D12GraphicsCommandList* commandList);

protected:
	// 카메라 변환 행렬
	XMFLOAT4X4 m_view;
	// 투영 변환 행렬
	XMFLOAT4X4 m_projection;

	// 뷰포트, 씨저 사각형
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;

	float m_aspectRatio;
};