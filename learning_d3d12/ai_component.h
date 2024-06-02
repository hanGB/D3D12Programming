#pragma once
#include "per_component.h"

class AiComponent : public PERComponent {
public:
	AiComponent();
	virtual ~AiComponent();
	virtual void Initialize();

	virtual void Update(float dTime);
};