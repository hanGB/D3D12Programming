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