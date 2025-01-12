#include "stdafx.h"
#include "crate_app.h"
#include "geometry_generator.h"

CrateApp::CrateApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
}

CrateApp::~CrateApp()
{
	if (m_d3dDevice)
	{
		FlushCommandQueue();
	}
}

bool CrateApp::Initialize()
{
	if (!D3DApp::Initialize())
	{
		return false;
	}

	ThrowIfFailed(m_commandList->Reset(m_commandListAllocator.Get(), nullptr));

	BuildGeometry();
	BuildMaterials();

	// 머터리얼이 텍스처보다 먼저 생성되어 srv 힙 인덱스를 설정할 수 있어야 함
	LoadTexture();
	BuildDescriptorHeap();
	BuildTextureShaderResourceViews();

	BuildRenderItems();
	BuildFrameResources();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildPSO();

	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* cmdLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	FlushCommandQueue();

	return true;
}

void CrateApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX p = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_projectionTransform, p);
}

void CrateApp::Update(const GameTimer& gt)
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
	UpdateMaterialCBs(gt);
	UpdateMainPassCB(gt);
}

void CrateApp::Draw(const GameTimer& gt)
{
	auto& cmdListAllocator = m_currentFrameResource->cmdListAllocator;

	// 커맨드 할당자 리셋
	ThrowIfFailed(cmdListAllocator->Reset());

	ThrowIfFailed(m_commandList->Reset(cmdListAllocator.Get(), m_psos["opaque"].Get()));

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

	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	// 디스크립터 힙 설정
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_srvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// 현재 프레임 리소스의 패스 CBV 설정
	ID3D12Resource* passCB = m_currentFrameResource->passCB->Resource();
	m_commandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

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

void CrateApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_lastMousePosition.x = x;
	m_lastMousePosition.y = y;

	SetCapture(m_hMainWnd);
}

void CrateApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void CrateApp::OnMouseMove(WPARAM btnState, int x, int y)
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

void CrateApp::OnKeyboradInput(WPARAM btnState, bool isPressed)
{
}

void CrateApp::DrawRenderItems(ID3D12GraphicsCommandList* commandList, const std::vector<RenderItem*>& renderItems)
{
	UINT objectCBbyteSize = D3DUtil::CalculateConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matrialCBbyteSize = D3DUtil::CalculateConstantBufferByteSize(sizeof(MaterialConstants));

	ID3D12Resource* objectCB = m_currentFrameResource->objectCB->Resource();
	ID3D12Resource* materialCB = m_currentFrameResource->materialCB->Resource();

	// 각 렌더 아이템 그리기
	for (size_t i = 0; i < renderItems.size(); ++i)
	{
		RenderItem* renderItem = renderItems[i];

		D3D12_VERTEX_BUFFER_VIEW vbs[] = { renderItem->geometry->VertexBufferView(0), renderItem->geometry->VertexBufferView(1) };
		commandList->IASetVertexBuffers(0, 2, vbs);
		commandList->IASetIndexBuffer(&renderItem->geometry->IndexBufferView());
		commandList->IASetPrimitiveTopology(renderItem->primitiveTopology);

		// 텍스처
		CD3DX12_GPU_DESCRIPTOR_HANDLE texAddress(m_srvHeap->GetGPUDescriptorHandleForHeapStart());
		texAddress.Offset(renderItem->material->diffuseSrvHeapIndex, m_cbvSrvDescriptorSize);

		// 현재 프레임 리소스에 대한 이 오브젝트를 위한 상수 버퍼 가상 주소 계산
		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress();
		objCBAddress += renderItem->objectCBIndex * objectCBbyteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = materialCB->GetGPUVirtualAddress();
		matCBAddress += renderItem->material->cbIndex * matrialCBbyteSize;

		commandList->SetGraphicsRootDescriptorTable(0, texAddress);
		commandList->SetGraphicsRootConstantBufferView(1, objCBAddress);
		commandList->SetGraphicsRootConstantBufferView(3, matCBAddress);

		commandList->DrawIndexedInstanced(
			renderItem->indexCount, 1,
			renderItem->startIndexLocation,
			renderItem->baseVertexLocation,
			0);
	}
}

void CrateApp::UpdateCamera(const GameTimer& gt)
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

void CrateApp::UpdateObjectCBs(const GameTimer& gt)
{
	UploadBuffer<ObjectConstants>* currentObjectCB = m_currentFrameResource->objectCB.get();

	for (auto& e : m_allRenderItems)
	{
		// 상수들이 바꾸었을 때에만 cbuffer 자료 갱신
		if (e->numFramesDirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&e->world);
			XMMATRIX texTransform = XMLoadFloat4x4(&e->texTransform);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.world, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.texTransform, XMMatrixTranspose(texTransform));

			currentObjectCB->CopyData(e->objectCBIndex, objConstants);

			// 다음 프레임 자원으로 넘어감
			e->numFramesDirty--;
		}
	}
}

void CrateApp::UpdateMaterialCBs(const GameTimer& gt)
{
	auto currentMaterialCB = m_currentFrameResource->materialCB.get();

	for (auto& e : m_materials)
	{
		// 상수들이 바꾸었을 때에만 cbuffer 자료 갱신
		Material* mat = e.second.get();
		if (mat->numFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->matTransform);

			MaterialConstants matConstants;
			matConstants.diffuseAlbedo = mat->diffuseAlbedo;
			matConstants.fresnelR0 = mat->fresnelR0;
			matConstants.roughness = mat->roughness;

			currentMaterialCB->CopyData(mat->cbIndex, matConstants);

			// 다음 프레임 자원으로 넘어감
			mat->numFramesDirty--;
		}
	}
}

void CrateApp::UpdateMainPassCB(const GameTimer& gt)
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
	m_mainPassCB.ambientLight = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);

	// key light
	m_mainPassCB.lights[0].direction = XMFLOAT3(1.0f, -0.5f, 1.0f);
	m_mainPassCB.lights[0].strength = XMFLOAT3(0.7f, 0.7f, 0.7f);
	// fill light
	m_mainPassCB.lights[1].direction = XMFLOAT3(-1.0f, -0.5f, 1.0f);
	m_mainPassCB.lights[1].strength = XMFLOAT3(0.4f, 0.4f, 0.4f);
	// back light
	m_mainPassCB.lights[2].direction = XMFLOAT3(-1.0f, 0.5f, -1.0f);
	m_mainPassCB.lights[2].strength = XMFLOAT3(0.2f, 0.2f, 0.2f);


	UploadBuffer<PassConstants>* currentPassCB = m_currentFrameResource->passCB.get();
	currentPassCB->CopyData(0, m_mainPassCB);
}

void CrateApp::BuildGeometry()
{
	GeometryGenerator geoGenerator;
	GeometryGenerator::MeshData box = geoGenerator.CreateBox(4.0f, 4.0f, 4.0f, 0);

	// 필요한 정점 성분들을 추출해서 각 정점에 높이 함수 적용
	std::vector<VertexBaseData> baseDatas(box.vertices.size());
	std::vector<VertexLightingData> lightingDatas(box.vertices.size());
	for (size_t i = 0; i < box.vertices.size(); ++i)
	{
		baseDatas[i].pos = box.vertices[i].position;
		baseDatas[i].uv = box.vertices[i].texCoord;
		lightingDatas[i].normal = box.vertices[i].normal;
	}
	const UINT vbByteSizes[] = { (UINT)baseDatas.size() * sizeof(VertexBaseData), (UINT)lightingDatas.size() * sizeof(VertexLightingData) };

	std::vector<std::uint16_t> indices = box.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->name = "box_geometry";

	ThrowIfFailed(D3DCreateBlob(vbByteSizes[0], &geo->vertexBuffers[0].cpu));
	CopyMemory(geo->vertexBuffers[0].cpu->GetBufferPointer(), baseDatas.data(), vbByteSizes[0]);
	ThrowIfFailed(D3DCreateBlob(vbByteSizes[1], &geo->vertexBuffers[1].cpu));
	CopyMemory(geo->vertexBuffers[1].cpu->GetBufferPointer(), lightingDatas.data(), vbByteSizes[1]);
	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->indexBuffer.cpu));
	CopyMemory(geo->indexBuffer.cpu->GetBufferPointer(), indices.data(), ibByteSize);

	geo->vertexBuffers[0].gpu = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(), m_commandList.Get(), baseDatas.data(), vbByteSizes[0], geo->vertexBuffers[0].uploader);
	geo->vertexBuffers[1].gpu = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(), m_commandList.Get(), lightingDatas.data(), vbByteSizes[1], geo->vertexBuffers[1].uploader);
	geo->indexBuffer.gpu = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(), m_commandList.Get(), indices.data(), ibByteSize, geo->indexBuffer.uploader);

	geo->vertexBuffers[0].byteStride = sizeof(VertexBaseData);
	geo->vertexBuffers[0].byteSize = vbByteSizes[0];
	geo->vertexBuffers[1].byteStride = sizeof(VertexLightingData);
	geo->vertexBuffers[1].byteSize = vbByteSizes[1];
	geo->indexBuffer.format = DXGI_FORMAT_R16_UINT;
	geo->indexBuffer.byteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.indexCount = (UINT)indices.size();
	submesh.startIndexLocation = 0;
	submesh.baseVertexLocation = 0;

	geo->drawArgs["box"] = submesh;

	m_geometries[geo->name] = std::move(geo);
}

void CrateApp::BuildMaterials()
{
	auto wood = std::make_unique<Material>();
	wood->name = "wood";
	wood->cbIndex = 0;
	wood->diffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	wood->fresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	wood->roughness = 0.125f;

	m_materials[wood->name] = std::move(wood);
}

void CrateApp::BuildRenderItems()
{
	UINT objCBIndex = 0;
	D3D_PRIMITIVE_TOPOLOGY primitiveTopogoly = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	XMMATRIX gridWorld = XMMatrixIdentity();
	std::unique_ptr<RenderItem> gridRederItem
		= CreateRenderItem(gridWorld, objCBIndex++, "box_geometry", "box", "wood", primitiveTopogoly);
	m_allRenderItems.push_back(std::move(gridRederItem));


	// 이 예제의 모든 렌더 항목은 불투명함
	for (auto& e : m_allRenderItems)
	{
		m_opqaueRederItems.push_back(e.get());
	}
}

void CrateApp::BuildFrameResources()
{
	for (int i = 0; i < NUM_FRAME_RESOURCES; ++i)
	{
		m_frameResources.push_back(std::make_unique<CrateFrameResource>(m_d3dDevice.Get(), 1, (UINT)m_allRenderItems.size(), (UINT)m_materials.size()));
	}
}

void CrateApp::BuildRootSignature()
{
	// 텍스처 heapTable 설정
	CD3DX12_DESCRIPTOR_RANGE descRange;
	descRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_ROOT_PARAMETER slotRootParameter[4];
	// 루트 CBV 생성
	slotRootParameter[0].InitAsDescriptorTable(1, &descRange, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsConstantBufferView(0);
	slotRootParameter[2].InitAsConstantBufferView(1);
	slotRootParameter[3].InitAsConstantBufferView(2);

	// 정적 샘플러
	auto staticSamplers = D3DUtil::GetStaticsSamplers();

	// 루트 시그니쳐는 루트 매개변수들의 배열
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(
		4, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSignature = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSignature.GetAddressOf(),
		errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
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

void CrateApp::BuildShadersAndInputLayout()
{
	m_shaders["standard_vs"] = D3DUtil::LoadBinary(L"../x64/Debug/texture_vertex.cso");
	m_shaders["opaque_ps"] = D3DUtil::LoadBinary(L"../x64/Debug/texture_pixel.cso");

	m_inputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void CrateApp::BuildPSO()
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
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = m_backBufferFormat;
	psoDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = m_4xMsaaQuality ? (m_4xMsaaQuality - 1) : 0;
	psoDesc.DSVFormat = m_depthStencilFormat;
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_psos["opaque"])));
}

void CrateApp::LoadTexture()
{
	auto woodCrateTex = std::make_unique<Texture>();

	woodCrateTex->name = "wood_crate_tex";
	woodCrateTex->filename = L"./resource/WoodCrate01.dds";
	std::unique_ptr<uint8_t[]> ddsData;
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	
	// 텍스처 로드
	ThrowIfFailed(LoadDDSTextureFromFile(m_d3dDevice.Get(), woodCrateTex->filename.c_str(), woodCrateTex->resource.GetAddressOf(), ddsData, subresources));

	// 업로드 힙 생성
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(woodCrateTex->resource.Get(), 0, (UINT)subresources.size());
	ThrowIfFailed(m_d3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&woodCrateTex->uploadHeap)));
	
	// 서브 리소스 데이터 업로드
	UpdateSubresources(m_commandList.Get(), woodCrateTex->resource.Get(), woodCrateTex->uploadHeap.Get(), 0, 0, static_cast<UINT>(subresources.size()), subresources.data());

	// 리소스 배리어 설정 -> 필요 없는 경우도 있지만 명시적으로 리소스 배리어를 설정하는 것이 안전함
	m_commandList->ResourceBarrier(1, 
		&CD3DX12_RESOURCE_BARRIER::Transition(
		woodCrateTex->resource.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	m_textures[woodCrateTex->name] = std::move(woodCrateTex);
}

void CrateApp::BuildDescriptorHeap()
{
	// SRV
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));
}

void CrateApp::BuildTextureShaderResourceViews()
{
	// 힙의 시작을 가리키는 포인터
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_srvHeap->GetCPUDescriptorHandleForHeapStart());

	// wood_crate_tex
	auto& woodCrateTex = m_textures["wood_crate_tex"]->resource;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = woodCrateTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = woodCrateTex->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	m_d3dDevice->CreateShaderResourceView(woodCrateTex.Get(), &srvDesc, hDescriptor);
	m_materials["wood"]->diffuseSrvHeapIndex = 0;
}

std::unique_ptr<RenderItem> CrateApp::CreateRenderItem(const XMMATRIX& world, UINT objectCBIndex,
	const char* geometry, const char* submesh, const char* material, D3D_PRIMITIVE_TOPOLOGY primitiveTopology)
{
	std::unique_ptr<RenderItem> rederItem = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&rederItem->world, world);
	rederItem->objectCBIndex = objectCBIndex;
	rederItem->geometry = m_geometries[geometry].get();
	rederItem->material = m_materials[material].get();
	rederItem->primitiveTopology = primitiveTopology;
	rederItem->indexCount = rederItem->geometry->drawArgs[submesh].indexCount;
	rederItem->startIndexLocation = rederItem->geometry->drawArgs[submesh].startIndexLocation;
	rederItem->baseVertexLocation = rederItem->geometry->drawArgs[submesh].baseVertexLocation;

	return rederItem;
}
