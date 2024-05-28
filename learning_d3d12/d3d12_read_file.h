#pragma once
#include "stdafx.h"

namespace d3d12_util {
	
	ID3DBlob* ReadCsoToBuffer(const wchar_t* fileName) {
        ID3DBlob* blob = NULL;
        
        std::ifstream in(fileName, std::ios::binary);

        in.seekg(0, std::ios_base::end);
        auto size = (int)in.tellg();
        in.seekg(0, std::ios_base::beg);

        D3DCreateBlob(size, &blob);
        in.read((char*)blob->GetBufferPointer(), size);
        in.close();

        return blob;
	}
}