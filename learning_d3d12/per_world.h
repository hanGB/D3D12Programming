#pragma once
#include "d3d12_shader.h"
#include "d3d12_mesh.h"

class PERObject;
class D3D12Camera;

class PERWorld {
public:
	PERWorld();
	~PERWorld();

	void BuildObjects(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
	void ReleaseObjects();

	void SetCameraInformation(D3D12Camera* camera, int width, int height);

	void InputUpdate(float deltaTime) {};
	void AiUpdate(float deltaTime) {};
	void PhysicsUpdate(float deltaTime); 
	void GraphicsUpdate(float deltaTime);

	void Render(ID3D12GraphicsCommandList* commandList, D3D12Camera* camera);

	void ReleaseUploadBuffers();

private:
	// 파이프라인
	ID3D12RootSignature* m_rootSignature;
	d3d12_shader::Shader* m_shader;

	// 메쉬
	d3d12_mesh::Mesh* m_mesh;

	// 오브젝트
	PERObject* m_object;
};