#include "stdafx.h"
#include "per_world.h"
#include "d3d12_root.h"
#include "per_object.h"
#include "graphics_component.h"
#include "physics_component.h"
#include "d3d12_camera.h"
#include "graphics_components_shader.h"
#include "input_component.h"
#include "ai_component.h"
#include "player_input.h"
#include "rotating_ai.h"

PERWorld::PERWorld()
{
	m_rootSignature = NULL;
	m_shaders.reserve(c_MAXIMUM_SHADER);
	m_shaders.resize(c_MAXIMUM_SHADER);
	
	m_objects.reserve(c_INITIAL_MAXIMUM_OBJECTS);
	m_objects.resize(c_INITIAL_MAXIMUM_OBJECTS);

	m_mesh = NULL;
}

PERWorld::~PERWorld()
{
	m_shaders.clear();
	m_shaders.shrink_to_fit();

	m_objects.clear();
	m_objects.shrink_to_fit();
}

void PERWorld::BuildObjects(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
	// 루트 시그니처 생성
	m_rootSignature = d3d12_init::CreateRootSignature(device);

	// 쉐이더 생성
	m_numShaders = 0;
	m_shaders[m_numShaders] = new GraphicsComponentsShader(L"./shader/vertex_shader.cso", L"./shader/pixel_shader.cso");
	m_shaders[m_numShaders]->CreatePipelineState(device, m_rootSignature);
	m_numShaders++;

	m_mesh = new d3d12_mesh::CubeMeshDiffused(device, commandList, 12.0f, 12.0f, 12.0f);

	m_numObjects = 0;

	int xObjects = 10;
	int yObjects = 10;
	int zObjects = 10;
	
	float xPitch = 12.f * 2.5f;
	float yPitch = 12.f * 2.5f;
	float zPitch = 12.f * 2.5f;

	for (int x = -xObjects; x <= xObjects; ++x) {
		for (int y = -xObjects; y <= xObjects; ++y) {
			for (int z = -xObjects; z <= xObjects; ++z) {
				if (m_numObjects >= m_maxObjects) {
					m_maxObjects *= 2;
					m_objects.reserve(m_maxObjects);
					m_objects.resize(m_maxObjects);
				}

				PERObject* object = CreateObject();
				object->SetPosition(XMFLOAT3(xPitch * x, yPitch * y, zPitch * z));
				m_objects[m_numObjects++] = object;
			}
		}
	}
}

void PERWorld::ReleaseObjects()
{
	m_rootSignature->Release();
	m_mesh->ReleaseUploadBuffers();

	for (int i = 0; i < m_numShaders; ++i)
	{
		m_shaders[i]->Release();

	}
	m_numShaders = 0;

	for (int i = 0; i < m_numObjects; ++i)
	{
		delete m_objects[i];

	}
	m_numShaders = 0;
}

void PERWorld::SetCameraInformation(D3D12Camera* camera, int width, int height)
{
	camera->GenerateProjectionMatrix(90.0f, (float)width / (float)height, 0.1f, 500.0f);
	camera->GenerateViewMatrix(XMFLOAT3(0.0f, 0.0f, -50.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
}

void PERWorld::InputUpdate(PERController& controller, float deltaTime)
{
	for (int i = 0; i < m_numObjects; ++i) {
		m_objects[i]->GetInput().Update(controller, deltaTime);
	}
}

void PERWorld::AiUpdate(float deltaTime)
{
	for (int i = 0; i < m_numObjects; ++i) {
		m_objects[i]->GetAi().Update(deltaTime);
	}
}

void PERWorld::PhysicsUpdate(float deltaTime)
{
	for (int i = 0; i < m_numObjects; ++i) {
		m_objects[i]->GetPhysics().Update(deltaTime);
	}
}
void PERWorld::GraphicsUpdate(float deltaTime)
{
	for (int i = 0; i < m_numObjects; ++i) {
		m_objects[i]->GetGraphics().Update(deltaTime);
	}
}

void PERWorld::Render(ID3D12GraphicsCommandList* commandList, D3D12Camera* camera)
{
	camera->SetViewportsAndScissorRect(commandList);
	commandList->SetGraphicsRootSignature(m_rootSignature);
	camera->UpdateShaderVariables(commandList);

	for (int i = 0; i < m_numShaders; ++i) {
		m_shaders[i]->Render(commandList, camera);
	}
}

void PERWorld::ReleaseUploadBuffers()
{
	for (int i = 0; i < m_numShaders; ++i) {
		m_shaders[i]->ReleaseShaderVariables();
		m_shaders[i]->ReleaseUploadBuffers();
	}
}

PERObject* PERWorld::CreateObject()
{
	InputComponent* input = new InputComponent();
	RotatingAi* ai = new RotatingAi();
	ai->SetAmount(XMFLOAT3(0.f, 90.f, 0.f));
	PhysicsComponent* physics = new PhysicsComponent();
	GraphicsComponent* graphics = new GraphicsComponent();
	graphics->SetMesh(m_mesh);

	PERObject* object = new PERObject(input, ai, physics, graphics);
	dynamic_cast<GraphicsComponentsShader*>(m_shaders[0])->AddGraphicsComponent(graphics);

	return object;
}
