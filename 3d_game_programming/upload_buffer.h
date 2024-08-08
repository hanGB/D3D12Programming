#pragma once
#include "stdafx.h"
#include "d3d_util.h"

template<typename T>
class UploadBuffer
{
public:
	UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer)
		: m_isConstantBuffer(isConstantBuffer)
	{
		m_elementByteSize = sizeof(T);

		// 상수 버퍼 원소의 크기는 반드시 256바이트의 배수여야 함
		if (isConstantBuffer)
		{
			m_elementByteSize = D3DUtil::CalculateConstantBufferByteSize(sizeof(T));
		}

		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_elementByteSize * elementCount),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_uploadBuffer)));

		ThrowIfFailed(m_uploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedData)));

		// 리소스를 다 사용하기 전에는 해제할 필요없음
	}

	UploadBuffer(const UploadBuffer& rhs) = delete;
	UploadBuffer& operator=(const UploadBuffer& rhs) = delete;
	~UploadBuffer()
	{
		if (!m_uploadBuffer)
		{
			m_uploadBuffer->Unmap(0, nullptr);
		}
		m_mappedData = nullptr;
	}

	ID3D12Resource& Resource() const
	{
		return m_uploadBuffer.Get();
	}
	void CopyData(int elementIndex, const T& data)
	{
		memcpy(&m_mappedData[elementIndex * m_elementByteSize], &data, sizeof(T));
	}

private:
	ComPtr<ID3D12RTesource> m_uploadBuffer;
	BYTE* m_mappedData = nullptr;

	UINT m_elementByteSize = 0;
	bool m_isConstantBuffer = false;
};
