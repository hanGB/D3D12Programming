#pragma once
#include "stdafx.h"

namespace d3d12_init {

	ID3D12CommandQueue* CreateCommandQueue(ID3D12Device* device)
	{
		ID3D12CommandQueue* commandQueue = NULL;

		D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
		::ZeroMemory(&commandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
		commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		
		// 다이렉트 커맨드 큐 생성
		HRESULT result = device->CreateCommandQueue(&commandQueueDesc, 
			__uuidof(ID3D12CommandQueue), (void**)&commandQueue);

		return commandQueue;
	}

	ID3D12CommandAllocator* CreateCommandAllocator(ID3D12Device* device)
	{
		ID3D12CommandAllocator* commandAllocator = NULL;

		// 다이렉트 커맨드 할당자 생성
		HRESULT result = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
			__uuidof(ID3D12CommandAllocator), (void**)&commandAllocator);

		return commandAllocator;
	}

	ID3D12GraphicsCommandList* CreateCommandList(ID3D12Device* device, ID3D12CommandAllocator* commandAllocator)
	{
		ID3D12GraphicsCommandList* commandList = NULL;

		// 다이렉트 커맨드 리스트 생성
		HRESULT result = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, NULL,
			__uuidof(ID3D12GraphicsCommandList), (void**)&commandList);

		// 커맨드 리스트 생성 시 기본 열림이므로 닫음
		result = commandList->Close();

		return commandList;
	}
}
