#include "stdafx.h"
#include "d3d12_camera.h"

D3D12Camera::D3D12Camera()
{
	m_view = Matrix4x4::Identity();
	m_projection = Matrix4x4::Identity();

	m_aspectRatio = (float)PER_DEFAULT_WINDOW_WIDTH / (float)PER_DEFAULT_WINDOW_HEIGHT;
	m_viewport = { 0, 0, (float)PER_DEFAULT_WINDOW_WIDTH, (float)PER_DEFAULT_WINDOW_HEIGHT, 0.0, 1.0f };
	m_scissorRect = { 0, 0, (LONG)PER_DEFAULT_WINDOW_WIDTH, (LONG)PER_DEFAULT_WINDOW_HEIGHT };

	m_position = XMFLOAT3(0.f, 0.0f, 0.f);
	m_right = XMFLOAT3(1.f, 0.0f, 0.f);
	m_look = XMFLOAT3(0.f, 0.0f, 1.f);
	m_up = XMFLOAT3(0.f, 1.0f, 0.f);
	m_pitch = 0.0f;
	m_yaw = 0.0f;
	m_roll = 0.0f;
	m_offSet = XMFLOAT3(0.f, 0.0f, 0.f);
	m_timeLag = 0.0f;
	m_lookAt = XMFLOAT3(0.f, 0.f, 0.f);
	m_viewMode = 0x00;
	m_player = NULL;
}

D3D12Camera::D3D12Camera(D3D12Camera* camera)
{
	if (camera)
	{
		*this = camera;
	}
	else
	{
		m_view = Matrix4x4::Identity();
		m_projection = Matrix4x4::Identity();

		m_aspectRatio = (float)PER_DEFAULT_WINDOW_WIDTH / (float)PER_DEFAULT_WINDOW_HEIGHT;
		m_viewport = { 0, 0, (float)PER_DEFAULT_WINDOW_WIDTH, (float)PER_DEFAULT_WINDOW_HEIGHT, 0.0, 1.0f };
		m_scissorRect = { 0, 0, (LONG)PER_DEFAULT_WINDOW_WIDTH, (LONG)PER_DEFAULT_WINDOW_HEIGHT };

		m_position = XMFLOAT3(0.f, 0.0f, 0.f);
		m_right = XMFLOAT3(1.f, 0.0f, 0.f);
		m_look = XMFLOAT3(0.f, 0.0f, 1.f);
		m_up = XMFLOAT3(0.f, 1.0f, 0.f);
		m_pitch = 0.0f;
		m_yaw = 0.0f;
		m_roll = 0.0f;
		m_offSet = XMFLOAT3(0.f, 0.0f, 0.f);
		m_timeLag = 0.0f;
		m_lookAt = XMFLOAT3(0.f, 0.f, 0.f);
		m_viewMode = 0x00;
		m_player = NULL;
	}
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

void D3D12Camera::GenerateViewMatrix()
{
	m_view = Matrix4x4::LookAtLH(m_position, m_lookAt, m_up);
}

void D3D12Camera::GenerateViewMatrix(XMFLOAT3 position, XMFLOAT3 lookAt, XMFLOAT3 up)
{
	m_position = position;
	m_lookAt = lookAt;
	m_up = up;

	GenerateViewMatrix();
}

void D3D12Camera::RegenerateViewMatrix()
{
	// 카메라의 z축 기준으로 카메라 좌표축들이 직교하도록 카메라 변환 행렬 갱신
	m_look = Vector3::Normalize(m_look);
	m_right = Vector3::CrossProduct(m_up, m_look, true);
	m_up = Vector3::CrossProduct(m_look, m_right, true);

	m_view._11 = m_right.x; m_view._12 = m_up.x; m_view._13 = m_look.x;
	m_view._11 = m_right.y; m_view._12 = m_up.y; m_view._13 = m_look.y;
	m_view._11 = m_right.z; m_view._12 = m_up.z; m_view._13 = m_look.z;
	m_view._41 = -Vector3::DotProduct(m_position, m_right);
	m_view._42 = -Vector3::DotProduct(m_position, m_up);
	m_view._43 = -Vector3::DotProduct(m_position, m_look);
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

PERObject* D3D12Camera::GetPlayer()
{
	return m_player;
}

DWORD D3D12Camera::GetMode() const
{
	return m_viewMode;
}

XMFLOAT3& D3D12Camera::GetLookAt()
{
	return m_lookAt;
}

XMFLOAT3& D3D12Camera::GetRightVector()
{
	return m_right;
}

XMFLOAT3& D3D12Camera::GetUpVector()
{
	return m_up;
}
XMFLOAT3& D3D12Camera::GetLookVector()
{
	return m_look;
}

float& D3D12Camera::GetPitch()
{
	return m_pitch;
}

float& D3D12Camera::GetYaw()
{
	return m_yaw;
}

float& D3D12Camera::GetRoll()
{
	return m_roll;
}

XMFLOAT3& D3D12Camera::GetOffSet()
{
	return m_offSet;
}

float D3D12Camera::GetTimeLag() const
{
	return m_timeLag;
}

XMFLOAT4X4 D3D12Camera::GetViewMatrix() const
{
	return m_view;
}

XMFLOAT4X4 D3D12Camera::GetProjectionMatrix() const
{
	return m_projection;
}

D3D12_VIEWPORT D3D12Camera::GetViewport() const
{
	return m_viewport;
}

D3D12_RECT D3D12Camera::GetScissorRect() const
{
	return m_scissorRect;
}

void D3D12Camera::SetPlayer(PERObject* player)
{
	m_player = player;
}

void D3D12Camera::SetMode(DWORD mode)
{
	m_viewMode = mode;
}

void D3D12Camera::SetOffSet(XMFLOAT3 offSet)
{
	m_offSet = offSet;
}

void D3D12Camera::SetTimeLag(float timeLag)
{
	m_timeLag = timeLag;
}

void D3D12Camera::Move(XMFLOAT3& shift)
{
	m_position.x += shift.x;
	m_position.y += shift.y;
	m_position.z += shift.z;
}

void D3D12Camera::Rotate(float pitch, float yaw, float roll)
{
}

void D3D12Camera::Update(XMFLOAT3& lookAt, float timeElapsed)
{
}

void D3D12Camera::SetLookAt(XMFLOAT3& lookAt)
{
}
