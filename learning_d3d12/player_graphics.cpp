#include "stdafx.h"
#include "player_graphics.h"

PlayerGraphics::PlayerGraphics()
{
}

PlayerGraphics::~PlayerGraphics()
{
}

void PlayerGraphics::Render(ID3D12GraphicsCommandList* commandList, D3D12Camera* camera)
{
	if (camera->GetMode() == THIRD_PERSON_CAMERA)
	{
		GraphicsComponent::Render(commandList, camera, 1);
	}
}
