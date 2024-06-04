#include "stdafx.h"
#include "console_logger.h"
#include "game_framework.h"

HINSTANCE g_hInst;
HWND g_hWnd;
GameFramework g_gameFramework;

LPCTSTR lpszClass = L"Window Class";
LPCTSTR lpszWindowName = L"D3D12";

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	MSG Message;
	WNDCLASSEX WndClass;
	HACCEL hAccelTable;
	g_hInst = hInstance;

	// 윈도우 클래스 구조체 값 설정
	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	// 윈도우 클래스 등록
	RegisterClassEx(&WndClass);

	// 윈도우 생성
	g_hWnd = CreateWindow(lpszClass, lpszWindowName,
		WS_OVERLAPPEDWINDOW,
		PER_DEFAULT_WINDOW_LOCATION_X, PER_DEFAULT_WINDOW_LOCATION_Y,
		PER_DEFAULT_WINDOW_WIDTH, PER_DEFAULT_WINDOW_HEIGHT,
		NULL, (HMENU)NULL, hInstance, NULL);

	// 윈도우 출력
	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(lpszWindowName));

	// 이벤트 루프 처리
	while (true) {
		if (::PeekMessage(&Message, NULL, 0, 0, PM_REMOVE))
		{
			if (Message.message == WM_QUIT) break;
			if (!::TranslateAccelerator(Message.hwnd, hAccelTable, &Message))
			{
				::TranslateMessage(&Message);
				::DispatchMessage(&Message);
			}
		}
		else
		{
			g_gameFramework.Render();
		}
	}

	return (int)Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DWORD threadID = 0;

	// 메세지 처리하기
	switch (uMsg) {
	case WM_CREATE:
#ifdef PER_DEBUG
		PERLog::SetLogger(new ConsoleLogger());
#else
		PERLog::SetLogger(nullptr);
#endif 
		g_gameFramework.OnCreate(g_hInst, hWnd);
		break;

	case WM_SIZE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_KEYDOWN:
	case WM_KEYUP:
		g_gameFramework.OnProcessingWindowMessage(hWnd, uMsg, wParam, lParam);
		break;

	case WM_DESTROY:
		g_gameFramework.OnDestroy();

#ifdef PER_DEBUG
		PERLog::Logger().PrintAll();
		//system("pause");
#endif 
		PostQuitMessage(0);
		break;
	}
	// 위의 세 메시지 외의 나머지 메세지는 OS로
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
