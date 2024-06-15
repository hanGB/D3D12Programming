#include "stdafx.h"
#include "object_factory.h"
// input
#include "input_component.h"
#include "player_input.h"
// ai
#include "ai_component.h"
#include "rotating_ai.h"
// physics
#include "physics_component.h"
#include "player_physics.h"
// graphics
#include "graphics_component.h"
#include "player_graphics.h"
// other
#include "camera_component.h"

ObjectFactory::ObjectFactory(int input, int ai, int physics, int graphics)
{
	m_componentVectors.inputs.push_back(input);
	m_componentVectors.ais.push_back(ai);
	m_componentVectors.physicses.push_back(physics);
	m_componentVectors.graphicses.push_back(graphics);
}

ObjectFactory::ObjectFactory(std::vector<int> inputs, std::vector<int> ais, std::vector<int> physicses, std::vector<int> graphicses)
{
	m_componentVectors.inputs = inputs;
	m_componentVectors.ais = ais;
	m_componentVectors.physicses = physicses;
	m_componentVectors.graphicses = graphicses;
}

ObjectFactory::~ObjectFactory()
{
	m_componentVectors.inputs.clear();
	m_componentVectors.ais.clear();
	m_componentVectors.physicses.clear();
	m_componentVectors.graphicses.clear();
	m_componentVectors.others.clear();

	m_componentVectors.inputs.shrink_to_fit();
	m_componentVectors.ais.shrink_to_fit();
	m_componentVectors.physicses.shrink_to_fit();
	m_componentVectors.graphicses.shrink_to_fit();
	m_componentVectors.others.shrink_to_fit();
}

void ObjectFactory::AddOtherComponent(int type)
{
	m_componentVectors.others.push_back(type);
}

void ObjectFactory::SetShader(d3d12_shader::Shader* shader)
{
	m_shader = shader;
}

void ObjectFactory::SetMesh(d3d12_mesh::Mesh* mesh)
{
	m_mesh = mesh;
}

InputComponent* ObjectFactory::CreateInputs()
{
	return CreateComponents<InputComponent>(m_componentVectors.inputs, &ObjectFactory::CreateInput);
}

AiComponent* ObjectFactory::CreateAis()
{
	return CreateComponents<AiComponent>(m_componentVectors.ais, &ObjectFactory::CreateAi);
}

PhysicsComponent* ObjectFactory::CreatePhysicses()
{
	return CreateComponents<PhysicsComponent>(m_componentVectors.physicses, &ObjectFactory::CreatePhysics);
}

GraphicsComponent* ObjectFactory::CreateGraphicses()
{
	return CreateComponents<GraphicsComponent>(m_componentVectors.graphicses, &ObjectFactory::CreateGraphics);
}

PERComponent* ObjectFactory::CreateOthers()
{
	return CreateComponents<PERComponent>(m_componentVectors.others, &ObjectFactory::CreateOther);
}

InputComponent* ObjectFactory::CreateInput(int type)
{
	switch (type)
	{
	case PER_BASE_COMPONENT: return new InputComponent();
	case PER_PLAYER_INPUT: return new PlayerInput();

	default: return CreateInputAdditionalType(type);
	}
}

AiComponent* ObjectFactory::CreateAi(int type)
{
	switch (type)
	{
	case PER_BASE_COMPONENT: return new AiComponent();
	case PER_ROTATING_AI: return new RotatingAi();

	default: return CreateAiAdditionalType(type);
	}
}

PhysicsComponent* ObjectFactory::CreatePhysics(int type)
{
	switch (type)
	{
	case PER_BASE_COMPONENT: return new PhysicsComponent();
	case PER_PLAYER_PHYSICS: return new PlayerPhysics();

	default: return CreatePhysicsAdditionalType(type);
	}
}

GraphicsComponent* ObjectFactory::CreateGraphics(int type)
{
	switch (type)
	{
	case PER_BASE_COMPONENT: return new GraphicsComponent();
	case PER_PLAYER_GRAPHICS: return new PlayerGraphics();

	default: return CreateGraphicsAdditionalType(type);
	}
}

PERComponent* ObjectFactory::CreateOther(int type)
{
	switch (type)
	{
	case PER_CAMERA_COMPONENT: return new CameraComponent();

	default: return CreateOtherAdditionalType(type);
	}
}
