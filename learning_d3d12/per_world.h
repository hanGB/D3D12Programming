#pragma once

class PERWorld {
public:
	PERWorld();
	~PERWorld();

	void BuildObjects(ID3D12Device* dDevice);
	void ReleaseObjects();

	void InputUpdate(float deltaTime) {};
	void AiUpdate(float deltaTime) {};
	void PhysicsUpdate(float deltaTime) {};
	void GraphicsUpdate(float deltaTime) {};

	void Render(ID3D12GraphicsCommandList* commandList);

private:
	// 파이프라인
	ID3D12RootSignature* m_rootSignature;
	ID3D12PipelineState* m_pipelineState;
};