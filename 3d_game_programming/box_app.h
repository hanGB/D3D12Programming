#pragma once

#include "d3d_app.h"
#include "math_helper.h"
#include "upload_buffer.h"

//struct Vertex
//{
//	XMFLOAT3 pos;
//	XMFLOAT4 color;
//};

struct VPosData
{
	XMFLOAT3 pos;
};
struct VColorData
{
	XMFLOAT4 color;
};


struct ObjectConstants
{
	XMFLOAT4X4 worldViewProjection = MathHelper::Identity4x4();
};

class BoxApp : public D3DApp
{
public:
	BoxApp(HINSTANCE hInstance);
	BoxApp(const BoxApp& rhs) = delete;
	BoxApp& operator=(const BoxApp& rhs) = delete;
	~BoxApp();

	virtual bool Initialize() override;

private:
	virtual void OnResize() override;
	virtual void Update(const GameTimer& gt) override;
	virtual void Draw(const GameTimer& gt) override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y) override;

	void BuildDescriptorHeaps();
	void BuildConstantBuffers();
	void BuildRootSignature();
	void BuildshadersAndInputLayout();
	void BuildBoxGeometry();
	void BuildPyramidGeometry();
	void BuildPSO();

	ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;
	ComPtr<ID3D12DescriptorHeap> m_cbvHeap = nullptr;

	std::unique_ptr<UploadBuffer<ObjectConstants>> m_objectCB = nullptr;

	std::unique_ptr<MeshGeometry> m_boxGeometry = nullptr;
	std::unique_ptr<MeshGeometry> m_pyramidGeometry = nullptr;

	ComPtr<ID3DBlob> m_vsByteCode = nullptr;
	ComPtr<ID3DBlob> m_psByteCode = nullptr;

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;

	ComPtr<ID3D12PipelineState> m_pso = nullptr;

	XMFLOAT4X4 m_worldTransform = MathHelper::Identity4x4();
	XMFLOAT4X4 m_viewMatrix = MathHelper::Identity4x4();
	XMFLOAT4X4 m_projectionMatrix = MathHelper::Identity4x4();

	float m_theta = 1.5f * XM_PI;
	float m_phi = XM_PIDIV4;
	float m_radius = 5.0f;

	POINT m_lastMousePosition;
};