#include "RootSignature.h"

namespace neural::graphics {

void RootSignature::initialize(ID3D12Device* a_device, const std::vector<RootParameter>& a_slots)
{
	assert(a_device);
	std::vector<CD3DX12_ROOT_PARAMETER> slotRootParameters(a_slots.size());
	std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> ranges;
	for (int i = 0; i < a_slots.size(); ++i) {
		const auto& slot = a_slots[i];
		auto& rootParameter = slotRootParameters[i];
		switch (slot.parameterType)
		{
		case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
		{
			ranges.emplace_back();
			ranges.back().reserve(slot.descriptorTableRanges.size());
			for (int j = 0; j < slot.descriptorTableRanges.size(); ++j) {
				const auto& rangeParams = slot.descriptorTableRanges[j];
				ranges.back().push_back({ .RangeType = rangeParams.rangeType,
								   .NumDescriptors = rangeParams.numDescriptors,
								   .BaseShaderRegister = rangeParams.baseShaderRegister,
								   .RegisterSpace = rangeParams.registerSpace,
								   .OffsetInDescriptorsFromTableStart = rangeParams.offsetInDescriptorsFromTableStart });
			}
			rootParameter.InitAsDescriptorTable(ranges.back().size(), ranges.back().data(), slot.shaderVisibility);
		}
			break;
		case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
			rootParameter.InitAsConstants(slot.constants.num32BitValues, slot.constants.baseShaderRegister,
				                          slot.constants.registerSpace, slot.shaderVisibility);
			break;
		case D3D12_ROOT_PARAMETER_TYPE_CBV:
			rootParameter.InitAsConstantBufferView(slot.descriptor.baseShaderRegister, slot.descriptor.registerSpace,
				                                   slot.shaderVisibility);
			break;
		case D3D12_ROOT_PARAMETER_TYPE_SRV:
			rootParameter.InitAsShaderResourceView(slot.descriptor.baseShaderRegister, slot.descriptor.registerSpace,
				                                   slot.shaderVisibility);
			break;
		case D3D12_ROOT_PARAMETER_TYPE_UAV:
			rootParameter.InitAsUnorderedAccessView(slot.descriptor.baseShaderRegister, slot.descriptor.registerSpace,
												    slot.shaderVisibility);
			break;
		default:
			assert(false);
			break;
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
	DX_CALL(a_device->GetDeviceRemovedReason());
}

ID3D12RootSignature* RootSignature::getID3D12RootSignature()
{
	return m_rootSignature.Get();
}

}