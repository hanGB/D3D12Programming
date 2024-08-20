#pragma once
#include "d3d_app.h"
#include "shapes_frame_resource.h"

class ShapesApp : public D3DApp {
public:
	ShapesApp(HINSTANCE hInstance);
	~ShapesApp();

	virtual bool Initialize() override;

private:
	virtual void OnResize() override;
	virtual void Update(const GameTimer& gt) override;
	virtual void Draw(const GameTimer& gt) override;

	void BuildFrameResources();

	static const int c_NUM_FRAME_RESOURCES = 3;

	std::vector<std::unique_ptr<ShapesFrameResource>> m_frameResource;
	ShapesFrameResource* m_currentFrameResource = nullptr;
	int m_currentFrameResourceIndex = 0;
};