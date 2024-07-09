#pragma once

#include <utils/Macros.h>
#include <utils/Utils.h>
#include "RootSignature.h"

#include "CommonGraphicsHeaders.h"

#include <string_view>
#include <vector>
#include <cassert>

using Microsoft::WRL::ComPtr;
namespace neural::graphics {

class GraphicsPipeline {
public:
    struct CreateInfo {
        RootSignature rootSignature;
        std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
        std::string vertexShaderPath;
        D3D12_RASTERIZER_DESC rasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        std::string pixelShaderPath;
        D3D12_BLEND_DESC blendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        D3D12_DEPTH_STENCIL_DESC depthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);;
        UINT sampleMask = UINT_MAX;
        D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        std::vector<DXGI_FORMAT> RTVFormats;
        DXGI_FORMAT DSVFormat = DXGI_FORMAT_UNKNOWN;
        DXGI_SAMPLE_DESC sampleDesc = { .Count = 1, .Quality = 0 };
    };

    //GraphicsPipeline() {};

    void initialize(ID3D12Device* a_device, std::string_view a_debugName, CreateInfo a_info);
    void setAsPipeline(ComPtr<ID3D12GraphicsCommandList> a_commandList);
    //ID3D12PipelineState* getID3D12Pipeline() {
    //    return m_pipeline.Get();
    //}
private:
    ComPtr<ID3D12PipelineState> m_pipeline;
};
}