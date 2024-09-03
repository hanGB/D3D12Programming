#include "stdafx.h"
#include "land_and_waves_app.h"
#include "geometry_generator.h"

LandAndWavesApp::LandAndWavesApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
}

LandAndWavesApp::~LandAndWavesApp()
{
	if (m_d3dDevice)
	{
		FlushCommandQueue();
	}
}

bool LandAndWavesApp::Initialize()
{
	if (!D3DApp::Initialize())
	{
		return false;
	}

	ThrowIfFailed(m_commandList->Reset(m_commandListAllocator.Get(), nullptr));

	BuildLandGeometry();
	BuildRenderItems();
	BuildFrameResources();
	BuildDescriptorHeaps();
	BuildConstantBufferViews();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildPSO();

	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* cmdLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	FlushCommandQueue();

	return true;
}

void LandAndWavesApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX p = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_projectionTransform, p);
}

void LandAndWavesApp::Update(const GameTimer& gt)
{
	// 다음 프레임 리소스로 설정
	m_currentFrameResourceIndex = (m_currentFrameResourceIndex + 1) % NUM_FRAME_RESOURCES;
	m_currentFrameResource = m_frameResources[m_currentFrameResourceIndex].get();

	// GPU가 현재 프레임 리소스의 커맨드들을 다 처리했는지 확인
	if (m_currentFrameResource->fence != 0 && m_fence->GetCompletedValue() < m_currentFrameResource->fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(m_fence->SetEventOnCompletion(m_currentFrameResource->fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	// 카메라 업데이트
	UpdateCamera(gt);

	// 현재 프레임 리소스 갱신
	UpdateObjectCBs(gt);
	UpdateMainPassCB(gt);
}

void LandAndWavesApp::Draw(const GameTimer& gt)
{
	auto& cmdListAllocator = m_currentFrameResource->cmdListAllocator;

	// 커맨드 할당자 리셋
	ThrowIfFailed(cmdListAllocator->Reset());

	if (m_IsWireFrame)
	{
		ThrowIfFailed(m_commandList->Reset(cmdListAllocator.Get(), m_psos["opaque_wirefame"].Get()));
	}
	else
	{
		ThrowIfFailed(m_commandList->Reset(cmdListAllocator.Get(), m_psos["opaque"].Get()));
	}

	m_commandList->RSSetViewports(1, &m_screenViewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	// 리소스 용도에 관련된 상태 전이 통지
	m_commandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET));

	// 후면 버퍼 / 깊이 버퍼 클리어
	m_commandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	m_commandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// 렌더링 결과가 기록될 렌더 타켓 버퍼 저장
	m_commandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_cbvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	// 현재 프레임 리소스의 패스 CBV 설정
	int passCbvIndex = m_passCbvOffset + m_currentFrameResourceIndex;
	auto passCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_cbvHeap->GetGPUDescriptorHandleForHeapStart());
	passCbvHandle.Offset(passCbvIndex, m_cbvSrvDescriptorSize);
	m_commandList->SetGraphicsRootDescriptorTable(1, passCbvHandle);

	// 렌더 아이템 그리기
	DrawRenderItems(m_commandList.Get(), m_opqaueRederItems);

	// 리소스 용도에 관련된 상태 전이 통지
	m_commandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT));

	// 커맨드 기록 종료
	ThrowIfFailed(m_commandList->Close());

	// 커맨드 실행을 위해 커맨드 리스트를 커맨드 큐에 추가
	ID3D12CommandList* cmdLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	// 후면 버퍼와 전면 버퍼 교환
	ThrowIfFailed(m_swapChain->Present(0, 0));
	m_currentBackBuffer = (m_currentBackBuffer + 1) % c_SWAP_CHAIN_BUFFER_COUNT;

	// 현재 펜스 지점까지의 커맨드들을 표시하도록 펜스 값을 전진
	m_currentFrameResource->fence = ++m_currentFence;

	// 새 펜스 지점을 설정하는 커맨드를 커맨드 큐에 추가
	m_commandQueue->Signal(m_fence.Get(), m_currentFence);

	// GPU가 아직 이전 프레임들의 명령을 처리하고 있지만 관련 리소스를 건들지 않기 때문에 문제 없음
}

void LandAndWavesApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_lastMousePosition.x = x;
	m_lastMousePosition.y = y;

	SetCapture(m_hMainWnd);
}

void LandAndWavesApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void LandAndWavesApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// 마우스 한 픽셀 이동을 4분의 1도에 대응
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - m_lastMousePosition.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - m_lastMousePosition.y));

		// 마우스 입력에 기초에 각도 갱신(카메라가 상자를 중심으로 공전)
		m_theta += dx;
		m_phi += dy;

		// m_phi 각도 제한
		m_phi = MathHelper::Clamp(m_phi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// 마우스 한 픽셀 이동을 장면의 0.005단위에 대응
		float dx = XMConvertToRadians(0.05f * static_cast<float>(x - m_lastMousePosition.x));
		float dy = XMConvertToRadians(0.05f * static_cast<float>(y - m_lastMousePosition.y));

		// 마우스 입력에 기초해서 카메라 반지름 갱신
		m_radius += dx - dy;

		// 반지름 제한
		m_radius = MathHelper::Clamp(m_radius, 3.0f, 15.0f);
	}


	m_lastMousePosition.x = x;
	m_lastMousePosition.y = y;
}

void LandAndWavesApp::OnKeyboradUse(WPARAM btnState, bool isPressed)
{
	if (btnState == VK_F1)
	{
		if (isPressed)
		{
			m_IsWireFrame = !m_IsWireFrame;
		}
	}
}

void LandAndWavesApp::DrawRenderItems(ID3D12GraphicsCommandList* commandList, const std::vector<RenderItem*>& renderItems)
{
	UINT objectCBbyteSize = D3DUtil::CalculateConstantBufferByteSize(sizeof(ObjectConstants));

	ID3D12Resource* objectCB = m_currentFrameResource->objectCB->Resource();

	// 각 렌더 아이템 그리기
	for (size_t i = 0; i < renderItems.size(); ++i)
	{
		RenderItem* renderItem = renderItems[i];

		commandList->IASetVertexBuffers(0, 1, &renderItem->geometry->VertexBufferView(0));
		commandList->IASetIndexBuffer(&renderItem->geometry->IndexBufferView());
		commandList->IASetPrimitiveTopology(renderItem->primitiveTopology);

		// 현재 프레임 리소스에 대한 서술자 힙에서 이 오브젝트를 위한 CBV 오프셋 계산
		UINT cbvIndex = m_currentFrameResourceIndex * (UINT)m_opqaueRederItems.size() + renderItem->objectCBIndex;
		auto cbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_cbvHeap->GetGPUDescriptorHandleForHeapStart());
		cbvHandle.Offset(cbvIndex, m_cbvSrvDescriptorSize);

		commandList->SetGraphicsRootDescriptorTable(0, cbvHandle);

		commandList->DrawIndexedInstanced(
			renderItem->indexCount, 1,
			renderItem->startIndexLocation,
			renderItem->baseVertexLocation,
			0);
	}
}

void LandAndWavesApp::UpdateCamera(const GameTimer& gt)
{
	// 구면 좌표를 직교 좌표로 변환
	m_eyePosition.x = m_radius * sinf(m_phi) * cosf(m_theta);
	m_eyePosition.z = m_radius * sinf(m_phi) * sinf(m_theta);
	m_eyePosition.y = m_radius * cosf(m_phi);

	// 시야 행렬 구축
	XMVECTOR pos = XMVectorSet(m_eyePosition.x, m_eyePosition.y, m_eyePosition.z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_viewTransform, view);
}

void LandAndWavesApp::UpdateObjectCBs(const GameTimer& gt)
{
	UploadBuffer<ObjectConstants>* currentObjectCB = m_currentFrameResource->objectCB.get();

	for (auto& e : m_allRenderItems)
	{
		// 상수들이 바꾸었을 때에만 cbuffer 자료 갱신
		if (e->numFamesDirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&e->world);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.world, XMMatrixTranspose(world));

			currentObjectCB->CopyData(e->objectCBIndex, objConstants);

			// 다음 프레임 자원으로 넘어감
			e->numFamesDirty--;
		}
	}
}

void LandAndWavesApp::UpdateMainPassCB(const GameTimer& gt)
{
	XMMATRIX view = XMLoadFloat4x4(&m_viewTransform);
	XMMATRIX projection = XMLoadFloat4x4(&m_projectionTransform);

	XMMATRIX viewProjection = XMMatrixMultiply(view, projection);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProjection = XMMatrixInverse(&XMMatrixDeterminant(projection), projection);
	XMMATRIX invViewProjection = XMMatrixInverse(&XMMatrixDeterminant(viewProjection), viewProjection);

	XMStoreFloat4x4(&m_mainPassCB.view, XMMatrixTranspose(view));
	XMStoreFloat4x4(&m_mainPassCB.invView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&m_mainPassCB.projection, XMMatrixTranspose(projection));
	XMStoreFloat4x4(&m_mainPassCB.invProjection, XMMatrixTranspose(invProjection));
	XMStoreFloat4x4(&m_mainPassCB.viewProjection, XMMatrixTranspose(viewProjection));
	XMStoreFloat4x4(&m_mainPassCB.invViewProjection, XMMatrixTranspose(invViewProjection));
	m_mainPassCB.eyePosW = m_eyePosition;
	m_mainPassCB.renderTargetSize = XMFLOAT2((float)m_clientWidth, (float)m_clientHeight);
	m_mainPassCB.invRenderTargetSize = XMFLOAT2(1.0f / (float)m_clientWidth, 1.0f / (float)m_clientHeight);
	m_mainPassCB.nearZ = 1.0f;
	m_mainPassCB.farZ = 1000.0f;
	m_mainPassCB.totalTime = gt.TotalTime();
	m_mainPassCB.deltaTime = gt.DeltaTime();

	UploadBuffer<PassConstants>* currentPassCB = m_currentFrameResource->passCB.get();
	currentPassCB->CopyData(0, m_mainPassCB);
}

void LandAndWavesApp::BuildLandGeometry()
{
	GeometryGenerator geoGenerator;
	GeometryGenerator::MeshData grid = geoGenerator.CreateGrid(160.0f, 160.0f, 50, 50);

	// 필요한 정점 성분들을 추출해서 각 정점에 높이 함수 적용
	std::vector<Vertex> vertices(grid.vertices.size());
	for (size_t i = 0; i < grid.vertices.size(); ++i)
	{
		XMFLOAT3& p = grid.vertices[i].position;
		vertices[i].pos = p;
		vertices[i].pos.y = GetHillsHeight(p.x, p.z);

		// 높이에 기초해서 정점 색성 설정
		if (vertices[i].pos.y < -10.0f)
		{
			// 해변의 모래색
			vertices[i].color = XMFLOAT4(1.0f, 0.96f, 0.62f, 1.0f);
		}
		else if (vertices[i].pos.y < 5.0f)
		{
			// 밝은 녹황색
			vertices[i].color = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
		}
		else if (vertices[i].pos.y < 12.0f)
		{
			// 짙은 녹황색
			vertices[i].color = XMFLOAT4(0.1f, 0.48f, 0.19f, 1.0f);
		}
		else if (vertices[i].pos.y < 20.0f)
		{
			// 짙은 갈생
			vertices[i].color = XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);
		}
		else
		{
			// 흰색(눈)
			vertices[i].color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}
	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = grid.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->name = "land_geometry";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->vertexBuffers[0].cpu));
	CopyMemory(geo->vertexBuffers[0].cpu->GetBufferPointer(), vertices.data(), vbByteSize);
	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->indexBuffer.cpu));
	CopyMemory(geo->indexBuffer.cpu->GetBufferPointer(), indices.data(), ibByteSize);

	geo->vertexBuffers[0].gpu = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(), m_commandList.Get(), vertices.data(), vbByteSize, geo->vertexBuffers[0].uploader);
	geo->indexBuffer.gpu = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(), m_commandList.Get(), indices.data(), ibByteSize, geo->indexBuffer.uploader);

	geo->vertexBuffers[0].byteStride = sizeof(Vertex);
	geo->vertexBuffers[0].byteSize = vbByteSize;
	geo->indexBuffer.format = DXGI_FORMAT_R16_UINT;
	geo->indexBuffer.byteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.indexCount = (UINT)indices.size();
	submesh.startIndexLocation = 0;
	submesh.baseVertexLocation = 0;

	geo->drawArgs["grid"] = submesh;

	m_geometries[geo->name] = std::move(geo);
}

void LandAndWavesApp::BuildRenderItems()
{
	UINT objCBIndex = 0;
	D3D_PRIMITIVE_TOPOLOGY primitiveTopogoly = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	XMMATRIX gridWorld = XMMatrixIdentity();
	std::unique_ptr<RenderItem> gridRederItem
		= CreateRenderItem(gridWorld, objCBIndex++, "land_geometry", "grid", primitiveTopogoly);
	m_allRenderItems.push_back(std::move(gridRederItem));


	// 이 예제의 모든 렌더 항목은 불투명함
	for (auto& e : m_allRenderItems)
	{
		m_opqaueRederItems.push_back(e.get());
	}
}

void LandAndWavesApp::BuildFrameResources()
{
	for (int i = 0; i < NUM_FRAME_RESOURCES; ++i)
	{
		m_frameResources.push_back(std::make_unique<ShapesFrameResource>(m_d3dDevice.Get(), 1, (UINT)m_allRenderItems.size()));
	}
}

void LandAndWavesApp::BuildDescriptorHeaps()
{
	UINT objectCount = (UINT)m_opqaueRederItems.size();

	// 각 프레임 리소스의 오브젝트마다 하나의 서술자 필요 + 각 프레임 리소스의 패스별 CBV 하나 필요
	UINT numDescriptors = (objectCount + 1) * NUM_FRAME_RESOURCES;

	// 패스별 CBV의 시작 오프셋 저장
	m_passCbvOffset = objectCount * NUM_FRAME_RESOURCES;

	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = numDescriptors;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;

	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvHeap)));
}

void LandAndWavesApp::BuildConstantBufferViews()
{
	UINT objectCBByteSize = D3DUtil::CalculateConstantBufferByteSize(sizeof(ObjectConstants));
	UINT objectCount = (UINT)m_opqaueRederItems.size();

	// 각 프레임 리소스에 오브젝트 수 만큼의 CBV 생성
	for (int frameIndex = 0; frameIndex < NUM_FRAME_RESOURCES; ++frameIndex)
	{
		ID3D12Resource* objectCB = m_frameResources[frameIndex]->objectCB->Resource();
		for (UINT i = 0; i < objectCount; ++i)
		{
			D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objectCB->GetGPUVirtualAddress();

			// 현재 버퍼에서 i번째 물체별 상수 버퍼의 오프셋을 가상 주소에 더함
			cbAddress += i * objectCBByteSize;

			// 서술자 힙에서 i번째 물체별 CBV의 오브셋
			int heapIndex = frameIndex * objectCount + i;
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
			handle.Offset(heapIndex, m_cbvSrvDescriptorSize);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = cbAddress;
			cbvDesc.SizeInBytes = objectCBByteSize;

			m_d3dDevice->CreateConstantBufferView(&cbvDesc, handle);
		}
	}

	UINT passCBByteSize = D3DUtil::CalculateConstantBufferByteSize(sizeof(PassConstants));

	// 마지막 세 서술자는 각 프레임 자원의 패스별 CBV
	for (int frameIndex = 0; frameIndex < NUM_FRAME_RESOURCES; ++frameIndex)
	{
		ID3D12Resource* passCB = m_frameResources[frameIndex]->passCB->Resource();

		// 패스별 버퍼는 프레임 리소스당 하나의 상수 버퍼만 저장
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = passCB->GetGPUVirtualAddress();

		// 서술자 힙 안에서 패스별 CBV의 오프셋
		int heapIndex = m_passCbvOffset + frameIndex;
		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
		handle.Offset(heapIndex, m_cbvSrvDescriptorSize);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = passCBByteSize;

		m_d3dDevice->CreateConstantBufferView(&cbvDesc, handle);
	}
}

void LandAndWavesApp::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE cbvTable0;
	cbvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	CD3DX12_DESCRIPTOR_RANGE cbvTable1;
	cbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	CD3DX12_ROOT_PARAMETER slotRootParameter[2];

	// 루트 CBV 생성
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable0);
	slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable1);

	// 루트 시그니쳐는 루트 매개변수들의 배열
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(
		2, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSignature = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSignature.GetAddressOf(),
		errorBlob.GetAddressOf());

	ThrowIfFailed(m_d3dDevice->CreateRootSignature(
		0,
		serializedRootSignature->GetBufferPointer(),
		serializedRootSignature->GetBufferSize(),
		IID_PPV_ARGS(&m_rootSignature)));
}

void LandAndWavesApp::BuildShadersAndInputLayout()
{
	m_shaders["standard_vs"] = D3DUtil::LoadBinary(L"./shader/shapes_vertex.cso");
	m_shaders["opaque_ps"] = D3DUtil::LoadBinary(L"./shader/shapes_pixel.cso");

	m_inputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void LandAndWavesApp::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_shaders["standard_vs"]->GetBufferPointer()),
		m_shaders["standard_vs"]->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_shaders["opaque_ps"]->GetBufferPointer()),
		m_shaders["opaque_ps"]->GetBufferSize()
	};
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = m_backBufferFormat;
	psoDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = m_4xMsaaQuality ? (m_4xMsaaQuality - 1) : 0;
	psoDesc.DSVFormat = m_depthStencilFormat;
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_psos["opaque_wirefame"])));

	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_psos["opaque"])));
}

float LandAndWavesApp::GetHillsHeight(float x, float z) const
{
	return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

std::unique_ptr<RenderItem> LandAndWavesApp::CreateRenderItem(const XMMATRIX& world, UINT objectCBIndex,
	const char* geometry, const char* submesh, D3D_PRIMITIVE_TOPOLOGY primitiveTopology)
{
	std::unique_ptr<RenderItem> rederItem = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&rederItem->world, world);
	rederItem->objectCBIndex = objectCBIndex;
	rederItem->geometry = m_geometries[geometry].get();
	rederItem->primitiveTopology = primitiveTopology;
	rederItem->indexCount = rederItem->geometry->drawArgs[submesh].indexCount;
	rederItem->startIndexLocation = rederItem->geometry->drawArgs[submesh].startIndexLocation;
	rederItem->baseVertexLocation = rederItem->geometry->drawArgs[submesh].baseVertexLocation;

	return rederItem;
}
