#include "stdafx.h"
#include "shapes_app.h"
#include "geometry_generator.h"

ShapesApp::ShapesApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
}

ShapesApp::~ShapesApp()
{
}

bool ShapesApp::Initialize()
{
	D3DApp::Initialize();

	BuildDescriptorHeaps();
	BuildFrameResources();
	BuildShapeGeometry();
	BuildConstantBuffers();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildPSO();

	return true;
}

void ShapesApp::OnResize()
{
	D3DApp::OnResize();
}

void ShapesApp::Update(const GameTimer& gt)
{
	// 다음 프레임 리소스로 설정
	m_currentFrameResourceIndex = (m_currentFrameResourceIndex + 1) % NUM_FRAME_RESOURCES;

	// GPU가 현재 프레임 리소스의 커맨드들을 다 처리했는지 확인
	if (m_currentFrameResource->fence != 0 && m_fence->GetCompletedValue() < m_currentFrameResource->fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(m_fence->SetEventOnCompletion(m_currentFrameResource->fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	// 현재 프레임 리소스 갱신
	UpdateObjectCBs(gt);
	UpdateMainPassCB(gt);
}

void ShapesApp::Draw(const GameTimer& gt)
{
	// 이 프레임의 커맨드 리스트를 구축하고 제출


	// 현재 펜스 지점까지의 커맨드들을 표시하도록 펜스 값을 전진
	m_currentFrameResource->fence = ++m_currentFence;

	// 새 펜스 지점을 설정하는 커맨드를 커맨드 큐에 추가
	m_commandQueue->Signal(m_fence.Get(), m_currentFence);

	// GPU가 아직 이전 프레임들의 명령을 처리하고 있지만 관련 리소스를 건들지 않기 때문에 문제 없음
}

void ShapesApp::UpdateObjectCBs(const GameTimer& gt)
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

void ShapesApp::UpdateMainPassCB(const GameTimer& gt)
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

void ShapesApp::BuildFrameResources()
{
	for (int i = 0; i < NUM_FRAME_RESOURCES; ++i)
	{
		m_frameResource.push_back(std::make_unique<ShapesFrameResource>(m_d3dDevice, 1, (UINT)m_allRenderItems.size()));
	}
}

void ShapesApp::BuildShapeGeometry()
{
	GeometryGenerator geoGenerator;
	GeometryGenerator::MeshData cylinder = geoGenerator.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);

	// 일단 실린더만 만들 수 있기 때문에 offset은 0으로 설정
	UINT cylinderVertexOffset = 0;
	UINT cylinderIndexOffset = 0;

	// 정점/색인 버퍼에서 각 물체가 차지하는 영역을 나타내는 SubmeshGeometry 객체 정의
	SubmeshGeometry cylinderSubmesh; 
	cylinderSubmesh.indexCount = (UINT)cylinder.indices32.size();
	cylinderSubmesh.startIndexLocation = cylinderIndexOffset;
	cylinderSubmesh.baseVertexLocation = cylinderVertexOffset;

	// 필요한 정점 성분을 추출하고 모든 메시의 정점을 하나의 정점 버퍼에 넣음
	size_t totalVertexCount = cylinder.vertices.size();

	std::vector<Vertex> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < cylinder.vertices.size(); ++i, ++k)
	{
		vertices[k].pos = cylinder.vertices[i].position;
		vertices[k].color = XMFLOAT4(Colors::SteelBlue);
	}

	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(cylinder.GetIndices16()), std::end(cylinder.GetIndices16()));

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	std::unique_ptr<MeshGeometry> geometry = std::make_unique<MeshGeometry>();
	geometry->name = "shape geometry";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geometry->vertexBuffers[0].cpu));
	CopyMemory(geometry->vertexBuffers[0].cpu->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geometry->indexBuffer.cpu));
	CopyMemory(geometry->indexBuffer.cpu->GetBufferPointer(), indices.data(), ibByteSize);

	geometry->vertexBuffers[0].gpu = D3DUtil::CreateDefaultBuffer(
			m_d3dDevice.Get(), 
			m_commandList.Get(), 
			vertices.data(), vbByteSize, 
			geometry->vertexBuffers[0].uploader);

	geometry->indexBuffer.gpu = D3DUtil::CreateDefaultBuffer(
		m_d3dDevice.Get(),
		m_commandList.Get(),
		indices.data(), ibByteSize,
		geometry->indexBuffer.uploader);

	geometry->vertexBuffers[0].byteStride = sizeof(Vertex);
	geometry->vertexBuffers[0].byteSize = vbByteSize;
	geometry->indexBuffer.format = DXGI_FORMAT_R8_UINT;
	geometry->indexBuffer.byteSize = ibByteSize;

	geometry->drawArgs["cylinder"] = cylinderSubmesh;

	m_geometries[geometry->name] = std::move(geometry);
}

void ShapesApp::BuildRenderItems()
{
	UINT objCBIndex = 0;
	for (int i = 0; i < 5; ++i)
	{
		std::unique_ptr<RenderItem> leftCylinderRederItem = std::make_unique<RenderItem>();
		std::unique_ptr<RenderItem> rightCylinderRederItem = std::make_unique<RenderItem>();

		XMMATRIX leftCylinderworld = XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f);
		XMMATRIX rightCylinderworld = XMMatrixTranslation(5.0f, 1.5f, -10.0f + i * 5.0f);

		XMStoreFloat4x4(&leftCylinderRederItem->world, leftCylinderworld);
		leftCylinderRederItem->objectCBIndex = objCBIndex++;
		leftCylinderRederItem->geometry = m_geometries["shape geometry"].get();
		leftCylinderRederItem->primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		leftCylinderRederItem->indexCount = leftCylinderRederItem->geometry->drawArgs["cylinder"].indexCount;
		leftCylinderRederItem->startIndexLocation = leftCylinderRederItem->geometry->drawArgs["cylinder"].startIndexLocation;
		leftCylinderRederItem->baseVertexLocation = leftCylinderRederItem->geometry->drawArgs["cylinder"].baseVertexLocation;

		XMStoreFloat4x4(&rightCylinderRederItem->world, rightCylinderworld);
		rightCylinderRederItem->objectCBIndex = objCBIndex++;
		rightCylinderRederItem->geometry = m_geometries["shape geometry"].get();
		rightCylinderRederItem->primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		rightCylinderRederItem->indexCount = rightCylinderRederItem->geometry->drawArgs["cylinder"].indexCount;
		rightCylinderRederItem->startIndexLocation = rightCylinderRederItem->geometry->drawArgs["cylinder"].startIndexLocation;
		rightCylinderRederItem->baseVertexLocation = rightCylinderRederItem->geometry->drawArgs["cylinder"].baseVertexLocation;

		m_allRenderItems.push_back(std::move(leftCylinderRederItem));
		m_allRenderItems.push_back(std::move(rightCylinderRederItem));
	}

	// 이 예제의 모든 렌더 항목은 불투명함
	for (auto& e : m_allRenderItems)
	{
		m_opqaueRederItems.push_back(e.get());
	}
}

void ShapesApp::BuildDescriptorHeaps()
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

	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
}

void ShapesApp::BuildConstantBufferViews()
{
	UINT objectCBByteSize = D3DUtil::CalculateConstantBufferByteSize(sizeof(ObjectConstants));
	UINT objectCount = (UINT)m_opqaueRederItems.size();
	
	// 각 프레임 리소스에 오브젝트 수 만큼의 CBV 생성
	for (int frameIndex = 0; frameIndex < NUM_FRAME_RESOURCES; ++frameIndex)
	{
		ID3D12Resource* objectCB = m_frameResource[frameIndex]->objectCB->Resource();
		for (UINT i = 0; i < objectCount; ++i)
		{
			D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objectCB->GetGPUVirtualAddress();

			// 현재 버퍼에서 i번째 물체별 상수 버퍼의 오프셋을 가상 주소에 더함
			cbAddress += i * objectCBByteSize;

			// 서술자 힙에서 i번째 물체별 CBV의 오브셋
			int heapIndex = frameIndex * objectCount + i;
			CD3DX12_CPU_DESCRIPTOR_HANDLE handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
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
		ID3D12Resource* passCB = m_frameResource[frameIndex]->passCB->Resource();

		// 패스별 버퍼는 프레임 리소스당 하나의 상수 버퍼만 저장
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = passCB->GetGPUVirtualAddress();

		// 서술자 힙 안에서 패스별 CBV의 오프셋
		int heapIndex = m_passCbvOffset + frameIndex;
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
		handle.Offset(heapIndex, m_cbvSrvDescriptorSize);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = passCBByteSize;

		m_d3dDevice->CreateConstantBufferView(&cbvDesc, handle);
	}
}

void ShapesApp::BuildRootSignature()
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

void ShapesApp::BuildShadersAndInputLayout()
{
	m_vsByteCode = D3DUtil::LoadBinary(L"./shader/shapes_vertex.cso");
	m_psByteCode = D3DUtil::LoadBinary(L"./shader/shapes_pixel.cso");
	
	m_inputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void ShapesApp::BuildPSO()
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
	psoDesc.SampleDesc.Quality = m_4xMsaaQuality ? (m_4xMsaaQuality - 1) : 0;
	psoDesc.DSVFormat = m_depthStencilFormat;
	
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pso)));
}
