#include "stdafx.h"
#include "d3d_util.h"
#include <comdef.h>
#include <fstream>

DxException::DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber) 
	: m_errorCode(hr), m_functionName(functionName), m_filename(filename), m_lineNumber(lineNumber)
{

}

std::wstring DxException::ToString() const
{
    // Get the string description of the error code.
    _com_error err(m_errorCode);
    std::wstring msg = err.ErrorMessage();

    return m_functionName + L" failed in " + m_filename + L"; line " + std::to_wstring(m_lineNumber) + L"; error: " + msg;
}

ComPtr<ID3DBlob> D3DUtil::LoadBinary(const std::wstring& filename)
{
    std::ifstream in(filename, std::ios::binary);

    in.seekg(0, std::ios_base::end);
    std::ifstream::pos_type size = (int)in.tellg();
    in.seekg(0, std::ios_base::beg);

    ComPtr<ID3DBlob> blob;
    ThrowIfFailed(D3DCreateBlob(size, blob.GetAddressOf()));

    in.read((char*)blob->GetBufferPointer(), size);
    in.close();

    return blob;
}

ComPtr<ID3D12Resource> D3DUtil::CreateDefaultBuffer(
    ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const void* initData, UINT64 byteSize, ComPtr<ID3D12Resource>& uploadBuffer)
{
    ComPtr<ID3D12Resource> defaultBuffer;

    // 실제 기본 버퍼 리소스 생성
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(byteSize),
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&defaultBuffer)));

    // CPU 메모리 데이터를 기본 버퍼에 복사하기 위해 새로운 중간 업로드 버퍼 생성
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(byteSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&uploadBuffer)));


    // 기본 버퍼로 복사할 데이터 서술
    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = initData;
    subResourceData.RowPitch = byteSize;
    subResourceData.SlicePitch = subResourceData.RowPitch;

    // 데이터를 기본 버퍼 리소스로 복사하기 위해 상태 전이를 통지
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
    UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

    // 아직 실제로 복사가 이루어지지 않았으므로 업로드 버퍼를 삭제하지 않음

    return defaultBuffer;
}
