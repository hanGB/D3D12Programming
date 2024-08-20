#pragma once
#include "d3d_app.h"
#include "math_helper.h"
#include "shapes_frame_resource.h"

static const int NUM_FRAME_RESOURCES = 3;

struct RenderItem 
{
	RenderItem() = default;

	// 월드 변환 행렬
	XMFLOAT4X4 world = MathHelper::Identity4x4();

	// 물체의 자료가 변해서 상수 버퍼를 갱신해야 하는 지 여부
	int numFamesDirty = NUM_FRAME_RESOURCES;

	// 이 렌더 항목의 물체 상수 버퍼에 해당하는 GPU 상수 버퍼 인덱스
	UINT objectCBIndex = -1;

	// 기하 구조
	MeshGeometry* geometry = nullptr;

	// 기본 도형 위상 구조
	D3D12_PRIMITIVE_TOPOLOGY primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// DrawIndexedInstanced 매개변수
	UINT indexCount = 0;
	UINT startIndexLocation = 0;
	int baseVertexLocation = 0;

};

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

	void BuildFrameResources();

	// 프레임 리소스
	std::vector<std::unique_ptr<ShapesFrameResource>> m_frameResource;
	ShapesFrameResource* m_currentFrameResource = nullptr;
	int m_currentFrameResourceIndex = 0;

	// 렌더 아이템
	// 모든 렌더 아이템의 목록
	std::vector<std::unique_ptr<RenderItem>> m_allRenderItems;
	// PSO별 렌더 아이템
	std::vector<RenderItem*> m_opqaueRederItems;
	std::vector<RenderItem*> m_transparentRenderItems;
 };