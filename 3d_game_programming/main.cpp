#include "stdafx.h"
#include "init_direct3d_app.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	// 디버그 빌드시 실행시점 메모리 점검 기능 온
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		InitDirect3DApp app(hInstance);

		if (!app.Initialize()) return 0;

		return app.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);

		return 0;
	}
}