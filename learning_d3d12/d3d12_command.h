#pragma once
#include "stdafx.h"

namespace d3d12_init {

	ID3D12CommandQueue* CreateCommandQueue(ID3D12Device* d3dDevice)
	{
		ID3D12CommandQueue* d3dCommandQueue = NULL;

		D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
		::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
		d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		
		// 다이렉트 커맨드 큐 생성
		HRESULT hResult = d3dDevice->CreateCommandQueue(&d3dCommandQueueDesc, 
			__uuidof(ID3D12CommandQueue), (void**)&d3dCommandQueue);

		return d3dCommandQueue;
	}

	ID3D12CommandAllocator* CreateCommandAllocator(ID3D12Device* d3dDevice)
	{
		ID3D12CommandAllocator* d3dCommandAllocator = NULL;

		// 다이렉트 커맨드 할당자 생성
		HRESULT hResult = d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
			__uuidof(ID3D12CommandAllocator), (void**)&d3dCommandAllocator);

		return d3dCommandAllocator;
	}

	ID3D12GraphicsCommandList* CreateCommandList(ID3D12Device* d3dDevice, ID3D12CommandAllocator* d3dCommandAllocator)
	{
		ID3D12GraphicsCommandList* d3dCommandList = NULL;

		// 다이렉트 커맨드 리스트 생성
		HRESULT hResult = d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, d3dCommandAllocator, NULL,
			__uuidof(ID3D12GraphicsCommandList), (void**)&d3dCommandList);

		// 커맨드 리스트 생성 시 기본 열림이므로 닫음
		hResult = d3dCommandList->Close();

		return d3dCommandList;
	}
}
