#include "stdafx.h"
#include "rotating_ai.h"
#include "per_object.h"

RotatingAi::RotatingAi()
{
}

RotatingAi::~RotatingAi()
{
}

void RotatingAi::Initialize()
{
	AiComponent::Initialize();
}

void RotatingAi::Update(float dTime)
{
	XMFLOAT3 rotateForce = m_amount;
	GetOwner()->SetRotateForce(rotateForce);

	AiComponent::Update(dTime);
}

void RotatingAi::SetAmount(XMFLOAT3 amount)
{
	m_amount = amount;
}
