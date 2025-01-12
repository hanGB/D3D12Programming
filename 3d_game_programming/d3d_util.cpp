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

ComPtr<ID3DBlob> D3DUtil::CompileShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint, const std::string& target)
{
    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    HRESULT hr = S_OK;
    ComPtr<ID3DBlob> byteCode = nullptr;
    ComPtr<ID3DBlob> errors;
    hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, 
        entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

    if (errors != nullptr)
        OutputDebugStringA((char*)errors->GetBufferPointer());

    ThrowIfFailed(hr);

    return byteCode;
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

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> D3DUtil::GetStaticsSamplers()
{
    const CD3DX12_STATIC_SAMPLER_DESC pointWrap(    
        0,                                  // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_POINT,     // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,    // adressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,    // adressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP);   // adressW

    const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
        1,
        D3D12_FILTER_MIN_MAG_MIP_POINT,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

    const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
        2,
        D3D12_FILTER_MIN_MAG_MIP_LINEAR,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP);

    const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
        3,
        D3D12_FILTER_MIN_MAG_MIP_LINEAR,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

    const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
        4,                                  // shaderRegister
        D3D12_FILTER_ANISOTROPIC,           // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,    // adressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,    // adressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,    // adressW
        0.0f,                               // mipLODBias
        8);                                 // maxAnisotropy

    const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
        5,
        D3D12_FILTER_ANISOTROPIC,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
        0.0f,
        8);

    return { 
        pointWrap, pointClamp, 
        linearWrap, linearClamp, 
        anisotropicWrap, anisotropicClamp };
}

std::unique_ptr<Texture> D3DUtil::CreateTextureFromDDSFile(const char* name, const wchar_t* filename, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
    auto texture = std::make_unique<Texture>();

    texture->name = name;
    texture->filename = filename;
    std::unique_ptr<uint8_t[]> ddsData;
    std::vector<D3D12_SUBRESOURCE_DATA> subresources;

    // 텍스처 로드
    ThrowIfFailed(LoadDDSTextureFromFile(device, texture->filename.c_str(), texture->resource.GetAddressOf(), ddsData, subresources));

    // 업로드 힙 생성
    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture->resource.Get(), 0, (UINT)subresources.size());
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&texture->uploadHeap)));

    // 서브 리소스 데이터 업로드
    UpdateSubresources(cmdList, texture->resource.Get(), texture->uploadHeap.Get(), 0, 0, static_cast<UINT>(subresources.size()), subresources.data());

    // 리소스 배리어 설정 -> 필요 없는 경우도 있지만 명시적으로 리소스 배리어를 설정하는 것이 안전함
    cmdList->ResourceBarrier(1,
        &CD3DX12_RESOURCE_BARRIER::Transition(
            texture->resource.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    return texture;
}
