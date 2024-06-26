#pragma once
#include "d3d12_shader.h"
#include "d3d12_mesh.h"

class PERObject;
class PERPlayer;
class D3D12Camera;
class PERController;
class ObjectFactory;
class ObjectStorage;
class ResourceStorage;

class PERWorld {
public:
	PERWorld(ObjectStorage& objectStorage, ResourceStorage& resourceStorage);
	~PERWorld();

	void BuildObjects(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
	void ReleaseObjects();

	void SetCameraInformation(D3D12Camera* camera, int width, int height);

	void InputUpdate(PERController& controller, float deltaTime);
	void AiUpdate(float deltaTime);
	void PhysicsUpdate(float deltaTime); 
	void GraphicsUpdate(float deltaTime);

	void Render(ID3D12GraphicsCommandList* commandList, ID3D12DescriptorHeap* dsvDescriptorHeap, D3D12Camera* camera);

	void ReleaseUploadBuffers();

	PERPlayer* GetPlayer();

protected:
	void BuildResources(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

	static const int c_MAXIMUM_SHADER = 512;
	static const int c_INITIAL_MAXIMUM_OBJECTS = 1024;
	
	std::vector<d3d12_shader::Shader*> m_shaders;
	int m_numShaders = 0;

	std::vector<PERObject*> m_objects;
	int m_numObjects = 0;
	int m_maxObjects = c_INITIAL_MAXIMUM_OBJECTS;

	// 스토리지(프레임워크에서 생성됨)
	ObjectStorage& m_objectStorage;
	ResourceStorage& m_resourceStorage;

	// 임시로 월드에 저장
	ObjectFactory* m_factory;
	ObjectFactory* m_playerFactory;

private:
	// 파이프라인
	ID3D12RootSignature* m_rootSignature;
	PERPlayer* m_player;
};