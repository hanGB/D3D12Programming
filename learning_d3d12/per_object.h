#pragma once
#include "graphics_component.h"

class PERObject {
public:
	PERObject(GraphicsComponent* m_graphics);
	~PERObject();

	GraphicsComponent& GetGraphics();

private:
	GraphicsComponent* m_graphics = nullptr;

	XMFLOAT3 m_position;
};