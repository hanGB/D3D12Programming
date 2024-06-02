#include "stdafx.h"
#include "ai_component.h"

AiComponent::AiComponent()
{
}

AiComponent::~AiComponent()
{
}

void AiComponent::Initialize()
{
	PERComponent::Initialize();
}

void AiComponent::Update(float dTime)
{
	if (GetNext()) dynamic_cast<AiComponent*>(GetNext())->Update(dTime);
}
