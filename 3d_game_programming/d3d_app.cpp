#include "stdafx.h"
#include "d3d_app.h"

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return D3DApp::GetApp()->MsgProc(hwnd, msg, wParam, lParam);
}

D3DApp* D3DApp::m_app = nullptr;

D3DApp::D3DApp(HINSTANCE hInstance)
    : m_hAppInst(hInstance)
{
    assert(m_app == nullptr);
    m_app = this;
}

D3DApp::~D3DApp()
{
    if (m_d3dDevice != nullptr)
    {
        FlushCommandQueue();
    }
}

D3DApp* D3DApp::GetApp()
{
    return m_app;
}

HINSTANCE D3DApp::AppInst() const
{
    return m_hAppInst;
}

HWND D3DApp::MainWnd() const
{
    return m_hMainWnd;
}

float D3DApp::AspectRatio() const
{
    return (float)m_clientWidth / (float)m_clientHeight;
}

bool D3DApp::Get4xMsaaState() const
{
    return m_4xMsaaState;
}

void D3DApp::Set4xMsaaState(bool value)
{
    if (m_4xMsaaState != value)
    {
        m_4xMsaaState = value;

        // 변경된 MSAA 상태에 맞게 스왑체인 재생성
        CreateSwapChain();
        OnResize();
    }
}

int D3DApp::Run()
{
    MSG msg = { 0 };

    // 타이머 리셋
    m_timer.Reset();

    // 윈도우 종료 메시지가 올 때까지 루프
    while (msg.message != WM_QUIT) 
    {
        // 메세지가 있으면 처리
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        // 없으면 게임 루프 실행
        else
        {
            m_timer.Tick();

            if (!m_appPause)
            {
                CalculateFrameStats();
                Update(m_timer);
                Draw(m_timer);
            }
            else
            {
                Sleep(100);
            }
        }
    }

    return (int)msg.wParam;
}

bool D3DApp::Initialize()
{
    if (!InitMainWindow()) return false;

    if (!InitDrect3D()) return false;

    OnResize();

    return true;
}

LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        // 응용 프로그램 활성화/비활성화
    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE)
        {
            m_appPause = true;
            m_timer.Stop();
        }
        else
        {
            m_appPause = false;
            m_timer.Start();
        }
        return 0;

        // 윈도우 사이즈 변경
    case WM_SIZE:
        // 새로운 해상도 저장
        m_clientWidth = LOWORD(lParam);
        m_clientHeight = HIWORD(lParam);

        if (m_d3dDevice)
        {
            // 최소화
            if (wParam == SIZE_MINIMIZED)
            {
                m_appPause = true;
                m_minimized = true;
                m_maximized = false;
            }
            // 최대화
            else if (wParam == SIZE_MAXIMIZED)
            {
                m_appPause = false;
                m_minimized = false;
                m_maximized = true;
                OnResize();
            }
            // 이전 사이즈로 복귀
            else if (wParam == SIZE_RESTORED)
            {
                if (m_minimized)
                {
                    m_appPause = false;
                    m_minimized = false;
                    OnResize();
                }
                else if (m_maximized)
                {
                    m_appPause = false;
                    m_maximized = false;
                    OnResize();
                }
                else if (m_resizing)
                {

                }
                else
                {
                    OnResize();
                }
            }
        }
        return 0;

        // 사용자가 크기 변경 테두리를 잡았을 때
    case WM_ENTERSIZEMOVE:
        m_appPause = true;
        m_resizing = true;
        m_timer.Stop();
        return 0;

        // 사용자가 크기 변경 테두리를 놓았을 때
    case WM_EXITSIZEMOVE:
        m_appPause = false;
        m_resizing = false;
        m_timer.Start();
        OnResize();
        return 0;

        // 창이 파괴되려 할 때
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

        // 메뉴가 활성화되어서 사용자가 키를 눌렀지만 그 키가 어디에도 해당되지 않을 경우
    case WM_MENUCHAR:
        // Alt-Enter를 눌렸을 때 소리가 나지 않게 설정
        return MAKELRESULT(0, MNC_CLOSE);

    // 창이 너무 작아지지 않게 하기 위해 설정
    case WM_GETMINMAXINFO:
        ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
        ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
        return 0;

    // 마우스 입력 처리
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
        OnMouseDown(wParam, LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        OnMouseUp(wParam, LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_MOUSEMOVE:
        OnMouseMove(wParam, LOWORD(lParam), HIWORD(lParam));
        return 0;

    case WM_KEYUP:
        if (wParam == VK_ESCAPE) PostQuitMessage(0);
        else if (wParam == VK_F2) Set4xMsaaState(!m_4xMsaaState);

        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void D3DApp::CreateRtvAndDsvDescriptorHeaps()
{
    // 렌더 타겟 힙 생성
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = c_SWAP_CHAIN_BUFFER_COUNT;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

    // 깊이 스탠실 힙 생성
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
}

void D3DApp::OnResize()
{
}

bool D3DApp::InitMainWindow()
{
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_hAppInst;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = L"MainWnd";

    if (!RegisterClass(&wc))
    {
        MessageBox(0, L"RegisterClass failed.", 0, 0);
        return false;
    }

    // 클라이언트 해상도에 맞추어 윈도우 사각형 계산
    RECT R = { 0, 0, m_clientWidth, m_clientHeight };
    AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
    int width = R.right - R.left;
    int height = R.bottom - R.top;

    m_hMainWnd = CreateWindow(
        L"MainWnd", 
        m_mainWndCaption.c_str(), 
        WS_OVERLAPPEDWINDOW, 
        CW_USEDEFAULT, CW_USEDEFAULT, width, height, 
        0, 
        0, 
        m_hAppInst, 
        0);

    ShowWindow(m_hMainWnd, SW_SHOW);
    UpdateWindow(m_hMainWnd);

    return true;
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
    // 커맨드 큐 생성
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    ThrowIfFailed(m_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

    // 커맨드 할당자 생성
    ThrowIfFailed(m_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandListAllocator)));

    // 커맨드 리스트 생성
    ThrowIfFailed(m_d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, 
        m_commandListAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));

    // 닫힌 상태로 시작(Rest 호출 시 커맨드 리스트가 닫혀 있어야 함)
    m_commandList->Close();
}

void D3DApp::CreateSwapChain()
{
    // 새 스왑체인을 생성하기 전 기존 스왑체인 해제
    m_swapChain.Reset();

    // 스왑체인 서술
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    swapChainDesc.BufferDesc.Width = m_clientWidth;
    swapChainDesc.BufferDesc.Height = m_clientHeight;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferDesc.Format = m_backBufferFormat;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
    swapChainDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = c_SWAP_CHAIN_BUFFER_COUNT;
    swapChainDesc.OutputWindow = m_hMainWnd;
    swapChainDesc.Windowed = true;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    // 스왑체인 생성
    ThrowIfFailed(m_dxgiFactory->CreateSwapChain(m_commandQueue.Get(), &swapChainDesc, m_swapChain.GetAddressOf()));
}

void D3DApp::FlushCommandQueue()
{
}

ID3D12Resource* D3DApp::CurrentBackBuffer() const
{
    return m_swapChainBuffer[m_currentBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE D3DApp::CurrentBackBufferView() const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_currentBackBuffer, m_rtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3DApp::DepthStencilView() const
{
    return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
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
