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
#if defined(DEBUG) || defined(_DEBUG)
    // 디버그층 활성화
    ComPtr<ID3D12Debug> debugController;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
    debugController->EnableDebugLayer();
#endif

    // DXGI 팩토리 생성
    ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)));

    // 하드웨어 어뎁터를 나타내는 장치 생성
    HRESULT hardwareResult = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_d3dDevice));
    // 장치 생성 실패 했을 경우 WARP 어뎁터를 나타내는 장치 생성
    if (FAILED(hardwareResult))
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        ThrowIfFailed(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

        ThrowIfFailed(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_d3dDevice)));
    }

    // 펜스 생성
    ThrowIfFailed(m_d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

    // 디스크립터 크기 얻기
    m_rtvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_dsvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    m_cbvSrvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // 4x MSAA 지원 설정
    // Direct3D 11, 12급 장치는 모든 렌더 대상 형식에서 지원하지만 명식적으로 점검
    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
    msQualityLevels.Format = m_backBufferFormat;
    msQualityLevels.SampleCount = 4;
    msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    msQualityLevels.NumQualityLevels = 0;
    ThrowIfFailed(m_d3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msQualityLevels, sizeof(msQualityLevels)));

    m_4xMsaaQuality = msQualityLevels.NumQualityLevels;
    assert(m_4xMsaaQuality > 0 && "Unexpected MSAA quality level");

#ifdef _DEBUG
    // 어뎁터 로그 출력
    LogAdapters();
#endif

    // 커맨드 관련 오브젝트 생성
    CreateCommandObjects();
    // 스왑체인 생성
    CreateSwapChain();
    // 디스크립터 힙 생성
    CreateRtvAndDsvDescriptorHeaps();

    return true;
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
    UINT i = 0;
    IDXGIAdapter* adapter = nullptr;
    std::vector<IDXGIAdapter*> adapterList;
    while (m_dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC desc;
        adapter->GetDesc(&desc);

        std::wstring text = L"***Adapte: ";
        text += desc.Description;
        text += L"\n";

        OutputDebugString(text.c_str());

        adapterList.push_back(adapter);

        ++i;
    }

    for (auto adapterInList : adapterList)
    {
        LogAdapterOutputs(adapterInList);
        ReleaseCom(adapterInList)
    }
}

void D3DApp::LogAdapterOutputs(IDXGIAdapter* adapter)
{
    UINT i = 0;
    IDXGIOutput* output = nullptr;
    while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_OUTPUT_DESC desc;
        output->GetDesc(&desc);

        std::wstring text = L"***Output: ";
        text += desc.DeviceName;
        text += L"\n";

        LogOutputDisplayModes(output, DXGI_FORMAT_B8G8R8A8_UNORM);

        ReleaseCom(output);

        ++i;
    }
}

void D3DApp::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
    UINT count = 0;
    UINT flags = 0;

    // nullptr를 인수로 호출 시 목록의 크기를 얻음
    output->GetDisplayModeList(format, flags, &count, nullptr);

    std::vector<DXGI_MODE_DESC> modeList(count);
    output->GetDisplayModeList(format, flags, &count, &modeList[0]);

    for (auto& mode : modeList)
    {
        UINT n = mode.RefreshRate.Numerator;
        UINT d = mode.RefreshRate.Denominator;
        std::wstring text =
            L"Width = " + std::to_wstring(mode.Width) + L" " +
            L"Height = " + std::to_wstring(mode.Height) + L" " +
            L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d)
            + L"\n";

        ::OutputDebugString(text.c_str());
    }
}
