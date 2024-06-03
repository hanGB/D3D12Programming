#pragma once
#include "input_component.h"
#include "ai_component.h"
#include "physics_component.h"
#include "graphics_component.h"

class PERObject {
public:
	PERObject(InputComponent* input, AiComponent* ai, PhysicsComponent* physics, GraphicsComponent* graphics);
	virtual ~PERObject();

	virtual void Initialize();

	InputComponent& GetInput();
	AiComponent& GetAi();
	PhysicsComponent& GetPhysics();
	GraphicsComponent& GetGraphics();
	// 컨포넌트 타입으로 얻기(같은 카테고리의 컨포넌트를 넘김(위 함수들로(GetGraphics 등)), nullptr일 경우 일반 컨포넌트에서 찾음)
	template<class T>
	T* GetComponentWithType(PERComponent* sameCategoryComponent = nullptr);

	void AddComponent(PERComponent* component);

	// 기본 정보 getter, setter
	XMFLOAT3 GetPosition() const;
	XMFLOAT3 GetScale() const;
	XMFLOAT3 GetRotation() const;
	XMFLOAT3 GetLocalAsixForce();
	XMFLOAT3 GetWorldAsixForce();
	XMFLOAT3 GetRotateForce();

	virtual void SetPosition(XMFLOAT3 position);
	virtual void SetScale(XMFLOAT3 scale);
	virtual void SetRotation(XMFLOAT3 rotation);
	void SetLocalAsixForce(XMFLOAT3 force);
	void SetWorldAsixForce(XMFLOAT3 force);
	void SetRotateForce(XMFLOAT3 force);

	// 로컬 축 정보
	virtual XMFLOAT3 GetLookVector();
	virtual XMFLOAT3 GetUpVector();
	virtual XMFLOAT3 GetRightVector();
	virtual XMFLOAT4X4 GetWorldTransform();

protected:
	// 크기 변환이 제외된 월드 변환(실제 렌더링 할 때 크기 변환을 곱해서 넘김)
	XMFLOAT4X4 m_worldTransform = Matrix4x4::Identity();

	XMFLOAT3 m_position = { 0.f, 0.f, 0.f };
	XMFLOAT3 m_scale = { 1.f, 1.f, 1.f };
	XMFLOAT3 m_rotation = { 0.f, 0.f, 0.f };

private:
	template <class T>
	T* GetComponent(PERComponent* component);

	// 일반 컨포넌트
	PERComponent* m_component = nullptr;
	// 업데이트가 있는 특수한 종류의 컨포넌트
	InputComponent* m_input = nullptr;
	AiComponent* m_ai = nullptr;
	PhysicsComponent* m_physics = nullptr;
	GraphicsComponent* m_graphics = nullptr;

	// 이동이나 회전을 위한 힘(한 번 사용하면 초기화 됨)
	XMFLOAT3 m_localAsixForce = { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 m_worldAsixForce = { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 m_rotateForce = { 0.0f, 0.0f, 0.0f };
};


template<class T>
inline T* PERObject::GetComponentWithType(PERComponent* sameCategoryComponent)
{
	if (sameCategoryComponent == NULL) return GetComponent<T>(m_component);
	else return GetComponent<T>(sameCategoryComponent);
}

template<class T>
inline T* PERObject::GetComponent(PERComponent* component)
{
	T* findComponent;
	while (component)
	{
		// 컨포넌트를 해당 타입으로 다이나믹 캐스트 가능하면 리턴
		findComponent = dynamic_cast<T*>(m_component);
		if (findComponent) return findComponent;
		// 다음 컨포넌트 확인
		component = component->GetNext();
	}
	return nullptr;
}
