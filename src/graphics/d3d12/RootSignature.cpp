#include "RootSignature.h"

namespace neural::graphics {

void RootSignature::initialize(ID3D12Device* a_device, std::vector<RootParameter> slots)
{
	std::vector<CD3DX12_ROOT_PARAMETER> slotRootParameters(slots.size());
	for (int i = 0; i < slots.size(); ++i) {
		if (slots[i].parameterType == RootParameterType::DescriptorTable) {
			CD3DX12_DESCRIPTOR_RANGE cbvTable;
			const DescriptorTable& table = slots[i].descriptorTable;
			cbvTable.Init(table.descriptorType, table.numDescriptors, table.baseShaderRegister);
			slotRootParameters[i].InitAsDescriptorTable(1, &cbvTable, table.visibility);
		}
		else {
			assert(false);
		}
	}

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(slotRootParameters.size(), slotRootParameters.data(), 0,
		nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	DX_CALL(hr);
	DX_CALL(a_device->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_rootSignature)));
}

void RootSignature::bindResources(ComPtr<ID3D12GraphicsCommandList> a_commandList)
{
}

void RootSignature::setAsRootSignature(ComPtr<ID3D12GraphicsCommandList> a_commandList)
{
	a_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
}

ID3D12RootSignature* RootSignature::getID3D12RootSignature()
{
	return m_rootSignature.Get();
}

}