#pragma once

#include <utils/Macros.h>
#include <utils/Utils.h>

#include <d3d12.h>
#include <wrl.h>
#include <d3dx12.h>

#include <vector>

using Microsoft::WRL::ComPtr;
namespace neural::graphics {

class GraphicsPipeline;
class RootSignature {
public:
	enum class RootParameterType {
		DescriptorTable, Constants, Descriptor
	};

	struct DescriptorTable {
		D3D12_DESCRIPTOR_RANGE_TYPE descriptorType;
		uint32_t numDescriptors;
		uint32_t baseShaderRegister;
		D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL;
	};

	struct RootParameter {
		RootParameterType parameterType;
		union {
			DescriptorTable descriptorTable;
		};
	};

	//RootSignature() {};
	void initialize(ID3D12Device* a_device, std::vector<RootParameter> slots);
	void bindResources(ComPtr<ID3D12GraphicsCommandList> a_commandList);
	void setAsRootSignature(ComPtr<ID3D12GraphicsCommandList> a_commandList);
private:
	friend class GraphicsPipeline;
	ID3D12RootSignature* getID3D12RootSignature();
	ComPtr<ID3D12RootSignature> m_rootSignature;
};
}