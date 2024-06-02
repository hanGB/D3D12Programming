#pragma once
#include "d3d12_shader.h"
#include "d3d12_mesh.h"

class PERObject;
class D3D12Camera;
class PERController;

class PERWorld {
public:
	PERWorld();
	~PERWorld();

	void BuildObjects(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
	void ReleaseObjects();

	void SetCameraInformation(D3D12Camera* camera, int width, int height);

	void InputUpdate(PERController& controller, float deltaTime);
	void AiUpdate(float deltaTime);
	void PhysicsUpdate(float deltaTime); 
	void GraphicsUpdate(float deltaTime);

	void Render(ID3D12GraphicsCommandList* commandList, D3D12Camera* camera);

	void ReleaseUploadBuffers();

protected:
	// 임시(오브젝트 펙토리 클래스를 만들 것)
	PERObject* CreateObject();

	static const int c_MAXIMUM_SHADER = 512;
	static const int c_INITIAL_MAXIMUM_OBJECTS = 1024;

	std::vector<d3d12_shader::Shader*> m_shaders;
	int m_numShaders = 0;

	std::vector<PERObject*> m_objects;
	int m_numObjects = 0;
	int m_maxObjects = c_INITIAL_MAXIMUM_OBJECTS;

private:
	// 파이프라인
	ID3D12RootSignature* m_rootSignature;

	// 메쉬
	d3d12_mesh::Mesh* m_mesh;
};