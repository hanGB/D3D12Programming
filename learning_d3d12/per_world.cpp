#include "stdafx.h"
#include "per_world.h"
#include "object_factory.h"
#include "d3d12_root.h"
#include "per_object.h"
#include "d3d12_camera.h"
#include "instancing_shader.h"
#include "per_player.h"
#include "object_type.h"
#include "resource_type.h"
#include "object_storage.h"
#include "resource_storage.h"

PERWorld::PERWorld(ObjectStorage& objectStorage, ResourceStorage& resourceStorage)
	: m_objectStorage(objectStorage), m_resourceStorage(resourceStorage)
{
	m_rootSignature = NULL;
	m_shaders.reserve(c_MAXIMUM_SHADER);
	m_shaders.resize(c_MAXIMUM_SHADER);

	m_objects.reserve(c_INITIAL_MAXIMUM_OBJECTS);
	m_objects.resize(c_INITIAL_MAXIMUM_OBJECTS);
}

PERWorld::~PERWorld()
{
	delete m_player;
	delete m_factory;
	delete m_playerFactory;
	
	m_shaders.clear();
	m_shaders.shrink_to_fit();

	m_objects.clear();
	m_objects.shrink_to_fit();
}

void PERWorld::BuildObjects(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
	// 루트 시그니처 생성
	m_rootSignature = d3d12_init::CreateRootSignature(device);

	// 리소스 생성
	BuildResources(device, commandList);

	// 플레이어 설정
	m_playerFactory = new ObjectFactory(PER_PLAYER, PER_PLAYER_INPUT, PER_BASE_COMPONENT, PER_PLAYER_PHYSICS, PER_PLAYER_GRAPHICS);
	m_playerFactory->AddOtherComponent(PER_CAMERA_COMPONENT);
	m_playerFactory->SetResourceType(PER_AIRPLANE);
	m_player = m_playerFactory->CreateObject<PERPlayer>();
	m_player->GetGraphics().SetMesh(m_resourceStorage.GetMesh(PER_MESH_AIRPLANE));
	m_player->Build(device, commandList, m_rootSignature);

	// 쉐이더 생성
	m_numShaders = 0;
	m_shaders[m_numShaders] = new InstancingShader(L"./shader/instancing_vertex.cso", L"./shader/instancing_pixel.cso");
	m_shaders[m_numShaders]->CreatePipelineState(device, m_rootSignature);
	m_numShaders++;

	m_numObjects = 0;

	int xObjects = 8;
	int yObjects = 8;
	int zObjects = 8;
	
	float xPitch = 12.f * 2.5f;
	float yPitch = 12.f * 2.5f;
	float zPitch = 12.f * 2.5f;

	d3d12_mesh::Mesh* cubeMesh = m_resourceStorage.GetMesh(PER_MESH_CUBE);
	for (int x = -xObjects; x <= xObjects; ++x) {
		for (int y = -xObjects; y <= xObjects; ++y) {
			for (int z = -xObjects; z <= xObjects; ++z) {
				if (m_numObjects >= m_maxObjects) {
					m_maxObjects *= 2;
					m_objects.reserve(m_maxObjects);
					m_objects.resize(m_maxObjects);
				}

				PERObject* object = m_objectStorage.PopObject(PER_FIXED);
				object->GetGraphics().SetMesh(cubeMesh);
				dynamic_cast<GraphicsComponentsShader*>(m_shaders[0])->AddGraphicsComponent(&object->GetGraphics());
				object->SetPosition(XMFLOAT3(xPitch * x, yPitch * y, zPitch * z));
				m_objects[m_numObjects++] = object;
			}
		}
	}
	m_shaders[0]->CreateShaderVariables(device, commandList);
}

void PERWorld::ReleaseObjects()
{
	m_rootSignature->Release();

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
	m_player->GetInput().Update(controller, deltaTime);
}

void PERWorld::AiUpdate(float deltaTime)
{
	for (int i = 0; i < m_numObjects; ++i) {
		m_objects[i]->GetAi().Update(deltaTime);
	}
	m_player->GetAi().Update(deltaTime);
}

void PERWorld::PhysicsUpdate(float deltaTime)
{
	for (int i = 0; i < m_numObjects; ++i) {
		m_objects[i]->GetPhysics().Update(deltaTime);
	}
	m_player->GetPhysics().Update(deltaTime);
}
void PERWorld::GraphicsUpdate(float deltaTime)
{
	for (int i = 0; i < m_numObjects; ++i) {
		m_objects[i]->GetGraphics().Update(deltaTime);
	}
	m_player->GetGraphics().Update(deltaTime);
}

void PERWorld::Render(ID3D12GraphicsCommandList* commandList, ID3D12DescriptorHeap* dsvDescriptorHeap, D3D12Camera* camera)
{
	camera->SetViewportsAndScissorRect(commandList);
	commandList->SetGraphicsRootSignature(m_rootSignature);
	camera->UpdateShaderVariables(commandList);

	for (int i = 0; i < m_numShaders; ++i) {
		m_shaders[i]->Render(commandList, camera);
	}

	commandList->ClearDepthStencilView(dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, NULL);
	if (m_player) m_player->GetGraphics().Render(commandList, camera, 1);

}

void PERWorld::ReleaseUploadBuffers()
{
	for (int i = 0; i < m_numShaders; ++i) {
		m_shaders[i]->ReleaseUploadBuffers();
	}
}

PERPlayer* PERWorld::GetPlayer()
{
	return m_player;
}

void PERWorld::BuildResources(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
	if (!m_resourceStorage.CheckIfMeshExists(PER_MESH_CUBE))
	{
		d3d12_mesh::Mesh* cube = new d3d12_mesh::CubeMeshDiffused(device, commandList, 12.0f, 12.0f, 12.0f);
		m_resourceStorage.AddMesh(PER_MESH_CUBE, cube);
	}
	if (!m_resourceStorage.CheckIfMeshExists(PER_MESH_AIRPLANE))
	{
		d3d12_mesh::Mesh* airplane = new d3d12_mesh::AirplaneMeshDiffused(device, commandList, 20.f, 20.f, 4.f);
		m_resourceStorage.AddMesh(PER_MESH_AIRPLANE, airplane);
	}
}
