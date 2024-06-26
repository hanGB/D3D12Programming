#pragma once
#include "object_factory.h"
#include "per_component.h"
#include "d3d12_shader.h"
#include "d3d12_mesh.h"

class InputComponent;
class AiComponent;
class PhysicsComponent;
class GraphicsComponent;

class ObjectFactory {
public:
	ObjectFactory(int objectType, int input, int ai, int physics, int graphics);
	ObjectFactory(int objectType, std::vector<int> inputs, std::vector<int> ais,
		std::vector<int> physicses, std::vector<int> graphicses);
	virtual ~ObjectFactory();

	template<class T>
	T* CreateObject();

	void AddOtherComponent(int type);
	void SetShader(d3d12_shader::Shader* shader);
	void SetMeshType(int meshType);

	int GetObjectType() const;

private:
	InputComponent* CreateInputs();
	AiComponent* CreateAis();
	PhysicsComponent* CreatePhysicses();
	GraphicsComponent* CreateGraphicses();
	PERComponent* CreateOthers();

	template <class TComponent>
	TComponent* CreateComponents(std::vector<int>& types, std::function<TComponent* (ObjectFactory&, int type)> CreateFunc);

	// 기본 타입
	InputComponent* CreateInput(int type);
	AiComponent* CreateAi(int type);
	PhysicsComponent* CreatePhysics(int type);
	GraphicsComponent* CreateGraphics(int type);
	PERComponent* CreateOther(int type);

	// 추가 타입
	virtual InputComponent* CreateInputAdditionalType(int type) { return nullptr; }
	virtual AiComponent* CreateAiAdditionalType(int type) { return nullptr; }
	virtual PhysicsComponent* CreatePhysicsAdditionalType(int type) { return nullptr; }
	virtual GraphicsComponent* CreateGraphicsAdditionalType(int type) { return nullptr; }
	virtual PERComponent* CreateOtherAdditionalType(int type) { return nullptr; }

	ComponentVectors m_componentVectors;
	d3d12_shader::Shader* m_shader = nullptr;

	int m_meshType;
	int m_objectType;
};

template<class T>
inline T* ObjectFactory::CreateObject()
{
	T* object = new T(*this, CreateInputs(), CreateAis(), CreatePhysicses(), CreateGraphicses());
	object->AddComponent(CreateOthers());
	object->GetGraphics().SetShader(m_shader);
	object->GetGraphics().SetMeshType(m_meshType);

	return object;
}

template<class TComponent>
inline TComponent* ObjectFactory::CreateComponents(std::vector<int>& types, std::function<TComponent* (ObjectFactory&, int type)> CreateFunc)
{
	if (types.empty()) return nullptr;

	TComponent* component = CreateFunc(*this, *types.cbegin());
	if (types.size() == 1) return component;

	if (types.size() > 2)
	{
		TComponent* frontComponent = component;
		for (auto it = types.cbegin() + 1; it != types.cend(); ++it) {
			TComponent* nextComponent = CreateFunc(*this, *it);
			frontComponent->SetNext(nextComponent);
			frontComponent = nextComponent;
		}
	}
	return component;
}