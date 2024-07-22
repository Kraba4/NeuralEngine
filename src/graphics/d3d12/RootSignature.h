#pragma once

#include <utils/Macros.h>
#include <utils/Utils.h>

#include <d3d12.h>
#include <wrl.h>
#include <d3dx12.h>

#include <vector>

using Microsoft::WRL::ComPtr;
namespace neural::graphics {

class RootSignature {
public:
    struct DescriptorRange {
        D3D12_DESCRIPTOR_RANGE_TYPE rangeType;
        UINT baseShaderRegister = UINT_MAX;
        UINT numDescriptors = UINT_MAX;
        UINT registerSpace = 0;
        UINT offsetInDescriptorsFromTableStart =
            D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    };

    struct RootConstants
    {
        UINT baseShaderRegister = UINT_MAX;
        UINT num32BitValues = UINT_MAX;
        UINT registerSpace = 0;
    };

    struct RootDescriptor
    {
        UINT baseShaderRegister = UINT_MAX;
        UINT registerSpace = 0;
    };


    struct RootParameter {
        D3D12_ROOT_PARAMETER_TYPE parameterType;

        std::vector<DescriptorRange> descriptorTableRanges; // fill one depending on parameterType
        RootConstants constants;                            // fill one depending on parameterType
        RootDescriptor descriptor;                          // fill one depending on parameterType

        D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    };

    void initialize(ID3D12Device* a_device, const std::vector<RootParameter>& slots);
    ID3D12RootSignature* getID3D12RootSignature();
private:
    ComPtr<ID3D12RootSignature> m_rootSignature;
};
}