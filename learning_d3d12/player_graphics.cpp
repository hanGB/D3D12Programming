#include "stdafx.h"
#include "player_graphics.h"

PlayerGraphics::PlayerGraphics()
{
}

PlayerGraphics::~PlayerGraphics()
{
}

void PlayerGraphics::Render(ResourceStorage& resourceStorage, ID3D12GraphicsCommandList* commandList, D3D12Camera* camera, UINT numInstances)
{
	if (camera->GetMode() == THIRD_PERSON_CAMERA)
	{
		GraphicsComponent::Render(resourceStorage, commandList, camera, numInstances);
	}
}
