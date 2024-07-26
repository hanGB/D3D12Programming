#pragma once

// Direct3D 12
#define WIN32_MEAN_AND_LEAN
#include <Windows.h>
#include <wrl.h>
#include <shellapi.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <dxgidebug.h>
#include <mmsystem.h>

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib") 

// std
#include <string>
#include <vector>
