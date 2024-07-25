#include "stdafx.h"
#include "d3d_app.h"

D3DApp* D3DApp::GetApp()
{
    return nullptr;
}

HINSTANCE D3DApp::AppInst() const
{
    return HINSTANCE();
}

HWND D3DApp::MainWnd() const
{
    return HWND();
}

float D3DApp::AspectRatio() const
{
    return 0.0f;
}

bool D3DApp::Get4xMsaaState() const
{
    return false;
}

void D3DApp::Set4xMsaaState(bool value)
{
}

int D3DApp::Run()
{
    return 0;
}

bool D3DApp::Initialize()
{
    return false;
}

LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return LRESULT();
}

D3DApp::D3DApp(HINSTANCE hInstance)
{
}

D3DApp::~D3DApp()
{
}

void D3DApp::CreateRtvAndDsvDescriptorHeaps()
{
}

void D3DApp::OnResize()
{
}

bool D3DApp::InitMainWindow()
{
    return false;
}

bool D3DApp::InitDrect3D()
{
    return false;
}

void D3DApp::CreateCommandObjects()
{
}

void D3DApp::CreateSwapChain()
{
}

void D3DApp::FlushCommandQueue()
{
}

ID3D12Resource* D3DApp::CurrentBackBuffer() const
{
    return nullptr;
}

D3D12_CPU_DESCRIPTOR_HANDLE D3DApp::CurrentBackBufferView() const
{
    return D3D12_CPU_DESCRIPTOR_HANDLE();
}

D3D12_CPU_DESCRIPTOR_HANDLE D3DApp::DepthStencilView() const
{
    return D3D12_CPU_DESCRIPTOR_HANDLE();
}

void D3DApp::CalculateFrameStats()
{
}

void D3DApp::LogAdapters()
{
}

void D3DApp::LogAdapterOutputs(IDXGIAdapter* adapter)
{
}

void D3DApp::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
}
