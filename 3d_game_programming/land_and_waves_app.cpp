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
	
	m_waves = std::make_unique<Waves>(128, 128, 1.0f, 0.03f, 4.0f, 0.2f);

	BuildLandGeometry();
	BuildWavesGeometry();
	BuildTreeGeometry();
	BuildParticle();
	BuildTexture();
	BuildMaterialAndSrv();
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

	// 텍스처 애니메이션
	AnimateMaterials(gt);

	// 파도 업데이트
	UpdateWaves(gt);

	// 카메라 업데이트
	UpdateCamera(gt);

	// 현재 프레임 리소스 갱신
	UpdateObjectCBs(gt);
	UpdateMaterialCBs(gt);
	UpdateParticleCBs(gt);
	UpdateMainPassCB(gt);
}

void LandAndWavesApp::Draw(const GameTimer& gt)
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

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_srvHeap.Get()};
	m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	// 현재 프레임 리소스의 패스 CBV 설정
	ID3D12Resource* passCB = m_currentFrameResource->passCB->Resource();
	m_commandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	// 각 레이어 별로 렌더 아이템 그리기
	DrawRenderItems(m_commandList.Get(), m_renderItemsEachRenderLayers[RenderLayer::Opaque]);

	if (m_isDebugSurfaceNormal)
	{
		m_commandList->SetPipelineState(m_psos["debug_surface_normal"].Get());
		DrawRenderItems(m_commandList.Get(), m_renderItemsEachRenderLayers[RenderLayer::DebugSurfaceNormal]);
	}
	else
	{
		m_commandList->SetPipelineState(m_psos["debug_normal"].Get());
		DrawRenderItems(m_commandList.Get(), m_renderItemsEachRenderLayers[RenderLayer::DebugNormal]);
	}

	m_commandList->SetPipelineState(m_psos["lod_sphere"].Get());
	DrawRenderItems(m_commandList.Get(), m_renderItemsEachRenderLayers[RenderLayer::LODSphere]);

	m_commandList->SetPipelineState(m_psos["triangle_explosion"].Get());
	DrawRenderItems(m_commandList.Get(), m_renderItemsEachRenderLayers[RenderLayer::TriangleExplosion]);

	m_commandList->SetPipelineState(m_psos["alpha_test"].Get());
	DrawRenderItems(m_commandList.Get(), m_renderItemsEachRenderLayers[RenderLayer::AlphaTest]);

	m_commandList->SetPipelineState(m_psos["tree_sprite"].Get());
	DrawRenderItems(m_commandList.Get(), m_renderItemsEachRenderLayers[RenderLayer::TreeSprite]);

	m_commandList->SetPipelineState(m_psos["transparent"].Get());
	DrawRenderItems(m_commandList.Get(), m_renderItemsEachRenderLayers[RenderLayer::Transparent]);

	m_commandList->SetPipelineState(m_psos["gravity_particle"].Get());
	DrawRenderItemsForParticle(m_commandList.Get(), m_renderItemsEachRenderLayers[RenderLayer::Particle]);

	m_commandList->SetPipelineState(m_psos["gravity_particle_infinity"].Get());
	DrawRenderItemsForParticle(m_commandList.Get(), m_renderItemsEachRenderLayers[RenderLayer::ParticleInfinity]);

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
		// 마우스 한 픽셀 이동을 장면의 5단위에 대응
		float dx = XMConvertToRadians(5.f * static_cast<float>(x - m_lastMousePosition.x));
		float dy = XMConvertToRadians(5.f * static_cast<float>(y - m_lastMousePosition.y));

		// 마우스 입력에 기초해서 카메라 반지름 갱신
		m_radius += dx - dy;

		// 반지름 제한
		m_radius = MathHelper::Clamp(m_radius, 3.0f, 150.0f);
	}


	m_lastMousePosition.x = x;
	m_lastMousePosition.y = y;
}

void LandAndWavesApp::OnKeyboradInput(WPARAM btnState, bool isPressed)
{
	if (isPressed)
	{
		const float dt = m_timer.DeltaTime();

		if (btnState == VK_LEFT)
		{
			m_sunTheta -= 10.0f * dt;
		}
		if (btnState == VK_RIGHT)
		{
			m_sunTheta += 10.0f * dt;
		}
		if (btnState == VK_UP)
		{
			m_sunPhi -= 10.0f * dt;
		}
		if (btnState == VK_DOWN)
		{
			m_sunPhi += 10.0f * dt;
		}

		if (btnState == 'n' || btnState == 'N')
		{
			m_isDebugSurfaceNormal = !m_isDebugSurfaceNormal;
		}
	}
}

void LandAndWavesApp::DrawRenderItems(ID3D12GraphicsCommandList* commandList, const std::vector<RenderItem*>& renderItems)
{
	UINT objectCBbyteSize = D3DUtil::CalculateConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matrialCBbyteSize = D3DUtil::CalculateConstantBufferByteSize(sizeof(MaterialConstants));

	ID3D12Resource* objectCB = m_currentFrameResource->objectCB->Resource();
	ID3D12Resource* materialCB = m_currentFrameResource->materialCB->Resource();

	// 각 렌더 아이템 그리기
	for (size_t i = 0; i < renderItems.size(); ++i)
	{
		RenderItem* renderItem = renderItems[i];

		if (renderItem->geometry->vertexBufferCount == 1)
		{
			D3D12_VERTEX_BUFFER_VIEW vbvs[] = { renderItem->geometry->VertexBufferView(0) };
			commandList->IASetVertexBuffers(0, renderItem->geometry->vertexBufferCount, vbvs);
		}
		else
		{
			D3D12_VERTEX_BUFFER_VIEW vbvs[] = { renderItem->geometry->VertexBufferView(0), renderItem->geometry->VertexBufferView(1) };
			commandList->IASetVertexBuffers(0, renderItem->geometry->vertexBufferCount, vbvs);
		}		
		commandList->IASetIndexBuffer(&renderItem->geometry->IndexBufferView());
		commandList->IASetPrimitiveTopology(renderItem->primitiveTopology);

		// 텍스처 서술자 핸들
		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(m_srvHeap->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(renderItem->material->diffuseSrvHeapIndex, m_cbvSrvDescriptorSize);

		// 현재 프레임 리소스에 대한 이 오브젝트를 위한 상수 버퍼 가상 주소 계산
		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress();
		objCBAddress += renderItem->objectCBIndex * objectCBbyteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = materialCB->GetGPUVirtualAddress();
		matCBAddress += renderItem->material->cbIndex * matrialCBbyteSize;

		commandList->SetGraphicsRootDescriptorTable(0, tex);
		commandList->SetGraphicsRootConstantBufferView(1, objCBAddress);
		commandList->SetGraphicsRootConstantBufferView(3, matCBAddress);

		commandList->DrawIndexedInstanced(
			renderItem->indexCount, 1,
			renderItem->startIndexLocation,
			renderItem->baseVertexLocation,
			0);
	}
}

void LandAndWavesApp::DrawRenderItemsForParticle(ID3D12GraphicsCommandList* commandList, const std::vector<RenderItem*>& renderItems)
{
	UINT objectCBbyteSize = D3DUtil::CalculateConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matrialCBbyteSize = D3DUtil::CalculateConstantBufferByteSize(sizeof(MaterialConstants));
	UINT particleCBbyteSize = D3DUtil::CalculateConstantBufferByteSize(sizeof(ParticleConstants));

	ID3D12Resource* objectCB = m_currentFrameResource->objectCB->Resource();
	ID3D12Resource* materialCB = m_currentFrameResource->materialCB->Resource();
	ID3D12Resource* particleCB = m_currentFrameResource->particleCB->Resource();

	// 각 렌더 아이템 그리기
	for (size_t i = 0; i < renderItems.size(); ++i)
	{
		RenderItem* renderItem = renderItems[i];

		D3D12_VERTEX_BUFFER_VIEW vbvs[] = { renderItem->particle->VertexBufferView() };
		commandList->IASetVertexBuffers(0, _countof(vbvs), vbvs);
		commandList->IASetIndexBuffer(&renderItem->particle->IndexBufferView());
		commandList->IASetPrimitiveTopology(renderItem->primitiveTopology);

		// 텍스처 서술자 핸들
		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(m_srvHeap->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(renderItem->material->diffuseSrvHeapIndex, m_cbvSrvDescriptorSize);

		// 현재 프레임 리소스에 대한 이 오브젝트를 위한 상수 버퍼 가상 주소 계산
		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress();
		objCBAddress += renderItem->objectCBIndex * objectCBbyteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = materialCB->GetGPUVirtualAddress();
		matCBAddress += renderItem->material->cbIndex * matrialCBbyteSize;
		D3D12_GPU_VIRTUAL_ADDRESS particleCBAddress = particleCB->GetGPUVirtualAddress();
		particleCBAddress += renderItem->particle->cbIndex * particleCBbyteSize;

		commandList->SetGraphicsRootDescriptorTable(0, tex);
		commandList->SetGraphicsRootConstantBufferView(1, objCBAddress);
		commandList->SetGraphicsRootConstantBufferView(3, matCBAddress);
		commandList->SetGraphicsRootConstantBufferView(4, particleCBAddress);

		commandList->DrawIndexedInstanced(
			renderItem->indexCount, 1,
			renderItem->startIndexLocation,
			renderItem->baseVertexLocation,
			0);
	}
}

void LandAndWavesApp::UpdateWaves(const GameTimer& gt)
{
	// 4분의 1초마다 무작위로 파도 생성
	static float timeBase = 0.0f;
	if (m_timer.TotalTime() - timeBase >= 0.25f)
	{
		timeBase += 0.25f;

		int i = MathHelper::Rand(4, m_waves->RowCount() - 5);
		int j = MathHelper::Rand(4, m_waves->ColumnCount() - 5);

		float r = MathHelper::RandF(0.2f, 0.5f);

		m_waves->Disturb(i, j, r);
	}

	// 파도 시뮬레이션 갱신
	m_waves->Update(gt.DeltaTime());

	// 새 정점들로 파도 정점 버퍼 갱신
	auto currentWavesBaseVB = m_currentFrameResource->wavesBaseVB.get();
	auto currentWavesLightingVB = m_currentFrameResource->wavesLightingVB.get();
	for (int i = 0; i < m_waves->VertexCount(); ++i)
	{
		VertexBaseData b;
		VertexLightingData l;

		b.pos = m_waves->Position(i);

		// 위치로 부터 텍스처 좌표 계산 [-w/2,w/2] --> [0,1]
		b.uv.x = 0.5f + b.pos.x / m_waves->Width();
		b.uv.y = 0.5f - b.pos.z / m_waves->Depth();

		l.normal = m_waves->Normal(i);

		currentWavesBaseVB->CopyData(i, b);
		currentWavesLightingVB->CopyData(i, l);
	}

	// 파도 렌더 항목의 동적 VB를 현재 프레임의 VB로 설정
	m_wavesRenderItem->geometry->vertexBuffers[0].gpu = currentWavesBaseVB->Resource();
	m_wavesRenderItem->geometry->vertexBuffers[1].gpu = currentWavesLightingVB->Resource();
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

void LandAndWavesApp::UpdateMaterialCBs(const GameTimer& gt)
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
			XMStoreFloat4x4(&matConstants.matTransform, XMMatrixTranspose(matTransform));
			matConstants.diffuseAlbedo = mat->diffuseAlbedo;
			matConstants.fresnelR0 = mat->fresnelR0;
			matConstants.roughness = mat->roughness;

			currentMaterialCB->CopyData(mat->cbIndex, matConstants);

			// 다음 프레임 자원으로 넘어감
			mat->numFramesDirty--;
		}
	}
}

void LandAndWavesApp::UpdateParticleCBs(const GameTimer& gt)
{
	auto currentParticleCB = m_currentFrameResource->particleCB.get();

	for (auto& e : m_particles)
	{
		// 상수들이 바꾸었을 때에만 cbuffer 자료 갱신
		Particle* particle = e.second.get();
		if (particle->numFramesDirty > 0)
		{
			ParticleConstants particleConstants;
			particleConstants.dragCoefficient = particle->dragCoefficient;
			particleConstants.startTime = particle->startTime;
			particleConstants.lifeTime = particle->lifeTime;

			currentParticleCB->CopyData(particle->cbIndex, particleConstants);

			// 다음 프레임 자원으로 넘어감
			particle->numFramesDirty--;
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
	m_mainPassCB.ambientLight = XMFLOAT4(0.01f, 0.01f, 0.01f, 1.0f);

	m_mainPassCB.gFogColor = XMFLOAT4(Colors::LightSteelBlue);
	m_mainPassCB.gFogStart = 50.0f;
	m_mainPassCB.gFogRange = 500.0f;

	XMVECTOR lightDirection = -MathHelper::SphericalToCartesian(1.0f, m_sunTheta, m_sunPhi);
	XMStoreFloat3(&m_mainPassCB.lights[0].direction, lightDirection);
	m_mainPassCB.lights[0].strength = XMFLOAT3(0.8f, 0.8f, 0.7f);

	UploadBuffer<PassConstants>* currentPassCB = m_currentFrameResource->passCB.get();
	currentPassCB->CopyData(0, m_mainPassCB);
}

void LandAndWavesApp::AnimateMaterials(const GameTimer& gt)
{
	// 물 재질 텍스처 좌표를 스크롤함
	auto waterMat = m_materials["water"].get();

	float& tu = waterMat->matTransform(3, 0);
	float& tv = waterMat->matTransform(3, 1);

	tu += 0.05f * gt.DeltaTime();
	tv += 0.01f * gt.DeltaTime();

	if (tu >= 1.0f) tu -= 1.0f;
	if (tv >= 1.0f) tv -= 1.0f;

	waterMat->matTransform(3, 0) = tu;
	waterMat->matTransform(3, 1) = tv;

	// 상수 버퍼 갱신
	waterMat->numFramesDirty = NUM_FRAME_RESOURCES;
}

void LandAndWavesApp::BuildLandGeometry()
{
	GeometryGenerator geoGenerator;
	GeometryGenerator::MeshData grid = geoGenerator.CreateGrid(160.0f, 160.0f, 50, 50);
	GeometryGenerator::MeshData box = geoGenerator.CreateBox(10.0f, 10.0f, 10.0f, 0);
	GeometryGenerator::MeshData sphere = geoGenerator.CreateGeosphere(2.5f, 0);

	// 필요한 정점 성분들을 추출해서 각 정점에 높이 함수 적용
	size_t verticesSize = grid.vertices.size() + box.vertices.size() + sphere.vertices.size();
	std::vector<VertexBaseData> baseDatas(verticesSize);
	std::vector<VertexLightingData> lightingDatas(verticesSize);
	int k = 0;
	for (size_t i = 0; i < grid.vertices.size(); ++i, ++k)
	{
		XMFLOAT3& p = grid.vertices[i].position;
		baseDatas[k].pos = p;
		baseDatas[k].pos.y = GetHillsHeight(p.x, p.z);
		baseDatas[k].uv = grid.vertices[i].texCoord;
		lightingDatas[k].normal = GetHillsNormal(p.x, p.z);
	}
	for (size_t i = 0; i < box.vertices.size(); ++i, ++k)
	{
		baseDatas[k].pos = box.vertices[i].position;
		baseDatas[k].uv = box.vertices[i].texCoord;
		lightingDatas[k].normal = box.vertices[i].normal;
	}
	for (size_t i = 0; i < sphere.vertices.size(); ++i, ++k)
	{
		baseDatas[k].pos = sphere.vertices[i].position;
		baseDatas[k].uv = sphere.vertices[i].texCoord;
		lightingDatas[k].normal = sphere.vertices[i].normal;
	}

	const UINT vbByteSizes[] = { (UINT)baseDatas.size() * sizeof(VertexBaseData), (UINT)lightingDatas.size() * sizeof(VertexLightingData) };

	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), grid.GetIndices16().begin(), grid.GetIndices16().end());
	indices.insert(indices.end(), box.GetIndices16().begin(), box.GetIndices16().end());
	indices.insert(indices.end(), sphere.GetIndices16().begin(), sphere.GetIndices16().end());

	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->name = "land_geometry";

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


	SubmeshGeometry girdSubmesh;
	girdSubmesh.indexCount = (UINT)grid.indices32.size();
	UINT startIndexLocation = 0;
	UINT baseVertexLocation = 0;
	girdSubmesh.startIndexLocation = startIndexLocation;
	girdSubmesh.baseVertexLocation = baseVertexLocation;

	SubmeshGeometry boxSubmesh;
	boxSubmesh.indexCount = (UINT)box.indices32.size();
	startIndexLocation += (UINT)grid.indices32.size();
	baseVertexLocation += (UINT)grid.vertices.size();
	boxSubmesh.startIndexLocation = startIndexLocation;
	boxSubmesh.baseVertexLocation = baseVertexLocation;

	SubmeshGeometry sphereSubmesh;
	sphereSubmesh.indexCount = (UINT)sphere.indices32.size();
	startIndexLocation += (UINT)box.indices32.size();
	baseVertexLocation += (UINT)box.vertices.size();
	sphereSubmesh.startIndexLocation = startIndexLocation;
	sphereSubmesh.baseVertexLocation = baseVertexLocation;

	geo->drawArgs["grid"] = girdSubmesh;
	geo->drawArgs["box"] = boxSubmesh;
	geo->drawArgs["sphere"] = sphereSubmesh;

	m_geometries[geo->name] = std::move(geo);
}

void LandAndWavesApp::BuildWavesGeometry()
{
	std::vector<std::uint16_t> indices(3 * m_waves->TriangleCount());
	assert(m_waves->VertexCount() < 0x0000ffff);

	// 각 쿼드 별로 반복
	int m = m_waves->RowCount();
	int n = m_waves->ColumnCount();
	int k = 0;
	for (int i = 0; i < m - 1; ++i)
	{
		for (int j = 0; j < n - 1; ++j)
		{
			indices[k] = i * n + j;
			indices[k + 1] = i * n + j + 1;
			indices[k + 2] = (i + 1) * n + j;

			indices[k + 3] = (i + 1) * n + j;
			indices[k + 4] = i * n + j + 1;
			indices[k + 5] = (i + 1) * n + j + 1;

			k += 6; // 다음 쿼드
		}
	}

	UINT vbByteSizes[] = { m_waves->VertexCount() * sizeof(VertexBaseData), m_waves->VertexCount() * sizeof(VertexLightingData) };
	UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->name = "water_geometry";

	// 동적으로 설정
	geo->vertexBuffers[0].cpu = nullptr;
	geo->vertexBuffers[0].gpu = nullptr;
	geo->vertexBuffers[1].cpu = nullptr;
	geo->vertexBuffers[1].gpu = nullptr;

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->indexBuffer.cpu));
	CopyMemory(geo->indexBuffer.cpu->GetBufferPointer(), indices.data(), ibByteSize);

	geo->indexBuffer.gpu = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		m_commandList.Get(), indices.data(), ibByteSize, geo->indexBuffer.uploader);

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

	geo->drawArgs["grid"] = submesh;

	m_geometries[geo->name] = std::move(geo);
}

void LandAndWavesApp::BuildTreeGeometry()
{
	// 필요한 정점 성분들을 추출해서 각 정점에 높이 함수 적용
	size_t numTree = 20;

	std::vector<TreeSpriteVertex> vertices(numTree);	
	for (size_t i = 0; i < numTree; ++i)
	{
		float x = MathHelper::RandF(-45.0f, 45.0f);
		float z = MathHelper::RandF(-45.0f, 45.0f);
		float y = GetHillsHeight(x, z);

		y += 8.0f;

		vertices[i].pos = XMFLOAT3(x, y, z);
		vertices[i].size = XMFLOAT2(20.0f, 20.0f);
	}
	std::vector<std::uint16_t> indices(numTree);
	for (int i = 0; i < numTree; ++i)
	{
		indices[i] = i;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(TreeSpriteVertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->name = "tree_geometry";
	geo->vertexBufferCount = 1;

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->vertexBuffers[0].cpu));
	CopyMemory(geo->vertexBuffers[0].cpu->GetBufferPointer(), vertices.data(), vbByteSize);
	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->indexBuffer.cpu));
	CopyMemory(geo->indexBuffer.cpu->GetBufferPointer(), indices.data(), ibByteSize);

	geo->vertexBuffers[0].gpu = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(), m_commandList.Get(), vertices.data(), vbByteSize, geo->vertexBuffers[0].uploader);
	geo->indexBuffer.gpu = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(), m_commandList.Get(), indices.data(), ibByteSize, geo->indexBuffer.uploader);

	geo->vertexBuffers[0].byteStride = sizeof(TreeSpriteVertex);
	geo->vertexBuffers[0].byteSize = vbByteSize;
	geo->indexBuffer.format = DXGI_FORMAT_R16_UINT;
	geo->indexBuffer.byteSize = ibByteSize;

	SubmeshGeometry treeSubmesh;
	treeSubmesh.indexCount = (UINT)indices.size();
	treeSubmesh.startIndexLocation = 0;
	treeSubmesh.baseVertexLocation = 0;

	geo->drawArgs["points"] = treeSubmesh;

	m_geometries[geo->name] = std::move(geo);
}

void LandAndWavesApp::BuildParticle()
{
	// 분수대
	int numParticle = 1000;
	int numStem = 20;

	std::vector<ParticleVertex> vertices(numParticle);

	int k = 0;

	for (int i = 0; i < numParticle / numStem; ++i)
	{
		for (float angle = 0.f; angle < 360.0f; angle += (360.0f / (float)numStem), ++k)
		{
			vertices[k].startPos = XMFLOAT3(5.0f * cosf(XMConvertToRadians(angle)), 50.0f, 5.0f * sinf(XMConvertToRadians(angle)));
			vertices[k].size = XMFLOAT2(2.5f, 2.5f);
			vertices[k].selfLightColor = XMFLOAT3(0.0f, 0.0f, 0.0f);
			vertices[k].startVelocity = XMFLOAT3(10.0f * cosf(XMConvertToRadians(angle)), 10.0f, 10.0f * sinf(XMConvertToRadians(angle)));
			vertices[k].timeDelay = i * 0.2f;
		}
	}
	CreateParticle(vertices, "fountain", 0, 0.0f, m_timer.TotalTime(), 5.0f);
}

void LandAndWavesApp::BuildRenderItems()
{
	UINT objCBIndex = 0;

	XMMATRIX wavesWorld = XMMatrixIdentity();
	XMMATRIX wavesTexTransform = XMMatrixIdentity();
	std::unique_ptr<RenderItem> wavesRenderItem
		= CreateRenderItem(wavesWorld, wavesTexTransform, objCBIndex++, 
			"water_geometry", "grid", "water", D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, RenderLayer::Transparent);
	m_wavesRenderItem = wavesRenderItem.get();
	m_allRenderItems.push_back(std::move(wavesRenderItem));
	
	XMMATRIX gridWorld = XMMatrixIdentity();
	XMMATRIX gridTexTransform = XMMatrixScaling(5.0f, 5.0f, 1.0f);
	std::unique_ptr<RenderItem> gridRenderItem
		= CreateRenderItem(gridWorld, gridTexTransform, objCBIndex++, 
			"land_geometry", "grid", "grass", D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, RenderLayer::Opaque);
	m_allRenderItems.push_back(std::move(gridRenderItem));
	std::unique_ptr<RenderItem> gridRenderItemForDebugNormal
		= CreateRenderItem(gridWorld, gridTexTransform, objCBIndex++, 
			"land_geometry", "grid", "grass", D3D_PRIMITIVE_TOPOLOGY_POINTLIST, RenderLayer::DebugNormal);
	m_allRenderItems.push_back(std::move(gridRenderItemForDebugNormal));
	std::unique_ptr<RenderItem> gridRenderItemForDebugSurfaceNormal
		= CreateRenderItem(gridWorld, gridTexTransform, objCBIndex++, 
			"land_geometry", "grid", "grass", D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, RenderLayer::DebugSurfaceNormal);
	m_allRenderItems.push_back(std::move(gridRenderItemForDebugSurfaceNormal));

	//for (float x = -50.0f; x < 51.0f; x += 5.0f)
	//{
		//for (float z = -50.0f; z < 51.0f; z += 5.0f)
		//{
			XMMATRIX sphereWorld = XMMatrixTranslation(0.0f, 5.0f, 0.0f);
			XMMATRIX  sphereTexTransform = XMMatrixIdentity();
			std::unique_ptr<RenderItem> sphereRenderItem
				= CreateRenderItem(sphereWorld, sphereTexTransform, objCBIndex++, 
					"land_geometry", "sphere", "white", D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, RenderLayer::TriangleExplosion);
			m_allRenderItems.push_back(std::move(sphereRenderItem));
		//}
	//}

	/*
	XMMATRIX treeWorld = XMMatrixIdentity();
	XMMATRIX treeTexTransform = XMMatrixIdentity();
	std::unique_ptr<RenderItem> treeRenderItem
		= CreateRenderItem(treeWorld, treeTexTransform, objCBIndex++, 
		"tree_geometry", "points", "tree", D3D_PRIMITIVE_TOPOLOGY_POINTLIST, RenderLayer::TreeSprite);
	m_allRenderItems.push_back(std::move(treeRenderItem));

	// 파티클
	XMMATRIX fountainWorld = XMMatrixIdentity();
	XMMATRIX fountainTexTransform = XMMatrixIdentity();
	std::unique_ptr<RenderItem> fountainRenderItem
		= CreateRenderItemForParticle(fountainWorld, fountainTexTransform, objCBIndex++, 
		"fountain", "water", D3D_PRIMITIVE_TOPOLOGY_POINTLIST, RenderLayer::ParticleInfinity);
	m_allRenderItems.push_back(std::move(fountainRenderItem));
	*/
}

void LandAndWavesApp::BuildFrameResources()
{
	for (int i = 0; i < NUM_FRAME_RESOURCES; ++i)
	{
		m_frameResources.push_back(std::make_unique<LandAndWavesFrameResource>(m_d3dDevice.Get(), 1, 
			(UINT)m_allRenderItems.size(), (UINT)m_materials.size(), (UINT)m_particles.size(), m_waves->VertexCount()));
	}
}

void LandAndWavesApp::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE descRange;
	descRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_ROOT_PARAMETER slotRootParameter[5];
	// 루트 CBV 생성
	slotRootParameter[0].InitAsDescriptorTable(1, &descRange, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsConstantBufferView(0);
	slotRootParameter[2].InitAsConstantBufferView(1);
	slotRootParameter[3].InitAsConstantBufferView(2);
	slotRootParameter[4].InitAsConstantBufferView(3);

	auto staticSamplers = D3DUtil::GetStaticsSamplers();

	// 루트 시그니쳐는 루트 매개변수들의 배열
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(
		5, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

void LandAndWavesApp::BuildShadersAndInputLayout()
{
	m_shaders["standard_vs"] = D3DUtil::CompileShader(L"../build_shader/standard_texture.hlsl", nullptr, "VS", "vs_5_1");
	m_shaders["standard_ps"] = D3DUtil::CompileShader(L"../build_shader/standard_texture.hlsl", nullptr, "PS", "ps_5_1");
	m_shaders["alpha_test_ps"] = D3DUtil::CompileShader(L"../build_shader/standard_texture.hlsl", nullptr, "AlphaTestedPS", "ps_5_1");

	m_shaders["tree_sprite_vs"] = D3DUtil::CompileShader(L"../build_shader/tree_sprite.hlsl", nullptr, "VS", "vs_5_1");
	m_shaders["tree_sprite_gs"] = D3DUtil::CompileShader(L"../build_shader/tree_sprite.hlsl", nullptr, "GS", "gs_5_1");
	m_shaders["tree_sprite_ps"] = D3DUtil::CompileShader(L"../build_shader/tree_sprite.hlsl", nullptr, "PS", "ps_5_1");

	m_shaders["gravity_particle_vs"] = D3DUtil::CompileShader(L"../build_shader/gravity_particle.hlsl", nullptr, "VS", "vs_5_1");
	m_shaders["gravity_particle_infinity_vs"] = D3DUtil::CompileShader(L"../build_shader/gravity_particle.hlsl", nullptr, "VSInfinity", "vs_5_1");
	m_shaders["gravity_particle_gs"] = D3DUtil::CompileShader(L"../build_shader/gravity_particle.hlsl", nullptr, "GS", "gs_5_1");
	m_shaders["gravity_particle_ps"] = D3DUtil::CompileShader(L"../build_shader/gravity_particle.hlsl", nullptr, "PS", "ps_5_1");

	m_shaders["lod_sphere_vs"] = D3DUtil::CompileShader(L"../build_shader/lod_sphere.hlsl", nullptr, "VS", "vs_5_1");
	m_shaders["lod_sphere_gs"] = D3DUtil::CompileShader(L"../build_shader/lod_sphere.hlsl", nullptr, "GS", "gs_5_1");
	m_shaders["lod_sphere_ps"] = D3DUtil::CompileShader(L"../build_shader/lod_sphere.hlsl", nullptr, "PS", "ps_5_1");

	m_shaders["triangle_explosion_vs"] = D3DUtil::CompileShader(L"../build_shader/triangle_explosion.hlsl", nullptr, "VS", "vs_5_1");
	m_shaders["triangle_explosion_gs"] = D3DUtil::CompileShader(L"../build_shader/triangle_explosion.hlsl", nullptr, "GS", "gs_5_1");
	m_shaders["triangle_explosion_ps"] = D3DUtil::CompileShader(L"../build_shader/triangle_explosion.hlsl", nullptr, "PS", "ps_5_1");

	m_shaders["debug_normal_vs"] = D3DUtil::CompileShader(L"../build_shader/debug_normal.hlsl", nullptr, "VS", "vs_5_1");
	m_shaders["debug_normal_gs"] = D3DUtil::CompileShader(L"../build_shader/debug_normal.hlsl", nullptr, "GS", "gs_5_1");
	m_shaders["debug_surface_normal_gs"] = D3DUtil::CompileShader(L"../build_shader/debug_normal.hlsl", nullptr, "GSSurface", "gs_5_1");
	m_shaders["debug_normal_ps"] = D3DUtil::CompileShader(L"../build_shader/debug_normal.hlsl", nullptr, "PS", "ps_5_1");

	m_inputLayouts["standard"] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	m_inputLayouts["tree_sprite"] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	m_inputLayouts["gravity_particle"] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "DELAY", 0, DXGI_FORMAT_R32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void LandAndWavesApp::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { m_inputLayouts["standard"].data(), (UINT)m_inputLayouts["standard"].size() };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_shaders["standard_vs"]->GetBufferPointer()),
		m_shaders["standard_vs"]->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_shaders["standard_ps"]->GetBufferPointer()),
		m_shaders["standard_ps"]->GetBufferSize()
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

	// 혼합용
	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentPsoDesc = psoDesc;

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

	// 알파 테스트용
	transparentPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_shaders["alpha_test_ps"]->GetBufferPointer()),
		m_shaders["alpha_test_ps"]->GetBufferSize()
	};
	transparentPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&transparentPsoDesc, IID_PPV_ARGS(&m_psos["alpha_test"])));

	// 나무 스프라이트
	D3D12_GRAPHICS_PIPELINE_STATE_DESC treeSpritePsoDesc = psoDesc;

	treeSpritePsoDesc.InputLayout = { m_inputLayouts["tree_sprite"].data(), (UINT)m_inputLayouts["tree_sprite"].size() };
	treeSpritePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	treeSpritePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_shaders["tree_sprite_vs"]->GetBufferPointer()),
		m_shaders["tree_sprite_vs"]->GetBufferSize()
	};
	treeSpritePsoDesc.GS =
	{
		reinterpret_cast<BYTE*>(m_shaders["tree_sprite_gs"]->GetBufferPointer()),
		m_shaders["tree_sprite_gs"]->GetBufferSize()
	};
	treeSpritePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_shaders["tree_sprite_ps"]->GetBufferPointer()),
		m_shaders["tree_sprite_ps"]->GetBufferSize()
	};
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&treeSpritePsoDesc, IID_PPV_ARGS(&m_psos["tree_sprite"])));

	// 중력을 받는 파티클
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gravityParticlePsoDesc = transparentPsoDesc;

	gravityParticlePsoDesc.InputLayout = { m_inputLayouts["gravity_particle"].data(), (UINT)m_inputLayouts["gravity_particle"].size() };
	gravityParticlePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	gravityParticlePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_shaders["gravity_particle_vs"]->GetBufferPointer()),
		m_shaders["gravity_particle_vs"]->GetBufferSize()
	};
	gravityParticlePsoDesc.GS =
	{
		reinterpret_cast<BYTE*>(m_shaders["gravity_particle_gs"]->GetBufferPointer()),
		m_shaders["gravity_particle_gs"]->GetBufferSize()
	};
	gravityParticlePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_shaders["gravity_particle_ps"]->GetBufferPointer()),
		m_shaders["gravity_particle_ps"]->GetBufferSize()
	};
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&gravityParticlePsoDesc, IID_PPV_ARGS(&m_psos["gravity_particle"])));
	gravityParticlePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_shaders["gravity_particle_infinity_vs"]->GetBufferPointer()),
		m_shaders["gravity_particle_infinity_vs"]->GetBufferSize()
	};
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&gravityParticlePsoDesc, IID_PPV_ARGS(&m_psos["gravity_particle_infinity"])));

	// LOD 구
	D3D12_GRAPHICS_PIPELINE_STATE_DESC lodSpherePsoDesc = psoDesc;
	lodSpherePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_shaders["lod_sphere_vs"]->GetBufferPointer()),
		m_shaders["lod_sphere_vs"]->GetBufferSize()
	};
	lodSpherePsoDesc.GS =
	{
		reinterpret_cast<BYTE*>(m_shaders["lod_sphere_gs"]->GetBufferPointer()),
		m_shaders["lod_sphere_gs"]->GetBufferSize()
	};
	lodSpherePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_shaders["lod_sphere_ps"]->GetBufferPointer()),
		m_shaders["lod_sphere_ps"]->GetBufferSize()
	};
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&lodSpherePsoDesc, IID_PPV_ARGS(&m_psos["lod_sphere"])));

	// 삼각형 폭발
	D3D12_GRAPHICS_PIPELINE_STATE_DESC triangleExplosionPsoDesc = psoDesc;
	triangleExplosionPsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_shaders["triangle_explosion_vs"]->GetBufferPointer()),
		m_shaders["triangle_explosion_vs"]->GetBufferSize()
	};
	triangleExplosionPsoDesc.GS =
	{
		reinterpret_cast<BYTE*>(m_shaders["triangle_explosion_gs"]->GetBufferPointer()),
		m_shaders["triangle_explosion_gs"]->GetBufferSize()
	};
	triangleExplosionPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_shaders["triangle_explosion_ps"]->GetBufferPointer()),
		m_shaders["triangle_explosion_ps"]->GetBufferSize()
	};
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&triangleExplosionPsoDesc, IID_PPV_ARGS(&m_psos["triangle_explosion"])));

	// 법선 디버깅
	D3D12_GRAPHICS_PIPELINE_STATE_DESC debugNoramlPsoDesc = psoDesc;

	debugNoramlPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	debugNoramlPsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_shaders["debug_normal_vs"]->GetBufferPointer()),
		m_shaders["debug_normal_vs"]->GetBufferSize()
	};
	debugNoramlPsoDesc.GS =
	{
		reinterpret_cast<BYTE*>(m_shaders["debug_normal_gs"]->GetBufferPointer()),
		m_shaders["debug_normal_gs"]->GetBufferSize()
	}; 
	debugNoramlPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_shaders["debug_normal_ps"]->GetBufferPointer()),
		m_shaders["debug_normal_ps"]->GetBufferSize()
	};
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&debugNoramlPsoDesc, IID_PPV_ARGS(&m_psos["debug_normal"])));

	debugNoramlPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	debugNoramlPsoDesc.GS =
	{
		reinterpret_cast<BYTE*>(m_shaders["debug_surface_normal_gs"]->GetBufferPointer()),
		m_shaders["debug_surface_normal_gs"]->GetBufferSize()
	};
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&debugNoramlPsoDesc, IID_PPV_ARGS(&m_psos["debug_surface_normal"])));
}

void LandAndWavesApp::BuildTexture()
{
	m_textures["grass"] = D3DUtil::CreateTextureFromDDSFile("grass", L"./resource/grass.dds", m_d3dDevice.Get(), m_commandList.Get());
	m_textures["water"] = D3DUtil::CreateTextureFromDDSFile("water", L"./resource/water1.dds", m_d3dDevice.Get(), m_commandList.Get());
	m_textures["wire_fence"] = D3DUtil::CreateTextureFromDDSFile("wire_fence", L"./resource/WireFence.dds", m_d3dDevice.Get(), m_commandList.Get());
	m_textures["tree_sprite"] = D3DUtil::CreateTextureFromDDSFile("tree_sprite", L"./resource/treeArray2.dds", m_d3dDevice.Get(), m_commandList.Get());
	m_textures["white"] = D3DUtil::CreateTextureFromDDSFile("tree_sprite", L"./resource/white1x1.dds", m_d3dDevice.Get(), m_commandList.Get());
}

void LandAndWavesApp::BuildMaterialAndSrv()
{
	// 머터리얼 생성
	m_materials["grass"] = std::make_unique<Material>("grass", 0,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(0.01f, 0.01f, 0.01f), 0.125f);
	m_materials["water"] = std::make_unique<Material>("water", 1,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f), XMFLOAT3(0.1f, 0.1f, 0.1f), 0.0f);
	m_materials["wire"] = std::make_unique<Material>("wire", 2,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(0.01f, 0.01f, 0.01f), 0.01f);
	m_materials["tree"] = std::make_unique<Material>("tree", 3,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(0.1f, 0.1f, 0.1f), 0.01f);
	m_materials["white"] = std::make_unique<Material>("white", 3,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(0.1f, 0.1f, 0.1f), 0.01f);

	// 서술자 힙 생성
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = (UINT)m_textures.size();
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));

	// srv 생성
	CreateSRVForTexture(0, m_textures["grass"]->resource.Get(), "grass");
	CreateSRVForTexture(1, m_textures["water"]->resource.Get(), "water");
	CreateSRVForTexture(2, m_textures["wire_fence"]->resource.Get(), "wire");
	CreateSRVForTexture(3, m_textures["tree_sprite"]->resource.Get(), "tree");
	CreateSRVForTexture(4, m_textures["white"]->resource.Get(), "white");
}

float LandAndWavesApp::GetHillsHeight(float x, float z) const
{
	return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

XMFLOAT3 LandAndWavesApp::GetHillsNormal(float x, float z) const
{
	// n = (-df/dx, 1 -df/dz)
	XMFLOAT3 n(
		-0.03f * z * cosf(0.1f * x) - 0.3f * cosf(0.1f * z),
		1.0f,
		-0.3f * sinf(0.1f * x) + 0.03f * x * sinf(0.1f * z));

	XMVECTOR uintNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, uintNormal);

	return n;
}

void LandAndWavesApp::CreateParticle(std::vector<ParticleVertex>& vertices, const char* name, UINT index, float dragCoefficient, float startTime, float lifeTime)
{
	std::vector<std::uint16_t> indices(vertices.size());
	for (int i = 0; i < vertices.size(); ++i)
	{
		indices[i] = i;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(ParticleVertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto particle = std::make_unique<Particle>();
	particle->name = name;

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &particle->vertexBuffer.cpu));
	CopyMemory(particle->vertexBuffer.cpu->GetBufferPointer(), vertices.data(), vbByteSize);
	ThrowIfFailed(D3DCreateBlob(ibByteSize, &particle->indexBuffer.cpu));
	CopyMemory(particle->indexBuffer.cpu->GetBufferPointer(), indices.data(), ibByteSize);

	particle->vertexBuffer.gpu = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(), m_commandList.Get(), vertices.data(), vbByteSize, particle->vertexBuffer.uploader);
	particle->indexBuffer.gpu = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(), m_commandList.Get(), indices.data(), ibByteSize, particle->indexBuffer.uploader);

	particle->vertexBuffer.byteStride = sizeof(ParticleVertex);
	particle->vertexBuffer.byteSize = vbByteSize;
	particle->indexBuffer.format = DXGI_FORMAT_R16_UINT;
	particle->indexBuffer.byteSize = ibByteSize;

	particle->indexCount = (UINT)indices.size();
	particle->cbIndex = index;

	if (dragCoefficient < 0.01f)
	{
		dragCoefficient = 0.01f;
	}
	particle->dragCoefficient = dragCoefficient;
	particle->startTime = startTime;
	particle->lifeTime = lifeTime;

	m_particles[particle->name] = std::move(particle);
}

std::unique_ptr<RenderItem> LandAndWavesApp::CreateRenderItem(const XMMATRIX& world, const XMMATRIX& texTransform, UINT objectCBIndex,
	const char* geometry, const char* submesh, const char* material, D3D_PRIMITIVE_TOPOLOGY primitiveTopology, RenderLayer layer)
{
	std::unique_ptr<RenderItem> renderItem = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&renderItem->world, world);
	XMStoreFloat4x4(&renderItem->texTransform, texTransform);
	renderItem->objectCBIndex = objectCBIndex;
	renderItem->geometry = m_geometries[geometry].get();
	renderItem->material = m_materials[material].get();
	renderItem->primitiveTopology = primitiveTopology;
	renderItem->indexCount = renderItem->geometry->drawArgs[submesh].indexCount;
	renderItem->startIndexLocation = renderItem->geometry->drawArgs[submesh].startIndexLocation;
	renderItem->baseVertexLocation = renderItem->geometry->drawArgs[submesh].baseVertexLocation;

	m_renderItemsEachRenderLayers[layer].push_back(renderItem.get());

	return renderItem;
}

std::unique_ptr<RenderItem> LandAndWavesApp::CreateRenderItemForParticle(const XMMATRIX& world, const XMMATRIX& texTransform, UINT objectCBIndex, 
	const char* particle, const char* material, D3D_PRIMITIVE_TOPOLOGY primitiveTopology, RenderLayer layer)
{
	std::unique_ptr<RenderItem> renderItem = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&renderItem->world, world);
	XMStoreFloat4x4(&renderItem->texTransform, texTransform);
	renderItem->objectCBIndex = objectCBIndex;
	renderItem->material = m_materials[material].get();
	renderItem->particle = m_particles[particle].get();
	renderItem->primitiveTopology = primitiveTopology;
	renderItem->indexCount = m_particles[particle]->indexCount;
	renderItem->startIndexLocation = 0;
	renderItem->baseVertexLocation = 0;

	m_renderItemsEachRenderLayers[layer].push_back(renderItem.get());

	return renderItem;
}

void LandAndWavesApp::CreateSRVForTexture(UINT index, ID3D12Resource* textureResource, const char* materialName)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_srvHeap->GetCPUDescriptorHandleForHeapStart());
	hDescriptor.Offset(index, m_cbvSrvDescriptorSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureResource->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = textureResource->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	m_d3dDevice->CreateShaderResourceView(textureResource, &srvDesc, hDescriptor);

	m_materials[materialName].get()->diffuseSrvHeapIndex = index;
}
