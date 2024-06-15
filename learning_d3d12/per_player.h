#pragma once
#include "per_object.h"

class PERPlayer : public PERObject {
public:
	PERPlayer(ObjectFactory& factory, InputComponent* input, AiComponent* ai, PhysicsComponent* physics, GraphicsComponent* graphics);
	~PERPlayer();

	void Build(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* rootSignature);

	virtual void Initialize();

	virtual void SetPosition(XMFLOAT3 position);
	virtual void SetScale(XMFLOAT3 scale);
	virtual void SetRotation(XMFLOAT3 rotation);

	// 로컬 축 정보
	virtual XMFLOAT3 GetLookVector();
	virtual XMFLOAT3 GetUpVector();
	virtual XMFLOAT3 GetRightVector();

	virtual XMFLOAT4X4 GetWorldTransform();

	void SetLookVector(XMFLOAT3 look);
	void SetUpVector(XMFLOAT3 up);
	void SetRightVector(XMFLOAT3 right);

private:
	XMFLOAT3 m_look = XMFLOAT3(0.f, 0.f, 1.f);
	XMFLOAT3 m_up = XMFLOAT3(0.f, 1.f, 0.f);
	XMFLOAT3 m_right = XMFLOAT3(1.f, 0.f, 0.f);
};

