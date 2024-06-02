#pragma once
#include "ai_component.h"

class RotatingAi : public AiComponent {
public:
	RotatingAi();
	virtual ~RotatingAi();

	virtual void Initialize();
	virtual void Update(float dTime);

	void SetAmount(XMFLOAT3 amount);

private:
	XMFLOAT3 m_amount = { 0.f, 30.f, 0.f };
};