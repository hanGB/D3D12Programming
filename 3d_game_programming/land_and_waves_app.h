#pragma once
#include "d3d_app.h"
#include "math_helper.h"
#include "land_and_waves_frame_resource.h"
#include "waves.h"

namespace land_and_waves
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
		// 파티클
		Particle* particle = nullptr;

		// 기본 도형 위상 구조
		D3D12_PRIMITIVE_TOPOLOGY primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		// DrawIndexedInstanced 매개변수
		UINT indexCount = 0;
		UINT startIndexLocation = 0;
		int baseVertexLocation = 0;
	};

	enum class RenderLayer : int
	{
		Opaque = 0,
		AlphaTest,
		TreeSprite,
		Transparent,
		Particle,
		ParticleInfinity,
		LODSphere,
		TriangleExplosion,
		DebugNormal,
		DebugSurfaceNormal,
		Count
	};
}

using namespace land_and_waves;

class LandAndWavesApp : public D3DApp {
public:
public:
	LandAndWavesApp(HINSTANCE hInstance);
	~LandAndWavesApp();

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
	void DrawRenderItemsForParticle(ID3D12GraphicsCommandList* commandList, const std::vector<RenderItem*>& renderItems);

	// 동적 버퍼 업데이트
	void UpdateWaves(const GameTimer& gt);
	// 상수 버퍼 업데이트
	void UpdateCamera(const GameTimer& gt);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMaterialCBs(const GameTimer& gt);
	void UpdateParticleCBs(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);
	// 텍스처 애니메이션
	void AnimateMaterials(const GameTimer& gt);

	// 초기화시의 빌드
	void BuildLandGeometry();
	void BuildWavesGeometry();
	void BuildTreeGeometry();
	void BuildParticle();
	void BuildTexture();
	void BuildMaterialAndSrv();
	void BuildRenderItems();
	void BuildFrameResources();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildPSO();

	float GetHillsHeight(float x, float z) const;
	XMFLOAT3 GetHillsNormal(float x, float z) const;

	void CreateParticle(std::vector<ParticleVertex>& vertices, const char* name, UINT index, float dragCoefficient, float startTime, float lifeTime);

	std::unique_ptr<RenderItem> CreateRenderItem(const XMMATRIX& world, const XMMATRIX& texTransform, UINT objectCBIndex,
		const char* geometry, const char* submesh, const char* material, D3D_PRIMITIVE_TOPOLOGY primitiveTopology, RenderLayer layer);

	std::unique_ptr<RenderItem> CreateRenderItemForParticle(const XMMATRIX& world, const XMMATRIX& texTransform, UINT objectCBIndex,
		const char* particle, const char* material, D3D_PRIMITIVE_TOPOLOGY primitiveTopology, RenderLayer layer);

	void CreateSRVForTexture(UINT index, ID3D12Resource* textureResource, const char* materialName);

	ComPtr<ID3D12DescriptorHeap> m_srvHeap = nullptr;

	// 프레임 리소스
	std::vector<std::unique_ptr<LandAndWavesFrameResource>> m_frameResources;
	LandAndWavesFrameResource* m_currentFrameResource = nullptr;
	int m_currentFrameResourceIndex = 0;

	PassConstants m_mainPassCB;
	UINT m_passCbvOffset = 0;

	ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;

	// 파이프라인 관련
	std::unordered_map<std::string, std::vector<D3D12_INPUT_ELEMENT_DESC>> m_inputLayouts;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> m_shaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> m_psos;

	// 렌더 아이템
	// 모든 렌더 아이템의 목록
	std::vector<std::unique_ptr<RenderItem>> m_allRenderItems;
	// PSO별 렌더 아이템
	std::unordered_map<RenderLayer, std::vector<RenderItem*>> m_renderItemsEachRenderLayers;

	// 뷰, 투영 변환
	XMFLOAT4X4 m_viewTransform = MathHelper::Identity4x4();
	XMFLOAT4X4 m_projectionTransform = MathHelper::Identity4x4();
	XMFLOAT3 m_eyePosition;

	// 지오메트리
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> m_geometries;
	// 파도
	std::unique_ptr<Waves> m_waves;
	RenderItem* m_wavesRenderItem;

	// 재질
	std::unordered_map<std::string, std::unique_ptr<Material>> m_materials;
	// 텍스처
	std::unordered_map<std::string, std::unique_ptr<Texture>> m_textures;

	// 파티클
	std::unordered_map<std::string, std::unique_ptr<Particle>> m_particles;

	// 마우스 입력
	POINT m_lastMousePosition;

	// 디버그 모드
	bool m_isDebugSurfaceNormal = false;

	// 카메라
	float m_theta = 1.5f * XM_PI;
	float m_phi = 0.2f * XM_PI;
	float m_radius = 30.0f;

	// 태양
	float m_sunTheta = 1.25f * XM_PI;
	float m_sunPhi = XM_PIDIV4;
};