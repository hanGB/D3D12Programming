#include "stdafx.h"
#include "d3d12_camera.h"

D3D12Camera::D3D12Camera(int width, int height)
{
	m_view = Matrix4x4::Identity();
	m_projection = Matrix4x4::Identity();

	m_aspectRatio = (float)width / (float)height;
	m_viewport = { 0, 0, (float)width, (float)height, 0.0, 1.0f };
	m_scissorRect = { 0, 0, (LONG)width, (LONG)height };
}

D3D12Camera::~D3D12Camera()
{
}

void D3D12Camera::CreeateShaderVariables(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{

}

void D3D12Camera::ReleaseShderVariables()
{
}

void D3D12Camera::UpdateShaderVariables(ID3D12GraphicsCommandList* commandList)
{
	XMFLOAT4X4 view = Matrix4x4::Transpose(m_view);
	XMFLOAT4X4 projection = Matrix4x4::Transpose(m_projection);

	// 루트 파라미터 인덱스 1에 저장
	commandList->SetGraphicsRoot32BitConstants(1, 16, &view, 0);
	commandList->SetGraphicsRoot32BitConstants(1, 16, &projection, 16);

}

void D3D12Camera::GenerateViewMatrix(XMFLOAT3 position, XMFLOAT3 lookAt, XMFLOAT3 up)
{
	m_view = Matrix4x4::LookAtLH(position, lookAt, up);
}

void D3D12Camera::GenerateProjectionMatrix(float fovAngle, float aspectRatio, float nearPlaneDistance, float farPlaneDistance)
{
	m_projection = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(fovAngle), aspectRatio, nearPlaneDistance, farPlaneDistance);
}

void D3D12Camera::SetViewport(int xTopLeft, int yTopLeft, int width, int height, float minZ, float maxZ)
{
	m_viewport.TopLeftX = (float)xTopLeft;
	m_viewport.TopLeftY = (float)yTopLeft;
	m_viewport.Width = (float)width;
	m_viewport.Height = (float)height;
	m_viewport.MinDepth = minZ;
	m_viewport.MaxDepth = maxZ;
}

void D3D12Camera::SetScissorRect(LONG left, LONG top, LONG right, LONG bottom)
{
	m_scissorRect.left = left;
	m_scissorRect.top = top;
	m_scissorRect.right = right;
	m_scissorRect.bottom = bottom;
}

void D3D12Camera::SetViewportsAndScissorRect(ID3D12GraphicsCommandList* commandList)
{
	commandList->RSSetViewports(1, &m_viewport);
	commandList->RSSetScissorRects(1, &m_scissorRect);
}
