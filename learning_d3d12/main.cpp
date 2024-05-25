#include "stdafx.h"
#include "console_logger.h"
#include "window_proc_with_thread.h"


// window_proc_with_thread.h 안 전역변수
extern HINSTANCE g_hInst;
extern HWND g_hWnd;
extern HANDLE g_hWorkerThreads[PER_NUM_WORKER_THREAD];
extern bool g_isGameEnd;

LPCTSTR lpszClass = L"Window Class";
LPCTSTR lpszWindowName = L"D3D12";

#define PER_DEBUG

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	MSG Message;
	WNDCLASSEX WndClass;
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

	// 이벤트 루프 처리
	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
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

		g_isGameEnd = false;
		// 스레드 생성
		CreateWorkerThreads(threadID);
		break;

	case WM_KEYDOWN:
		break;

	case WM_KEYUP:
		break;

	case WM_DESTROY:
		g_isGameEnd = true;
		// 게임 루프 스레드가 종료될 때 까지 무한 대기
		WaitForMultipleObjects(PER_NUM_WORKER_THREAD, g_hWorkerThreads, true, INFINITE);

#ifdef PER_DEBUG
		//system("pause");
#endif 

		PostQuitMessage(0);
		break;
	}
	// 위의 세 메시지 외의 나머지 메세지는 OS로
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
