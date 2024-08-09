#include "stdafx.h"
#include "box_app.h"

BoxApp::BoxApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
}

BoxApp::~BoxApp()
{
}

bool BoxApp::Initialize()
{
	if (!D3DApp::Initialize())
		return false;

	return true;
}

void BoxApp::OnResize()
{
}

void BoxApp::Update(const GameTimer& gt)
{
}

void BoxApp::Draw(const GameTimer& gt)
{
}

void BoxApp::OnMouseDown(WPARAM btnState, int x, int y)
{
}

void BoxApp::OnMouseUp(WPARAM btnState, int x, int y)
{
}

void BoxApp::OnMouseMove(WPARAM btnState, int x, int y)
{
}

void BoxApp::BuildDescriptorHeap()
{
}

void BoxApp::BuildConstantHeap()
{
}

void BoxApp::BuildRootSignature()
{
}

void BoxApp::BuildshadersAndInputLayout()
{
}

void BoxApp::BuildPSO()
{
}
