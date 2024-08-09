#pragma once

inline std::wstring AnsiToWString(const std::string& str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}

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

// 부분 메시
struct SubmeshGeometry
{
	UINT indexCount = 0;
	UINT startIndexLocation = 0;
	INT  baseVertexLocation = 0;

	DirectX::BoundingBox boundingBox;
};

struct MeshGeometry
{
	std::string name;

	// 시스템 메모리 복사본
	ComPtr<ID3DBlob> vertexBufferCPU = nullptr;
	ComPtr<ID3DBlob> indexBufferCPU = nullptr;

	ComPtr<ID3D12Resource> vertexBufferGPU = nullptr;
	ComPtr<ID3D12Resource> indexBufferGPU = nullptr;

	ComPtr<ID3D12Resource> vertexBufferUploader = nullptr;
	ComPtr<ID3D12Resource> indexBufferUploader = nullptr;

	// 버퍼 관련 자료
	UINT vertexByteStride = 0;
	UINT vertexBufferByteSize = 0;
	DXGI_FORMAT indexFormat = DXGI_FORMAT_R16_UINT;
	UINT indexBufferByteSize = 0;

	// 부분 메시들을 개별적으로 그릴 수 있도록 컨테이너에 보관
	std::unordered_map<std::string, SubmeshGeometry> drawArgs;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation	= vertexBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes	= vertexByteStride;
		vbv.SizeInBytes		= vertexBufferByteSize;
	}

	D3D12_INDEX_BUFFER_VIEW IndexBufferView() const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation	= indexBufferGPU->GetGPUVirtualAddress();
		ibv.Format			= indexFormat;
		ibv.SizeInBytes		= indexBufferByteSize;
	}

	// 리소스를 GPU에 모두 올린 후 메모리 해제 가능
	void DisposeUploaders()
	{
		vertexBufferUploader = nullptr;
		indexBufferUploader = nullptr;
	}
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