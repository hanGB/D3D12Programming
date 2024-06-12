#pragma once
#include "stdafx.h"

namespace d3d12_init {

	D3D12_ROOT_PARAMETER SetAndGetRootParameter(int num32BitValues, int shaderRegister);
	D3D12_ROOT_SIGNATURE_DESC SetAndGetRootSignatureDesc(D3D12_ROOT_PARAMETER* parameters, int numParameters);

	D3D12_ROOT_PARAMETER SetAndGetRootParameter(int num32BitValues, int shaderRegister)
	{
		D3D12_ROOT_PARAMETER rootParameter;

		rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		rootParameter.Constants.Num32BitValues = num32BitValues;
		rootParameter.Constants.ShaderRegister = shaderRegister;
		rootParameter.Constants.RegisterSpace = 0;
		rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		return rootParameter;
	}

	D3D12_ROOT_SIGNATURE_DESC SetAndGetRootSignatureDesc(D3D12_ROOT_PARAMETER* parameters, int numParameters)
	{
		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;

		::ZeroMemory(&rootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
		rootSignatureDesc.NumParameters = numParameters;
		rootSignatureDesc.pParameters = parameters;
		rootSignatureDesc.NumStaticSamplers = 0;
		rootSignatureDesc.pStaticSamplers = NULL;
		rootSignatureDesc.Flags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

		return rootSignatureDesc;
	}

	ID3D12RootSignature* CreateRootSignature(ID3D12Device* device)
	{
		ID3D12RootSignature* rootSignature = nullptr;
		ID3DBlob* rootSignatureBlob, * errorBlob;

		D3D12_ROOT_PARAMETER rootParameter[3] = { SetAndGetRootParameter(16, 0), SetAndGetRootParameter(32, 1) };
		rootParameter[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		rootParameter[2].Descriptor.ShaderRegister = 0;
		rootParameter[2].Descriptor.RegisterSpace = 0;
		rootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = SetAndGetRootSignatureDesc(rootParameter, _countof(rootParameter));

		// Blob 생성
		::D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			&rootSignatureBlob, &errorBlob);

		// 루트 시그니처 생성
		device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
			rootSignatureBlob->GetBufferSize(),
			__uuidof(ID3D12RootSignature), (void**)&rootSignature);

		// blob 삭제
		if (rootSignatureBlob) rootSignatureBlob->Release();
		if (errorBlob) errorBlob->Release();

		return rootSignature;
	}

}