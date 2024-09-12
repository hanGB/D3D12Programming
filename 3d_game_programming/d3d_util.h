#pragma once
#include "math_helper.h"

inline std::wstring AnsiToWString(const std::string& str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}

struct Texture
{
	// 텍스처 이름
	std::string name;

	std::wstring filename;

	ComPtr<ID3D12Resource> resource = nullptr;
	ComPtr<ID3D12Resource> uploadHeap = nullptr;
};

class D3DUtil
{
public:
	static UINT CalculateConstantBufferByteSize(UINT byteSize)
	{
		// 상수 버퍼는 최소 하드웨어 할당 크기의 배수여야 함(일반적으로 최소 하드웨어 할당 크기는 256)
	    // 가장 가까운 256의 배수로 만듦(255를 더하고 하위 2비트값을 0으로 처리함)
		return (byteSize + 255) & ~255; // 255는 16비트로 0x00ff
	}

	static ComPtr<ID3DBlob> LoadBinary(const std::wstring& filename);

	static ComPtr<ID3D12Resource> CreateDefaultBuffer(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		const void* initData,
		UINT64 byteSize,
		ComPtr<ID3D12Resource>& uploadBuffer);

	// 정적 샘플러
	static std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticsSamplers();

	// 텍스처 생성
	static std::unique_ptr<Texture> CreateTextureFromDDSFile(const char* name, const wchar_t* filename, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
};

class DxException
{
public:
	DxException() = default;
	DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

	std::wstring ToString() const;

	HRESULT m_errorCode = S_OK;
	std::wstring m_functionName;
	std::wstring m_filename;
	int m_lineNumber = -1;
};

struct VertexBuffer
{
	ComPtr<ID3DBlob> cpu = nullptr;
	ComPtr<ID3D12Resource> gpu = nullptr;
	ComPtr<ID3D12Resource> uploader = nullptr;

	UINT byteStride = 0;
	UINT byteSize = 0;
};
struct IndexBuffer
{
	ComPtr<ID3DBlob> cpu = nullptr;
	ComPtr<ID3D12Resource> gpu = nullptr;
	ComPtr<ID3D12Resource> uploader = nullptr;

	DXGI_FORMAT format = DXGI_FORMAT_R16_UINT;
	UINT byteSize = 0;
};

// 부분 메시
struct SubmeshGeometry
{
	UINT indexCount = 0;
	UINT startIndexLocation = 0;
	INT  baseVertexLocation = 0;

	BoundingBox boundingBox;
};

struct MeshGeometry
{
	std::string name;

	std::array<VertexBuffer, 2> vertexBuffers = { 
		VertexBuffer({ nullptr, nullptr, nullptr, 0, 0 }), 
		VertexBuffer({ nullptr, nullptr, nullptr, 0, 0 }) 
	};
	IndexBuffer indexBuffer = { nullptr, nullptr, nullptr, DXGI_FORMAT_R16_UINT, 0};

	// 부분 메시들을 개별적으로 그릴 수 있도록 컨테이너에 보관
	std::unordered_map<std::string, SubmeshGeometry> drawArgs;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView(int index) const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation	= vertexBuffers[index].gpu->GetGPUVirtualAddress();
		vbv.StrideInBytes	= vertexBuffers[index].byteStride;
		vbv.SizeInBytes		= vertexBuffers[index].byteSize;

		return vbv;
	}

	D3D12_INDEX_BUFFER_VIEW IndexBufferView() const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation	= indexBuffer.gpu->GetGPUVirtualAddress();
		ibv.Format			= indexBuffer.format;
		ibv.SizeInBytes		= indexBuffer.byteSize;

		return ibv;
	}

	// 리소스를 GPU에 모두 올린 후 업로드 버퍼 메모리 해제 가능
	void DisposeUploaders()
	{
		for (int i = 0; i < vertexBuffers.size(); ++i)
		{
			vertexBuffers[i].uploader = nullptr;
		}
		indexBuffer.uploader = nullptr;
	}
};

struct Material
{
	// 재질 이름
	std::string name;

	// 상수 버퍼의 인덱스
	int cbIndex = -1;

	// SRV 힙에서 해당하는 디뷰즈 텍스쳐 인덱스
	int diffuseSrvHeapIndex = -1;

	// 재질이 변해서 하당 상수 버퍼를 갱신해야 하는지 여부
	int numFramesDirty = NUM_FRAME_RESOURCES;

	// 세이딩에 쓰이는 재질 상수 버퍼 자료
	XMFLOAT4 diffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	XMFLOAT3 fresnelR0 = { 0.01f, 0.01f, 0.01f };
	float roughness = 0.25f;
	XMFLOAT4X4 matTransform = MathHelper::Identity4x4();
};

struct Light
{
	XMFLOAT3 strength;
	float falloffStart;
	XMFLOAT3 direction;
	float falloffEnd;
	XMFLOAT3 position;
	float spotPower;	
};

#ifndef ThrowIfFailed
#define ThrowIfFailed(x) \
{ \
	HRESULT hr__ = (x); \
	std::wstring wfn = AnsiToWString(__FILE__); \
	if (FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif // !ThrowIfFailed

#ifndef ReleaseCom
#define ReleaseCom(x) \
{ \
	if (x) { x->Release(); x = 0; } \
}
#endif