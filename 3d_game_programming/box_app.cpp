#include "stdafx.h"
#include "box_app.h"

BoxApp::BoxApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
}

BoxApp::~BoxApp()
{
}

bool BoxApp::Initialize()
{
	if (!D3DApp::Initialize())
		return false;

	// 커맨드 리스트 리셋
	ThrowIfFailed(m_commandList->Reset(m_commandListAllocator.Get(), nullptr));

	BuildDescriptorHeaps();
	BuildConstantBuffers();
	BuildRootSignature();
	BuildshadersAndInputLayout();
	BuildBoxGeometry();
	BuildPSO();

	// 초기화 커맨드 실행
	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// 초기화가 완료될 때까지 대기
	FlushCommandQueue();

	return true;
}

void BoxApp::OnResize()
{
	D3DApp::OnResize();

	// 윈도우 크기가 변경되었으므로 종횡비를 갱신하고 투명 행렬 다시 계산
	XMMATRIX projection = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_projectionMatrix, projection);
}

void BoxApp::Update(const GameTimer& gt)
{
	// 구면 좌표를 직교 좌표로 변환
	float x = m_radius * sinf(m_phi) * cosf(m_theta);
	float z = m_radius * sinf(m_phi) * sinf(m_theta);
	float y = m_radius * cosf(m_phi);

	// 시야 행렬 구축
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_viewMatrix, view);

	XMMATRIX world = XMLoadFloat4x4(&m_worldTransform);
	XMMATRIX projection = XMLoadFloat4x4(&m_projectionMatrix);
	XMMATRIX worldViewProjection = world * view * projection;

	// 최신의 worldViewProjection 행렬로 상수 버퍼 갱신
	ObjectConstants objConstants;
	XMStoreFloat4x4(&objConstants.worldViewProjection, XMMatrixTranspose(worldViewProjection));
	m_objectCB->CopyData(0, objConstants);
}

void BoxApp::Draw(const GameTimer& gt)
{
	// 커맨드 할당자 리셋
	ThrowIfFailed(m_commandListAllocator->Reset());

	// 커맨드 리스트 리셋
	ThrowIfFailed(m_commandList->Reset(m_commandListAllocator.Get(), m_pso.Get()));

	// 뷰포트, 씨져 설정
	m_commandList->RSSetViewports(1, &m_screenViewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	// 리소스 용도에 관련된 상태 전이를 통지
	m_commandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET));

	// 후면 버퍼와 깊이 버퍼 클리어
	m_commandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::Black, 0, nullptr);
	m_commandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0, 0, 0, nullptr);

	// 렌더링 결과가 기록될 렌더 타겟 버퍼 지정
	m_commandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	// 상수 버퍼 힙 설정
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_cbvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// 루트 시그니쳐 설정
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	// 버텍스 버퍼, 인덱스 버퍼, 토폴로지 설정
	m_commandList->IASetVertexBuffers(0, 1, &m_boxGeometry->VertexBufferView());
	m_commandList->IASetIndexBuffer(&m_boxGeometry->IndexBufferView());
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 쉐이더의 b0에 상수 버퍼 힙 연결
	m_commandList->SetGraphicsRootDescriptorTable(0, m_cbvHeap->GetGPUDescriptorHandleForHeapStart());
	// 그리기 커맨드
	m_commandList->DrawIndexedInstanced(m_boxGeometry->drawArgs["box"].indexCount, 1, 0, 0, 0);

	// 리소스 용도에 관련된 상태 전이를 통지
	m_commandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT));

	// 커맨드 기록 종료
	ThrowIfFailed(m_commandList->Close());

	// 커맨드 실행을 위해 커맨드 리스트를 커맨드 큐에 추가
	ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// 후면 버퍼와 전면 버퍼 교환
	ThrowIfFailed(m_swapChain->Present(0, 0));
	m_currentBackBuffer = (m_currentBackBuffer + 1) % c_SWAP_CHAIN_BUFFER_COUNT;

	// 이 프레임의 명령들이 모두 처리되길 대기(비효율적)
	FlushCommandQueue();
}

void BoxApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_lastMousePosition.x = x;
	m_lastMousePosition.y = y;

	SetCapture(m_hMainWnd);
}

void BoxApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void BoxApp::OnMouseMove(WPARAM btnState, int x, int y)
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

void BoxApp::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvHeap)));
}

void BoxApp::BuildConstantBuffers()
{
	m_objectCB = std::make_unique<UploadBuffer<ObjectConstants>>(m_d3dDevice.Get(), 1, true);
	UINT objCBByteSize = D3DUtil::CalculateConstantBufferByteSize(sizeof(ObjectConstants));

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_objectCB->Resource()->GetGPUVirtualAddress();
	// 버퍼에서 i번째 물체의 상수 버퍼의 오프셋을 얻음(현재 i = 0)
	int boxCBIndex = 0;
	cbAddress += boxCBIndex * objCBByteSize;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = D3DUtil::CalculateConstantBufferByteSize(sizeof(ObjectConstants));

	m_d3dDevice->CreateConstantBufferView(&cbvDesc, m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
}

void BoxApp::BuildRootSignature()
{
	// 루트 시그니쳐가 세이더 프로그램이 기대하는 리소스들을 정의함
	// 루트 매개변수는 디스크립터 테이블이거나 루트 디스크립터 또는 루트 상수
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	// CBV 하나를 담는 디스크립터 테이블 생성
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

	// 루트 시그니쳐는 루트 매개변수들의 배열
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(
		1, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// 상수 버퍼 하나로 구성된 디스크립터 구간을 가리키는 슬롯 하나로 이루어진 루트 시그니쳐 생성
	ComPtr<ID3DBlob> serializedRootSignature = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(
		&rootSignatureDesc, 
		D3D_ROOT_SIGNATURE_VERSION_1, 
		serializedRootSignature.GetAddressOf(), 
		errorBlob.GetAddressOf());

	if (errorBlob)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(m_d3dDevice->CreateRootSignature(
		0,
		serializedRootSignature->GetBufferPointer(),
		serializedRootSignature->GetBufferSize(),
		IID_PPV_ARGS(&m_rootSignature)));
}

void BoxApp::BuildshadersAndInputLayout()
{
	HRESULT hr = S_OK;
	m_vsByteCode = D3DUtil::LoadBinary(L"./shader/box_vertex.cso");
	m_psByteCode = D3DUtil::LoadBinary(L"./shader/box_pixel.cso");

	m_inputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
		  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
		  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void BoxApp::BuildBoxGeometry()
{
	// 버텍스, 인덱스 데이터
	std::array<Vertex, 8> vertices = 
	{
		Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta) })
	};
	std::array<std::uint16_t, 36> indices =
	{
		0, 1, 2,
		0, 2, 3,
		4, 6, 5,
		4, 7, 6,
		4, 5, 1,
		4, 1, 0,
		3, 2, 6,
		3, 6, 7,
		1, 5, 6,
		1, 6, 2,
		4, 0, 3,
		4, 3, 7
	};

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	m_boxGeometry = std::make_unique<MeshGeometry>();
	m_boxGeometry->name = "boxGeometry";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &m_boxGeometry->vertexBufferCPU));
	CopyMemory(m_boxGeometry->vertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
	ThrowIfFailed(D3DCreateBlob(ibByteSize, &m_boxGeometry->indexBufferCPU));
	CopyMemory(m_boxGeometry->indexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	m_boxGeometry->vertexBufferGPU = D3DUtil::CreateDefaultBuffer(
		m_d3dDevice.Get(), m_commandList.Get(), vertices.data(), vbByteSize, m_boxGeometry->vertexBufferUploader);
	m_boxGeometry->indexBufferGPU = D3DUtil::CreateDefaultBuffer(
		m_d3dDevice.Get(), m_commandList.Get(), indices.data(), ibByteSize, m_boxGeometry->indexBufferUploader);

	m_boxGeometry->vertexByteStride = sizeof(Vertex);
	m_boxGeometry->vertexBufferByteSize = vbByteSize;
	m_boxGeometry->indexFormat = DXGI_FORMAT_R16_UINT;
	m_boxGeometry->indexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.indexCount = (UINT)indices.size();
	submesh.startIndexLocation = 0;
	submesh.baseVertexLocation = 0;

	m_boxGeometry->drawArgs["box"] = submesh;
}

void BoxApp::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_vsByteCode->GetBufferPointer()),
		m_vsByteCode->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_psByteCode->GetBufferPointer()),
		m_psByteCode->GetBufferSize()
	};
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = m_backBufferFormat;
	psoDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
	psoDesc.DSVFormat = m_depthStencilFormat;

	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pso)));
}