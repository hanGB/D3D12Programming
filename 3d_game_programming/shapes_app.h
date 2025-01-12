#pragma once
#include "d3d_app.h"
#include "math_helper.h"
#include "shapes_frame_resource.h"

namespace shapes 
{
	struct RenderItem
	{
		RenderItem() = default;

		// 월드 변환 행렬
		XMFLOAT4X4 world = MathHelper::Identity4x4();
		XMFLOAT4X4 texTransform = MathHelper::Identity4x4();

		// 물체의 자료가 변해서 상수 버퍼를 갱신해야 하는 지 여부
		int numFramesDirty = NUM_FRAME_RESOURCES;

		// 이 렌더 항목의 물체 상수 버퍼에 해당하는 GPU 상수 버퍼 인덱스
		UINT objectCBIndex = -1;

		// 기하 구조
		MeshGeometry* geometry = nullptr;
		// 재질
		Material* material = nullptr;

		// 기본 도형 위상 구조
		D3D12_PRIMITIVE_TOPOLOGY primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		// DrawIndexedInstanced 매개변수
		UINT indexCount = 0;
		UINT startIndexLocation = 0;
		int baseVertexLocation = 0;

	};
}

using namespace shapes;

class ShapesApp : public D3DApp 
{
public:
	ShapesApp(HINSTANCE hInstance);
	~ShapesApp();

	virtual bool Initialize() override;

private:
	virtual void OnResize() override;
	virtual void Update(const GameTimer& gt) override;
	virtual void Draw(const GameTimer& gt) override;
	
	// 마우스, 키보드 사용
	virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y) override;
	virtual void OnKeyboradInput(WPARAM btnState, bool isPressed) override;

	// 렌더 아이템 그리기
	void DrawRenderItems(ID3D12GraphicsCommandList* commandList, const std::vector<RenderItem*>& renderItems);

	// 상수 버퍼 업데이트
	void UpdateCamera(const GameTimer& gt);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMaterialCBs(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);

	// 초기화시의 빌드
	void BuildShapeGeometry();
	void BuildSkullGeometry();
	void BuildMaterial();
	void BuildTexture();
	void BuildRenderItems();
	void BuildFrameResources();
	void BuildDescriptorHeaps();
	void BuildObjectConstantBufferViews();
	void BuildPassConstantBufferViews();
	void BuildTextureShaderResourceViews();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildPSO();

	std::unique_ptr<RenderItem> CreateRenderItem(const XMMATRIX& world, const XMMATRIX& texTransform, UINT objectCBIndex,
		const char* geometry, const char* submesh, const char* material, D3D_PRIMITIVE_TOPOLOGY primitiveTopology);

	ComPtr<ID3D12DescriptorHeap> m_cbvSrvHeap = nullptr;

	// 프레임 리소스
	std::vector<std::unique_ptr<ShapesFrameResource>> m_frameResources;
	ShapesFrameResource* m_currentFrameResource = nullptr;
	int m_currentFrameResourceIndex = 0;

	PassConstants m_mainPassCB;
	UINT m_passCbvOffset = 0;
	UINT m_textureSrvOffset = 0;

	ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;

	// 파이프라인 관련
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> m_shaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> m_psos;

	// 렌더 아이템
	// 모든 렌더 아이템의 목록
	std::vector<std::unique_ptr<RenderItem>> m_allRenderItems;
	// PSO별 렌더 아이템
	std::vector<RenderItem*> m_opqaueRederItems;
	std::vector<RenderItem*> m_transparentRenderItems;

	// 뷰, 투영 변환
	XMFLOAT4X4 m_viewTransform = MathHelper::Identity4x4();
	XMFLOAT4X4 m_projectionTransform = MathHelper::Identity4x4();
	XMFLOAT3 m_eyePosition;

	// 지오메트리(기하)
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> m_geometries;
	// 머터리얼(재질)
	std::unordered_map<std::string, std::unique_ptr<Material>> m_materials;
	// 텍스처
	std::unordered_map<std::string, std::unique_ptr<Texture>> m_textures;

	// 와이어 프레임 여부
	bool m_isWireFrame = false;
	bool m_isSpotLight = false;

	// 마우스 입력
	POINT m_lastMousePosition;

	// 카메라
	float m_theta = 1.5f * XM_PI;
	float m_phi = 0.2f * XM_PI;
	float m_radius = 15.0f;
 };