#include "stdafx.h"
#include "per_player.h"
#include "player_input.h"
#include "ai_component.h"
#include "player_physics.h"
#include "player_graphics.h"
#include "camera_component.h"
#include "d3d12_mesh.h"
#include "graphics_components_shader.h"

PERPlayer::PERPlayer()
	: PERObject(new PlayerInput(), new AiComponent(), new PlayerPhysics(), new PlayerGraphics)
{
	AddComponent(new CameraComponent());
}

PERPlayer::~PERPlayer()
{
}

void PERPlayer::Build(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* rootSignature)
{
	d3d12_mesh::AirplaneMeshDiffused* mesh = new d3d12_mesh::AirplaneMeshDiffused(device, commandList, 20.f, 20.f, 4.f);
	GraphicsComponentsShader* shader = new GraphicsComponentsShader(L"./shader/vertex_shader.cso", L"./shader/pixel_shader.cso");
	shader->CreatePipelineState(device, rootSignature);

	GetComponentWithType<CameraComponent>()->ChangeCamera(SPACE_SHIP_CAMERA, 0.0f);

	GetGraphics().CreateShaderVariables(device, commandList);
	GetGraphics().SetMesh(mesh);
	GetGraphics().SetShader(shader);

	SetPosition(XMFLOAT3(0.f, 0.f, -50.0f));
}

void PERPlayer::Initialize()
{
}

void PERPlayer::SetPosition(XMFLOAT3 position)
{
	m_position = position;
}

void PERPlayer::SetScale(XMFLOAT3 scale)
{
	m_scale = scale;
}

void PERPlayer::SetRotation(XMFLOAT3 rotation)
{
	m_rotation = rotation;
}

XMFLOAT3 PERPlayer::GetLookVector()
{
	return m_look;
}

XMFLOAT3 PERPlayer::GetUpVector()
{
	return m_up;
}

XMFLOAT3 PERPlayer::GetRightVector()
{
	return m_right;
}

XMFLOAT4X4 PERPlayer::GetWorldTransform()
{
	m_worldTransform._11 = m_right.x;
	m_worldTransform._12 = m_right.y;
	m_worldTransform._13 = m_right.z;
	m_worldTransform._21 = m_up.x;
	m_worldTransform._22 = m_up.y;
	m_worldTransform._23 = m_up.z;
	m_worldTransform._31 = m_look.x;
	m_worldTransform._32 = m_look.y;
	m_worldTransform._33 = m_look.z;
	m_worldTransform._41 = m_position.x;
	m_worldTransform._42 = m_position.y;
	m_worldTransform._43 = m_position.z;

	// 최종적으로 크기 변환을 추가해야 함
	return PERObject::GetWorldTransform();
}

void PERPlayer::SetLookVector(XMFLOAT3 look)
{
	m_look = look;
}

void PERPlayer::SetUpVector(XMFLOAT3 up)
{
	m_up = up;
}

void PERPlayer::SetRightVector(XMFLOAT3 right)
{
	m_right = right;
}
