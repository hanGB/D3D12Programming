#include "stdafx.h"
#include "shapes_app.h"
#include "geometry_generator.h"

ShapesApp::ShapesApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
}

ShapesApp::~ShapesApp()
{
	if (m_d3dDevice) 
	{
		FlushCommandQueue();
	}
}

bool ShapesApp::Initialize()
{
	if (!D3DApp::Initialize())
	{
		return false;
	}

	ThrowIfFailed(m_commandList->Reset(m_commandListAllocator.Get(), nullptr));

	BuildShapeGeometry();
	BuildSkullGeometry();
	BuildMaterial();
	BuildRenderItems();
	BuildFrameResources();
	BuildDescriptorHeaps();
	//BuildDescriptorHeapsWithRootConstants();
	BuildObjectConstantBufferViews();
	//BuildMaterialConstantBufferViews();
	BuildPassConstantBufferViews();
	BuildRootSignature();
	//BuildRootSignatureWithRootConstants();
	BuildShadersAndInputLayout();
	BuildPSO();

	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* cmdLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	FlushCommandQueue();

	return true;
}

void ShapesApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX p = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_projectionTransform, p);
}

void ShapesApp::Update(const GameTimer& gt)
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

void ShapesApp::Draw(const GameTimer& gt)
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
	//DrawRenderItemsWithRootConstants(m_commandList.Get(), m_opqaueRederItems);

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

void ShapesApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_lastMousePosition.x = x;
	m_lastMousePosition.y = y;

	SetCapture(m_hMainWnd);
}

void ShapesApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void ShapesApp::OnMouseMove(WPARAM btnState, int x, int y)
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

void ShapesApp::OnKeyboradInput(WPARAM btnState, bool isPressed)
{
	if (isPressed)
	{
		if (btnState == VK_F1)
		{
			m_IsWireFrame = !m_IsWireFrame;
		}
	}
}

void ShapesApp::DrawRenderItems(ID3D12GraphicsCommandList* commandList, const std::vector<RenderItem*>& renderItems)
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
		UINT objectCbvIndex = m_currentFrameResourceIndex * (UINT)m_opqaueRederItems.size() + renderItem->objectCBIndex;
		auto objectCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_cbvHeap->GetGPUDescriptorHandleForHeapStart());
		objectCbvHandle.Offset(objectCbvIndex, m_cbvSrvDescriptorSize);

		// 머터리얼 CBV 주소 계산
		D3D12_GPU_VIRTUAL_ADDRESS materialCbvAdress = materialCB->GetGPUVirtualAddress();
		materialCbvAdress += renderItem->material->cbIndex * materialCBbyteSize;

		commandList->SetGraphicsRootDescriptorTable(0, objectCbvHandle);
		commandList->SetGraphicsRootConstantBufferView(2, materialCbvAdress);

		commandList->DrawIndexedInstanced(
			renderItem->indexCount, 1, 
			renderItem->startIndexLocation, 
			renderItem->baseVertexLocation, 
			0);
	}
}

void ShapesApp::DrawRenderItemsWithRootConstants(ID3D12GraphicsCommandList* commandList, const std::vector<RenderItem*>& renderItems)
{
	// 각 렌더 아이템 그리기
	for (size_t i = 0; i < renderItems.size(); ++i)
	{
		RenderItem* renderItem = renderItems[i];

		D3D12_VERTEX_BUFFER_VIEW vbs[] = { renderItem->geometry->VertexBufferView(0), renderItem->geometry->VertexBufferView(1) };
		commandList->IASetVertexBuffers(0, 2, vbs);
		commandList->IASetIndexBuffer(&renderItem->geometry->IndexBufferView());
		commandList->IASetPrimitiveTopology(renderItem->primitiveTopology);

		XMMATRIX wrold = XMLoadFloat4x4(&renderItem->world);
		XMFLOAT4X4 transposedWorld;
		XMStoreFloat4x4(&transposedWorld, XMMatrixTranspose(wrold));
		commandList->SetGraphicsRoot32BitConstants(0, 16, (void*)&transposedWorld, 0);

		commandList->DrawIndexedInstanced(
			renderItem->indexCount, 1,
			renderItem->startIndexLocation,
			renderItem->baseVertexLocation,
			0);
	}
}

void ShapesApp::UpdateCamera(const GameTimer& gt)
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

void ShapesApp::UpdateObjectCBs(const GameTimer& gt)
{
	UploadBuffer<ObjectConstants>* currentObjectCB = m_currentFrameResource->objectCB.get();
	
	for (auto& e : m_allRenderItems)
	{
		// 상수들이 바꾸었을 때에만 cbuffer 자료 갱신
		if (e->numFramesDirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&e->world);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.world, XMMatrixTranspose(world));

			currentObjectCB->CopyData(e->objectCBIndex, objConstants);

			// 다음 프레임 자원으로 넘어감
			e->numFramesDirty--;
		}
	}
}

void ShapesApp::UpdateMaterialCBs(const GameTimer& gt)
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
	m_mainPassCB.ambientLight = XMFLOAT4(0.01f, 0.01f, 0.01f, 1.0f);

	// key light
	m_mainPassCB.lights[0].direction = XMFLOAT3(1.0f, -0.5f, 1.0f);
	m_mainPassCB.lights[0].strength = XMFLOAT3(0.9f, 0.9f, 0.8f);
	// fill light
	m_mainPassCB.lights[1].direction = XMFLOAT3(-1.0f, -0.5f, 1.0f);
	m_mainPassCB.lights[1].strength = XMFLOAT3(0.5f, 0.5f, 0.4f);
	// back light
	m_mainPassCB.lights[2].direction = XMFLOAT3(1.0f, -0.5f, -1.0f);
	m_mainPassCB.lights[2].strength = XMFLOAT3(0.25f, 0.25f, 0.20f);

	UploadBuffer<PassConstants>* currentPassCB = m_currentFrameResource->passCB.get();
	currentPassCB->CopyData(0, m_mainPassCB);
}

void ShapesApp::BuildShapeGeometry()
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
		lightingDatas[k].normal = box.vertices[i].normal;
	}
	for (size_t i = 0; i < grid.vertices.size(); ++i, ++k)
	{
		baseDatas[k].pos = grid.vertices[i].position;
		lightingDatas[k].normal = grid.vertices[i].normal;
		
	}
	for (size_t i = 0; i < cylinder.vertices.size(); ++i, ++k)
	{
		baseDatas[k].pos = cylinder.vertices[i].position;
		lightingDatas[k].normal = cylinder.vertices[i].normal;
	}
	for (size_t i = 0; i < sphere.vertices.size(); ++i, ++k)
	{
		baseDatas[k].pos = sphere.vertices[i].position;
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

void ShapesApp::BuildSkullGeometry()
{
	std::ifstream in("./resource/skull.txt");

	if (!in)
	{
		MessageBox(0, L"./resource/skull.txtt not found.", 0, 0);
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

void ShapesApp::BuildMaterial()
{
	auto bone = std::make_unique<Material>();
	bone->name = "bone";
	bone->cbIndex = 0;
	bone->diffuseAlbedo = XMFLOAT4(Colors::Gray);
	bone->fresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	bone->roughness = 0.5f;

	auto grass = std::make_unique<Material>();
	grass->name = "grass";
	grass->cbIndex = 1;
	grass->diffuseAlbedo = XMFLOAT4(Colors::ForestGreen);
	grass->fresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	grass->roughness = 0.3f;

	auto blueSteel = std::make_unique<Material>();
	blueSteel->name = "blue_steel";
	blueSteel->cbIndex = 2;
	blueSteel->diffuseAlbedo = XMFLOAT4(Colors::SteelBlue);
	blueSteel->fresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	blueSteel->roughness = 0.05f;

	auto redPlastic = std::make_unique<Material>();
	redPlastic->name = "red_plastic";
	redPlastic->cbIndex = 3;
	redPlastic->diffuseAlbedo = XMFLOAT4(Colors::Crimson);
	redPlastic->fresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	redPlastic->roughness = 0.2f;

	auto greenWood = std::make_unique<Material>();
	greenWood->name = "green_wood";
	greenWood->cbIndex = 4;
	greenWood->diffuseAlbedo = XMFLOAT4(Colors::DarkGreen);
	greenWood->fresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	greenWood->roughness = 0.9f;

	m_materials[bone->name] = std::move(bone);
	m_materials[grass->name] = std::move(grass);
	m_materials[blueSteel->name] = std::move(blueSteel);
	m_materials[redPlastic->name] = std::move(redPlastic);
	m_materials[greenWood->name] = std::move(greenWood);
}

void ShapesApp::BuildRenderItems()
{
	UINT objCBIndex = 0;
	D3D_PRIMITIVE_TOPOLOGY primitiveTopogoly = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	XMMATRIX skullWorld = XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	std::unique_ptr<RenderItem> skullRederItem
		= CreateRenderItem(skullWorld, objCBIndex++, "skull_geometry", "skull", "bone", primitiveTopogoly);
	m_allRenderItems.push_back(std::move(skullRederItem));

	XMMATRIX boxWorld = XMMatrixScaling(2.0f, 2.0f, 2.0f) * XMMatrixTranslation(0.0f, 0.5f, 0.0f);
	std::unique_ptr<RenderItem> boxRederItem
		= CreateRenderItem(boxWorld, objCBIndex++, "shape_geometry", "box", "green_wood", primitiveTopogoly);
	m_allRenderItems.push_back(std::move(boxRederItem));

	XMMATRIX gridWorld = XMMatrixIdentity();
	std::unique_ptr<RenderItem> gridRederItem 
		= CreateRenderItem(gridWorld, objCBIndex++, "shape_geometry", "grid", "grass", primitiveTopogoly);
	m_allRenderItems.push_back(std::move(gridRederItem));

	for (int i = 0; i < 5; ++i)
	{
		XMMATRIX leftCylinderWorld = XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f);
		XMMATRIX rightCylinderWorld = XMMatrixTranslation(5.0f, 1.5f, -10.0f + i * 5.0f);
		XMMATRIX leftSphereWorld = XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f);
		XMMATRIX rightSphereWorld = XMMatrixTranslation(5.0f, 3.5f, -10.0f + i * 5.0f);

		std::unique_ptr<RenderItem> leftCylinderRederItem 
			= CreateRenderItem(leftCylinderWorld, objCBIndex++, "shape_geometry", "cylinder", "blue_steel", primitiveTopogoly);
		std::unique_ptr<RenderItem> rightCylinderRederItem 
			= CreateRenderItem(rightCylinderWorld, objCBIndex++, "shape_geometry", "cylinder", "blue_steel", primitiveTopogoly);
		std::unique_ptr<RenderItem> leftSphereRederItem 
			= CreateRenderItem(leftSphereWorld, objCBIndex++, "shape_geometry", "sphere", "red_plastic", primitiveTopogoly);
		std::unique_ptr<RenderItem> rightSphereRederItem 
			= CreateRenderItem(rightSphereWorld, objCBIndex++, "shape_geometry", "sphere", "red_plastic", primitiveTopogoly);

		m_allRenderItems.push_back(std::move(leftCylinderRederItem));
		m_allRenderItems.push_back(std::move(rightCylinderRederItem));
		m_allRenderItems.push_back(std::move(leftSphereRederItem));
		m_allRenderItems.push_back(std::move(rightSphereRederItem));
	}

	// 이 예제의 모든 렌더 항목은 불투명함
	for (auto& e : m_allRenderItems)
	{
		m_opqaueRederItems.push_back(e.get());
	}
}

void ShapesApp::BuildFrameResources()
{
	for (int i = 0; i < NUM_FRAME_RESOURCES; ++i)
	{
		m_frameResources.push_back(std::make_unique<ShapesFrameResource>(m_d3dDevice.Get(), 1, (UINT)m_allRenderItems.size(), (UINT)m_materials.size()));
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

	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvHeap)));
}

void ShapesApp::BuildDescriptorHeapsWithRootConstants()
{
	// 각 프레임 리소스의 패스별 CBV 하나 필요
	UINT numDescriptors = NUM_FRAME_RESOURCES;

	// 패스별 CBV의 시작 오프셋 저장
	m_passCbvOffset = 0;

	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = numDescriptors;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;

	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvHeap)));
}

void ShapesApp::BuildObjectConstantBufferViews()
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
}

void ShapesApp::BuildPassConstantBufferViews()
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
		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
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

	CD3DX12_ROOT_PARAMETER slotRootParameter[3];
	// 루트 CBV 생성
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable0, D3D12_SHADER_VISIBILITY_VERTEX);
	slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable1);
	slotRootParameter[2].InitAsConstantBufferView(2, 0, D3D12_SHADER_VISIBILITY_PIXEL);

	// 루트 시그니쳐는 루트 매개변수들의 배열
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(
		3, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	
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

void ShapesApp::BuildRootSignatureWithRootConstants()
{
	CD3DX12_DESCRIPTOR_RANGE cbvTable1;
	cbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	CD3DX12_ROOT_PARAMETER slotRootParameter[3];

	// 루트 CBV 생성
	slotRootParameter[0].InitAsConstants(16, 0);
	slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable1);
	slotRootParameter[2].InitAsConstantBufferView(2);


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
	m_shaders["standard_vs"] = D3DUtil::LoadBinary(L"./shader/shapes_light_vertex.cso");
	m_shaders["opaque_ps"] = D3DUtil::LoadBinary(L"./shader/shapes_light_pixel.cso");

	m_inputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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

std::unique_ptr<RenderItem> ShapesApp::CreateRenderItem(const XMMATRIX& world, UINT objectCBIndex,
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
