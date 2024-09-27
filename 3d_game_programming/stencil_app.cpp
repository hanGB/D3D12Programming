#include "stdafx.h"
#include "stencil_app.h"
#include "geometry_generator.h"

StencilApp::StencilApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
}

StencilApp::~StencilApp()
{
	if (m_d3dDevice)
	{
		FlushCommandQueue();
	}
}

bool StencilApp::Initialize()
{
	if (!D3DApp::Initialize())
	{
		return false;
	}

	ThrowIfFailed(m_commandList->Reset(m_commandListAllocator.Get(), nullptr));

	BuildShapeGeometry();
	BuildSkullGeometry();
	BuildMaterialAndTexture();
	BuildRenderItems();
	BuildFrameResources();
	BuildDescriptorHeaps();
	BuildObjectConstantBufferViews();
	BuildPassConstantBufferViews();
	BuildTextureShaderResourceViews();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildPSO();

	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* cmdLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	FlushCommandQueue();

	return true;
}

void StencilApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX p = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_projectionTransform, p);
}

void StencilApp::Update(const GameTimer& gt)
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

void StencilApp::Draw(const GameTimer& gt)
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

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_cbvSrvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	// 현재 프레임 리소스의 패스 CBV 설정
	int passCbvIndex = m_passCbvOffset + m_currentFrameResourceIndex;
	auto passCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_cbvSrvHeap->GetGPUDescriptorHandleForHeapStart());
	passCbvHandle.Offset(passCbvIndex, m_cbvSrvDescriptorSize);
	m_commandList->SetGraphicsRootDescriptorTable(2, passCbvHandle);

	// 스텐실에 거울 영역 그리기
	m_commandList->OMSetStencilRef(1);
	m_commandList->SetPipelineState(m_psos["mark_stencil_mirrors"].Get());
	DrawRenderItems(m_commandList.Get(), m_renderItemsEachRenderLayers[RenderLayer::Mirrors]);

	// 반사상 그리기
	m_commandList->SetPipelineState(m_psos["draw_stencil_reflections"].Get());
	DrawRenderItems(m_commandList.Get(), m_renderItemsEachRenderLayers[RenderLayer::Reflected]);
	m_commandList->OMSetStencilRef(0);

	// 거울 그리기
	m_commandList->SetPipelineState(m_psos["transparent"].Get());
	DrawRenderItems(m_commandList.Get(), m_renderItemsEachRenderLayers[RenderLayer::Transparent]);

	// 불투명 그리기(거울로 가려진 벽 부분이 뚫려 있지 않기 때문에 거울을 먼저 그려 깊이 테스트로 거울로 가려진 벽 부분이 그려지지 않도록 해야 함)
	m_commandList->SetPipelineState(m_psos["opaque"].Get());
	DrawRenderItems(m_commandList.Get(), m_renderItemsEachRenderLayers[RenderLayer::Opaque]);

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

void StencilApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_lastMousePosition.x = x;
	m_lastMousePosition.y = y;

	SetCapture(m_hMainWnd);
}

void StencilApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void StencilApp::OnMouseMove(WPARAM btnState, int x, int y)
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

void StencilApp::OnKeyboradInput(WPARAM btnState, bool isPressed)
{

}

void StencilApp::DrawRenderItems(ID3D12GraphicsCommandList* commandList, const std::vector<RenderItem*>& renderItems)
{
	UINT objectCBbyteSize = D3DUtil::CalculateConstantBufferByteSize(sizeof(ObjectConstants));
	UINT materialCBbyteSize = D3DUtil::CalculateConstantBufferByteSize(sizeof(MaterialConstants));

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

		// 현재 프레임 리소스에 대한 서술자 힙에서 이 오브젝트를 위한 CBV 오프셋 계산
		UINT objectCbvIndex = m_currentFrameResourceIndex * (UINT)m_allRenderItems.size() + renderItem->objectCBIndex;
		auto objectCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_cbvSrvHeap->GetGPUDescriptorHandleForHeapStart());
		objectCbvHandle.Offset(objectCbvIndex, m_cbvSrvDescriptorSize);
		// 텍스처 SRV 핸들
		CD3DX12_GPU_DESCRIPTOR_HANDLE texSrvHandle(m_cbvSrvHeap->GetGPUDescriptorHandleForHeapStart());
		texSrvHandle.Offset(m_textureSrvOffset + renderItem->material->diffuseSrvHeapIndex, m_cbvSrvDescriptorSize);

		// 머터리얼 CBV 주소 계산
		D3D12_GPU_VIRTUAL_ADDRESS materialCbvAdress = materialCB->GetGPUVirtualAddress();
		materialCbvAdress += renderItem->material->cbIndex * materialCBbyteSize;

		commandList->SetGraphicsRootDescriptorTable(0, texSrvHandle);
		commandList->SetGraphicsRootDescriptorTable(1, objectCbvHandle);
		commandList->SetGraphicsRootConstantBufferView(3, materialCbvAdress);

		commandList->DrawIndexedInstanced(
			renderItem->indexCount, 1,
			renderItem->startIndexLocation,
			renderItem->baseVertexLocation,
			0);
	}
}

void StencilApp::UpdateCamera(const GameTimer& gt)
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

void StencilApp::UpdateObjectCBs(const GameTimer& gt)
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

void StencilApp::UpdateMaterialCBs(const GameTimer& gt)
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

void StencilApp::UpdateMainPassCB(const GameTimer& gt)
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
	m_mainPassCB.ambientLight = XMFLOAT4(0.01f, 0.01f, 0.01f, 1.0f);

	m_mainPassCB.gFogColor = XMFLOAT4(Colors::LightSteelBlue);
	m_mainPassCB.gFogStart = 30.0f;
	m_mainPassCB.gFogRange = 100.0f;

	XMVECTOR lightDirection = -MathHelper::SphericalToCartesian(1.0f, 1.75f * XM_PI, XM_PIDIV4);
	XMStoreFloat3(&m_mainPassCB.lights[0].direction, lightDirection);
	m_mainPassCB.lights[0].strength = XMFLOAT3(0.8f, 0.8f, 0.8f);

	UploadBuffer<PassConstants>* currentPassCB = m_currentFrameResource->passCB.get();
	currentPassCB->CopyData(0, m_mainPassCB);
}

void StencilApp::BuildShapeGeometry()
{
	GeometryGenerator geoGenerator;

	GeometryGenerator::MeshData box = geoGenerator.CreateBox(1.5f, 0.5f, 1.5f, 3);
	GeometryGenerator::MeshData grid = geoGenerator.CreateGrid(20.0f, 30.0f, 30, 20);
	GeometryGenerator::MeshData cylinder = geoGenerator.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);
	GeometryGenerator::MeshData sphere = geoGenerator.CreateSphere(0.5f, 20, 20);

	// 각 메시 별 오프셋 설정
	UINT boxVertexOffset = 0;
	UINT gridVertexOffset = boxVertexOffset + (UINT)box.vertices.size();
	UINT cylinderVertexOffset = gridVertexOffset + (UINT)grid.vertices.size();
	UINT sphereVertexOffset = cylinderVertexOffset + (UINT)cylinder.vertices.size();

	UINT boxIndexOffset = 0;
	UINT gridIndexOffset = boxIndexOffset + (UINT)box.indices32.size();
	UINT cylinderIndexOffset = gridIndexOffset + (UINT)grid.indices32.size();
	UINT sphereIndexOffset = cylinderIndexOffset + (UINT)cylinder.indices32.size();

	// 정점/색인 버퍼에서 각 메시가 차지하는 영역을 나타내는 SubmeshGeometry 객체 정의
	SubmeshGeometry boxSubmesh;
	boxSubmesh.indexCount = (UINT)box.indices32.size();
	boxSubmesh.startIndexLocation = boxIndexOffset;
	boxSubmesh.baseVertexLocation = boxVertexOffset;

	SubmeshGeometry gridSubmesh;
	gridSubmesh.indexCount = (UINT)grid.indices32.size();
	gridSubmesh.startIndexLocation = gridIndexOffset;
	gridSubmesh.baseVertexLocation = gridVertexOffset;

	SubmeshGeometry cylinderSubmesh;
	cylinderSubmesh.indexCount = (UINT)cylinder.indices32.size();
	cylinderSubmesh.startIndexLocation = cylinderIndexOffset;
	cylinderSubmesh.baseVertexLocation = cylinderVertexOffset;

	SubmeshGeometry sphereSubmesh;
	sphereSubmesh.indexCount = (UINT)sphere.indices32.size();
	sphereSubmesh.startIndexLocation = sphereIndexOffset;
	sphereSubmesh.baseVertexLocation = sphereVertexOffset;

	// 필요한 정점 성분을 추출하고 모든 메시의 정점을 하나의 정점 버퍼에 넣음
	size_t totalVertexCount = box.vertices.size() + grid.vertices.size() + cylinder.vertices.size() + sphere.vertices.size();

	std::vector<VertexBaseData> baseDatas(totalVertexCount);
	std::vector<VertexLightingData> lightingDatas(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.vertices.size(); ++i, ++k)
	{
		baseDatas[k].pos = box.vertices[i].position;
		baseDatas[k].uv = box.vertices[i].texCoord;
		lightingDatas[k].normal = box.vertices[i].normal;
	}
	for (size_t i = 0; i < grid.vertices.size(); ++i, ++k)
	{
		baseDatas[k].pos = grid.vertices[i].position;
		baseDatas[k].uv = grid.vertices[i].texCoord;
		lightingDatas[k].normal = grid.vertices[i].normal;

	}
	for (size_t i = 0; i < cylinder.vertices.size(); ++i, ++k)
	{
		baseDatas[k].pos = cylinder.vertices[i].position;
		baseDatas[k].uv = cylinder.vertices[i].texCoord;
		lightingDatas[k].normal = cylinder.vertices[i].normal;
	}
	for (size_t i = 0; i < sphere.vertices.size(); ++i, ++k)
	{
		baseDatas[k].pos = sphere.vertices[i].position;
		baseDatas[k].uv = sphere.vertices[i].texCoord;
		lightingDatas[k].normal = sphere.vertices[i].normal;
	}

	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
	indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));
	indices.insert(indices.end(), std::begin(cylinder.GetIndices16()), std::end(cylinder.GetIndices16()));
	indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));

	const UINT vbByteSizes[] = { (UINT)baseDatas.size() * sizeof(VertexBaseData), (UINT)lightingDatas.size() * sizeof(VertexLightingData) };
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	std::unique_ptr<MeshGeometry> geometry = std::make_unique<MeshGeometry>();
	geometry->name = "shape_geometry";

	ThrowIfFailed(D3DCreateBlob(vbByteSizes[0], &geometry->vertexBuffers[0].cpu));
	CopyMemory(geometry->vertexBuffers[0].cpu->GetBufferPointer(), baseDatas.data(), vbByteSizes[0]);

	ThrowIfFailed(D3DCreateBlob(vbByteSizes[1], &geometry->vertexBuffers[1].cpu));
	CopyMemory(geometry->vertexBuffers[1].cpu->GetBufferPointer(), lightingDatas.data(), vbByteSizes[1]);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geometry->indexBuffer.cpu));
	CopyMemory(geometry->indexBuffer.cpu->GetBufferPointer(), indices.data(), ibByteSize);

	geometry->vertexBuffers[0].gpu = D3DUtil::CreateDefaultBuffer(
		m_d3dDevice.Get(),
		m_commandList.Get(),
		baseDatas.data(), vbByteSizes[0],
		geometry->vertexBuffers[0].uploader);
	geometry->vertexBuffers[1].gpu = D3DUtil::CreateDefaultBuffer(
		m_d3dDevice.Get(),
		m_commandList.Get(),
		lightingDatas.data(), vbByteSizes[1],
		geometry->vertexBuffers[1].uploader);

	geometry->indexBuffer.gpu = D3DUtil::CreateDefaultBuffer(
		m_d3dDevice.Get(),
		m_commandList.Get(),
		indices.data(), ibByteSize,
		geometry->indexBuffer.uploader);

	geometry->vertexBuffers[0].byteStride = sizeof(VertexBaseData);
	geometry->vertexBuffers[0].byteSize = vbByteSizes[0];
	geometry->vertexBuffers[1].byteStride = sizeof(VertexLightingData);
	geometry->vertexBuffers[1].byteSize = vbByteSizes[1];
	geometry->indexBuffer.format = DXGI_FORMAT_R16_UINT; // D3D12에서는 오직 DXGI_FORMAT_R16_UINT와 DXGI_FORMAT_R32_UINT만 유효함
	geometry->indexBuffer.byteSize = ibByteSize;

	geometry->drawArgs["box"] = boxSubmesh;
	geometry->drawArgs["grid"] = gridSubmesh;
	geometry->drawArgs["cylinder"] = cylinderSubmesh;
	geometry->drawArgs["sphere"] = sphereSubmesh;

	m_geometries[geometry->name] = std::move(geometry);
}

void StencilApp::BuildSkullGeometry()
{
	std::ifstream in("./resource/skull.txt");

	if (!in)
	{
		MessageBox(0, L"./resource/skull.txt not found.", 0, 0);
		return;
	}

	int vertexCount;
	int triangleCount;
	std::string ignore;

	// 파일 앞 있는 갯수 읽기
	in >> ignore >> vertexCount;
	in >> ignore >> triangleCount;

	// 설명 건너 뛰기(단어 하나씩)
	in >> ignore >> ignore >> ignore >> ignore;

	// 버텍스 읽기
	std::vector<VertexBaseData> baseDatas(vertexCount);
	std::vector<VertexLightingData> lightingDatas(vertexCount);
	for (size_t i = 0; i < vertexCount; ++i)
	{
		baseDatas[i].uv = { 0.0f, 0.0f };

		in >> baseDatas[i].pos.x >> baseDatas[i].pos.y >> baseDatas[i].pos.z;
		in >> lightingDatas[i].normal.x >> lightingDatas[i].normal.y >> lightingDatas[i].normal.z;
	}

	in >> ignore >> ignore >> ignore;

	// 인덱스 읽기
	size_t indexCount = triangleCount * 3;
	std::vector<uint16_t> indices(indexCount);
	for (size_t i = 0; i < indexCount; ++i)
	{
		in >> indices[i];
	}

	in.close();

	const UINT vbByteSizes[] = { (UINT)baseDatas.size() * sizeof(VertexBaseData), (UINT)lightingDatas.size() * sizeof(VertexLightingData) };
	const UINT ibByteSize = (UINT)indexCount * sizeof(uint16_t);

	SubmeshGeometry submesh;
	submesh.indexCount = (UINT)indexCount;
	submesh.startIndexLocation = 0;
	submesh.baseVertexLocation = 0;

	// 지오메트리 생성
	std::unique_ptr<MeshGeometry> geometry = std::make_unique<MeshGeometry>();
	geometry->name = "skull_geometry";

	ThrowIfFailed(D3DCreateBlob(vbByteSizes[0], &geometry->vertexBuffers[0].cpu));
	CopyMemory(geometry->vertexBuffers[0].cpu->GetBufferPointer(), baseDatas.data(), vbByteSizes[0]);
	ThrowIfFailed(D3DCreateBlob(vbByteSizes[1], &geometry->vertexBuffers[1].cpu));
	CopyMemory(geometry->vertexBuffers[1].cpu->GetBufferPointer(), lightingDatas.data(), vbByteSizes[1]);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geometry->indexBuffer.cpu));
	CopyMemory(geometry->indexBuffer.cpu->GetBufferPointer(), indices.data(), ibByteSize);

	geometry->vertexBuffers[0].gpu
		= D3DUtil::CreateDefaultBuffer(
			m_d3dDevice.Get(),
			m_commandList.Get(),
			baseDatas.data(),
			vbByteSizes[0],
			geometry->vertexBuffers[0].uploader);
	geometry->vertexBuffers[1].gpu
		= D3DUtil::CreateDefaultBuffer(
			m_d3dDevice.Get(),
			m_commandList.Get(),
			lightingDatas.data(),
			vbByteSizes[1],
			geometry->vertexBuffers[1].uploader);

	geometry->indexBuffer.gpu
		= D3DUtil::CreateDefaultBuffer(
			m_d3dDevice.Get(),
			m_commandList.Get(),
			indices.data(),
			ibByteSize,
			geometry->indexBuffer.uploader);

	geometry->vertexBuffers[0].byteStride = sizeof(VertexBaseData);
	geometry->vertexBuffers[0].byteSize = vbByteSizes[0];
	geometry->vertexBuffers[1].byteStride = sizeof(VertexLightingData);
	geometry->vertexBuffers[1].byteSize = vbByteSizes[1];
	geometry->indexBuffer.byteSize = ibByteSize;
	geometry->indexBuffer.format = DXGI_FORMAT_R16_UINT;

	geometry->drawArgs["skull"] = submesh;

	m_geometries[geometry->name] = std::move(geometry);
}

void StencilApp::BuildMaterialAndTexture()
{
	auto bricks = std::make_unique<Material>();
	bricks->name = "bricks";
	bricks->cbIndex = 0;
	bricks->diffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	bricks->fresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	bricks->roughness = 0.8f;
	bricks->diffuseSrvHeapIndex = 0;
	m_textures["bricks"] = D3DUtil::CreateTextureFromDDSFile("bricks", L"./resource/bricks3.dds", m_d3dDevice.Get(), m_commandList.Get());

	auto checkboard = std::make_unique<Material>();
	checkboard->name = "checkboard";
	checkboard->cbIndex = 1;
	checkboard->diffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	checkboard->fresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	checkboard->roughness = 0.05f;
	checkboard->diffuseSrvHeapIndex = 1;
	m_textures["checkboard"] = D3DUtil::CreateTextureFromDDSFile("checkboard", L"./resource/checkboard.dds", m_d3dDevice.Get(), m_commandList.Get());

	auto ice = std::make_unique<Material>();
	ice->name = "ice";
	ice->cbIndex = 2;
	ice->diffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.3f);
	ice->fresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	ice->roughness = 0.01f;
	ice->diffuseSrvHeapIndex = 2;
	m_textures["ice"] = D3DUtil::CreateTextureFromDDSFile("ice", L"./resource/ice.dds", m_d3dDevice.Get(), m_commandList.Get());

	auto white = std::make_unique<Material>();
	white->name = "white";
	white->cbIndex = 3;
	white->diffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	white->fresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	white->roughness = 0.3f;
	white->diffuseSrvHeapIndex = 3;
	m_textures["white"] = D3DUtil::CreateTextureFromDDSFile("white", L"./resource/white1x1.dds", m_d3dDevice.Get(), m_commandList.Get());

	m_materials[bricks->name] = std::move(bricks);
	m_materials[checkboard->name] = std::move(checkboard);
	m_materials[ice->name] = std::move(ice);
	m_materials[white->name] = std::move(white);
}

void StencilApp::BuildRenderItems()
{
	UINT objCBIndex = 0;
	D3D_PRIMITIVE_TOPOLOGY primitiveTopogoly = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// 불투명
	XMMATRIX floorWorld = XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(5.0f, 0.0f, 0.0f);
	XMMATRIX floorTexTransform = XMMatrixScaling(5.0f, 5.0f, 1.0f);
	std::unique_ptr<RenderItem> floorRederItem
		= CreateRenderItem(floorWorld, floorTexTransform, objCBIndex++, "shape_geometry", "grid", "checkboard", primitiveTopogoly);
	m_renderItemsEachRenderLayers[RenderLayer::Opaque].push_back(floorRederItem.get());
	m_allRenderItems.push_back(std::move(floorRederItem));

	XMMATRIX wallWorld = XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixRotationRollPitchYaw(0.0f, 0.0f, -0.5f * XM_PI) * XMMatrixTranslation(0.0f, 5.f, 0.0f);
	XMMATRIX wallTexTransform = XMMatrixScaling(5.0f, 5.0f, 1.0f) * XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.5 * XM_PI);
	std::unique_ptr<RenderItem> wallRederItem
		= CreateRenderItem(wallWorld, wallTexTransform, objCBIndex++, "shape_geometry", "grid", "bricks", primitiveTopogoly);
	m_renderItemsEachRenderLayers[RenderLayer::Opaque].push_back(wallRederItem.get());
	m_allRenderItems.push_back(std::move(wallRederItem));

	XMMATRIX skullWorld = XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(5.0f, 1.0f, 0.0f);
	XMMATRIX skullTexTransform = XMMatrixIdentity();
	std::unique_ptr<RenderItem> skullRenderItem
		= CreateRenderItem(skullWorld, skullTexTransform, objCBIndex++, "skull_geometry", "skull", "white", primitiveTopogoly);
	m_renderItemsEachRenderLayers[RenderLayer::Opaque].push_back(skullRenderItem.get());
	m_allRenderItems.push_back(std::move(skullRenderItem));

	// 거울
	XMMATRIX mirrorWorld = XMMatrixScaling(0.35f, 0.35f, 0.35f) * XMMatrixRotationRollPitchYaw(0.0f, 0.0f, -0.5f * XM_PI) * XMMatrixTranslation(0.001f, 3.5f, 0.0f);
	XMMATRIX mirrorTexTransform = XMMatrixIdentity() * XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.5 * XM_PI);
	std::unique_ptr<RenderItem> mirrorRederItem
		= CreateRenderItem(mirrorWorld, mirrorTexTransform, objCBIndex++, "shape_geometry", "grid", "ice", primitiveTopogoly);
	m_renderItemsEachRenderLayers[RenderLayer::Mirrors].push_back(mirrorRederItem.get());
	m_renderItemsEachRenderLayers[RenderLayer::Transparent].push_back(mirrorRederItem.get());
	m_allRenderItems.push_back(std::move(mirrorRederItem));

	// 반사상
	XMMATRIX reflectedFloorWorld = XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(-5.0f, 0.0f, 0.0f);
	XMMATRIX reflectedFloorTexTransform = XMMatrixScaling(5.0f, 5.0f, 1.0f);
	std::unique_ptr<RenderItem> reflectedFloorRederItem
		= CreateRenderItem(reflectedFloorWorld, reflectedFloorTexTransform, objCBIndex++, "shape_geometry", "grid", "checkboard", primitiveTopogoly);
	m_renderItemsEachRenderLayers[RenderLayer::Reflected].push_back(reflectedFloorRederItem.get());
	m_allRenderItems.push_back(std::move(reflectedFloorRederItem));

	XMMATRIX reflectedSkullWorld = XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(-5.0f, 1.0f, 0.0f);
	XMMATRIX reflectedSkullTexTransform = XMMatrixIdentity();
	std::unique_ptr<RenderItem> reflectedSkullRenderItem
		= CreateRenderItem(reflectedSkullWorld, reflectedSkullTexTransform, objCBIndex++, "skull_geometry", "skull", "white", primitiveTopogoly);
	m_renderItemsEachRenderLayers[RenderLayer::Reflected].push_back(reflectedSkullRenderItem.get());
	m_allRenderItems.push_back(std::move(reflectedSkullRenderItem));
}

void StencilApp::BuildFrameResources()
{
	for (int i = 0; i < NUM_FRAME_RESOURCES; ++i)
	{
		m_frameResources.push_back(std::make_unique<StencilFrameResource>(m_d3dDevice.Get(), 1, (UINT)m_allRenderItems.size(), (UINT)m_materials.size()));
	}
}

void StencilApp::BuildDescriptorHeaps()
{
	UINT objectCount = (UINT)m_allRenderItems.size();

	// 각 프레임 리소스의 오브젝트마다 하나의 서술자 필요 + 각 프레임 리소스의 패스별 CBV 하나 필요
	UINT numDescriptors = (objectCount + 1) * NUM_FRAME_RESOURCES + (UINT)m_textures.size();

	// 패스별 CBV의 시작 오프셋 저장
	m_passCbvOffset = objectCount * NUM_FRAME_RESOURCES;
	// 텍스처 SRV 시작  오프셋 저장
	m_textureSrvOffset = (objectCount + 1) * NUM_FRAME_RESOURCES;

	D3D12_DESCRIPTOR_HEAP_DESC cbvSrvHeapDesc;
	cbvSrvHeapDesc.NumDescriptors = numDescriptors;
	cbvSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvSrvHeapDesc.NodeMask = 0;

	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&cbvSrvHeapDesc, IID_PPV_ARGS(&m_cbvSrvHeap)));
}

void StencilApp::BuildObjectConstantBufferViews()
{
	UINT objectCBByteSize = D3DUtil::CalculateConstantBufferByteSize(sizeof(ObjectConstants));
	UINT objectCount = (UINT)m_allRenderItems.size();

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
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cbvSrvHeap->GetCPUDescriptorHandleForHeapStart());
			handle.Offset(heapIndex, m_cbvSrvDescriptorSize);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = cbAddress;
			cbvDesc.SizeInBytes = objectCBByteSize;

			m_d3dDevice->CreateConstantBufferView(&cbvDesc, handle);
		}
	}
}

void StencilApp::BuildPassConstantBufferViews()
{
	UINT passCBByteSize = D3DUtil::CalculateConstantBufferByteSize(sizeof(PassConstants));

	// 마지막 세 서술자는 각 프레임 자원의 패스별 CBV
	for (int frameIndex = 0; frameIndex < NUM_FRAME_RESOURCES; ++frameIndex)
	{
		ID3D12Resource* passCB = m_frameResources[frameIndex]->passCB->Resource();

		// 패스별 버퍼는 프레임 리소스당 하나의 상수 버퍼만 저장
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = passCB->GetGPUVirtualAddress();

		// 서술자 힙 안에서 패스별 CBV의 오프셋
		int heapIndex = m_passCbvOffset + frameIndex;
		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cbvSrvHeap->GetCPUDescriptorHandleForHeapStart());
		handle.Offset(heapIndex, m_cbvSrvDescriptorSize);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = passCBByteSize;

		m_d3dDevice->CreateConstantBufferView(&cbvDesc, handle);
	}
}

void StencilApp::BuildTextureShaderResourceViews()
{
	auto resource = m_textures["bricks"]->resource.Get();
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_cbvSrvHeap->GetCPUDescriptorHandleForHeapStart());
	hDescriptor.Offset(m_textureSrvOffset + m_materials["bricks"]->diffuseSrvHeapIndex, m_cbvSrvDescriptorSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = resource->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = resource->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;
	m_d3dDevice->CreateShaderResourceView(resource, &srvDesc, hDescriptor);

	hDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cbvSrvHeap->GetCPUDescriptorHandleForHeapStart());
	hDescriptor.Offset(m_textureSrvOffset + m_materials["checkboard"]->diffuseSrvHeapIndex, m_cbvSrvDescriptorSize);
	resource = m_textures["checkboard"]->resource.Get();
	srvDesc.Format = resource->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = resource->GetDesc().MipLevels;
	m_d3dDevice->CreateShaderResourceView(resource, &srvDesc, hDescriptor);

	hDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cbvSrvHeap->GetCPUDescriptorHandleForHeapStart());
	hDescriptor.Offset(m_textureSrvOffset + m_materials["ice"]->diffuseSrvHeapIndex, m_cbvSrvDescriptorSize);
	resource = m_textures["ice"]->resource.Get();
	srvDesc.Format = resource->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = resource->GetDesc().MipLevels;
	m_d3dDevice->CreateShaderResourceView(resource, &srvDesc, hDescriptor);	

	hDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cbvSrvHeap->GetCPUDescriptorHandleForHeapStart());
	hDescriptor.Offset(m_textureSrvOffset + m_materials["white"]->diffuseSrvHeapIndex, m_cbvSrvDescriptorSize);
	resource = m_textures["white"]->resource.Get();
	srvDesc.Format = resource->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = resource->GetDesc().MipLevels;
	m_d3dDevice->CreateShaderResourceView(resource, &srvDesc, hDescriptor);
}

void StencilApp::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE srvTable;
	srvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE cbvTable0;
	cbvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	CD3DX12_DESCRIPTOR_RANGE cbvTable1;
	cbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	CD3DX12_ROOT_PARAMETER slotRootParameter[4];
	// 루트 파라미터 생성
	slotRootParameter[0].InitAsDescriptorTable(1, &srvTable, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable0);
	slotRootParameter[2].InitAsDescriptorTable(1, &cbvTable1);
	slotRootParameter[3].InitAsConstantBufferView(2);

	auto staticSampler = D3DUtil::GetStaticsSamplers();

	// 루트 시그니쳐는 루트 매개변수들의 배열
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(
		_countof(slotRootParameter), slotRootParameter, (UINT)staticSampler.size(), staticSampler.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

void StencilApp::BuildShadersAndInputLayout()
{
	m_shaders["standard_vs"] = D3DUtil::LoadBinary(L"../x64/Debug/texture_vertex.cso");
	m_shaders["opaque_ps"] = D3DUtil::LoadBinary(L"../x64/Debug/texture_pixel.cso");

	m_inputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void StencilApp::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;
	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
	opaquePsoDesc.pRootSignature = m_rootSignature.Get();
	opaquePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_shaders["standard_vs"]->GetBufferPointer()),
		m_shaders["standard_vs"]->GetBufferSize()
	};
	opaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_shaders["opaque_ps"]->GetBufferPointer()),
		m_shaders["opaque_ps"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = m_backBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m_4xMsaaQuality ? (m_4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = m_depthStencilFormat;
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&m_psos["opaque"])));

	// 투명
	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentPsoDesc = opaquePsoDesc;
	D3D12_RENDER_TARGET_BLEND_DESC blendDesc;
	blendDesc.BlendEnable = true;
	blendDesc.LogicOpEnable = false;
	blendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	transparentPsoDesc.BlendState.RenderTarget[0] = blendDesc;
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&transparentPsoDesc, IID_PPV_ARGS(&m_psos["transparent"])));

	// 스텐실 거울 영역 표시용
	CD3DX12_BLEND_DESC mirrorBlendDesc(D3D12_DEFAULT);
	mirrorBlendDesc.RenderTarget[0].RenderTargetWriteMask = 0;

	D3D12_DEPTH_STENCIL_DESC mirrorDSDesc;
	mirrorDSDesc.DepthEnable = true;
	mirrorDSDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	mirrorDSDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	mirrorDSDesc.StencilEnable = true;
	mirrorDSDesc.StencilReadMask = 0xff;
	mirrorDSDesc.StencilWriteMask = 0xff;
	mirrorDSDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	mirrorDSDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	mirrorDSDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	mirrorDSDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	mirrorDSDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	mirrorDSDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	mirrorDSDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	mirrorDSDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC markMirrorsPsoDesc = opaquePsoDesc;
	markMirrorsPsoDesc.BlendState = mirrorBlendDesc;
	markMirrorsPsoDesc.DepthStencilState = mirrorDSDesc;
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&markMirrorsPsoDesc, IID_PPV_ARGS(&m_psos["mark_stencil_mirrors"])));

	// 스텐실 반사상
	D3D12_DEPTH_STENCIL_DESC reflectionsDSDesc;
	reflectionsDSDesc.DepthEnable = true;
	reflectionsDSDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	reflectionsDSDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	reflectionsDSDesc.StencilEnable = true;
	reflectionsDSDesc.StencilReadMask = 0xff;
	reflectionsDSDesc.StencilWriteMask = 0xff;
	reflectionsDSDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;
	reflectionsDSDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC drawStencilReflectionsDesc = opaquePsoDesc;
	drawStencilReflectionsDesc.DepthStencilState = reflectionsDSDesc;
	drawStencilReflectionsDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	//drawStencilReflectionsDesc.RasterizerState.FrontCounterClockwise = true;
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&drawStencilReflectionsDesc, IID_PPV_ARGS(&m_psos["draw_stencil_reflections"])));
}

std::unique_ptr<RenderItem> StencilApp::CreateRenderItem(const XMMATRIX& world, const XMMATRIX& texTransform, UINT objectCBIndex,
	const char* geometry, const char* submesh, const char* material, D3D_PRIMITIVE_TOPOLOGY primitiveTopology)
{
	std::unique_ptr<RenderItem> rederItem = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&rederItem->world, world);
	XMStoreFloat4x4(&rederItem->texTransform, texTransform);
	rederItem->objectCBIndex = objectCBIndex;
	rederItem->geometry = m_geometries[geometry].get();
	rederItem->material = m_materials[material].get();
	rederItem->primitiveTopology = primitiveTopology;
	rederItem->indexCount = rederItem->geometry->drawArgs[submesh].indexCount;
	rederItem->startIndexLocation = rederItem->geometry->drawArgs[submesh].startIndexLocation;
	rederItem->baseVertexLocation = rederItem->geometry->drawArgs[submesh].baseVertexLocation;

	return rederItem;
}
