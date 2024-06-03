#pragma once
#include "graphics_component.h"
#include "d3d12_mesh.h"
#include "d3d12_shader.h"
#include "d3d12_camera.h"

class PlayerGraphics : public GraphicsComponent {
public:
	PlayerGraphics();
	virtual ~PlayerGraphics();

	virtual void Render(ID3D12GraphicsCommandList* commandList, D3D12Camera* camera);
};