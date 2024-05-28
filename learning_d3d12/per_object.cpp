#include "stdafx.h"
#include "per_object.h"

PERObject::PERObject(GraphicsComponent* graphics)
{
	m_graphics = graphics;
}

PERObject::~PERObject()
{
	delete m_graphics;
}

GraphicsComponent& PERObject::GetGraphics()
{
	return *m_graphics;
}
