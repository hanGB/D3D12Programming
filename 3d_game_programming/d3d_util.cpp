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
