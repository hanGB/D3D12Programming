#pragma once

#define WIN32_MEAN_AND_LEAN

// Direct3D 12
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

// std
#include <iostream>
#include <fstream>
#include <chrono>

// 설정
// 윈도우 기본 위치
#define PER_DEFAULT_WINDOW_LOCATION_X	200
#define PER_DEFAULT_WINDOW_LOCATION_Y	100
#define PER_DEFAULT_WINDOW_WIDTH		1920	
#define PER_DEFAULT_WINDOW_HEIGHT		1080

#define PER_NUM_WORKER_THREAD 2

// 업데이트 타임 설정
#define PER_MICROSEC_PER_UPDATE 8000
#define PER_MAXIMUM_UPDATE_LOOP_COUNT 4
// 최소 프레임 타임(최대 240FPS)
#define PER_MINIMUM_FRAME_TIME 4000

// 로그
#include "per_log.h"
