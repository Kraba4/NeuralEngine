#pragma once

#include <utils/Macros.h>
#include "Utils.h"
#include "RootSignature.h"

#include <d3d12.h>
#include <wrl.h>
#include <d3dx12.h>

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

    GraphicsPipeline() {};

    void initialize(ID3D12Device* a_device, std::string_view a_debugName, CreateInfo a_info) {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc;
        ZeroMemory(&pipelineDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
        if (a_info.inputLayout.size() > 0) {
            pipelineDesc.InputLayout = { a_info.inputLayout.data(), static_cast<uint32_t>(a_info.inputLayout.size()) };
        }
        pipelineDesc.pRootSignature = a_info.rootSignature.getID3D12RootSignature();
        std::vector<char> vsByteCode = utils::loadBinary(a_info.vertexShaderPath);
        pipelineDesc.VS = { vsByteCode.data(), vsByteCode.size()};
        std::vector<char> psByteCode = utils::loadBinary(a_info.pixelShaderPath);
        pipelineDesc.PS = { psByteCode.data(), psByteCode.size() };
        pipelineDesc.RasterizerState = a_info.rasterizerState;
        pipelineDesc.BlendState = a_info.blendState;
        pipelineDesc.DepthStencilState = a_info.depthStencilState;
        pipelineDesc.SampleMask = a_info.sampleMask;
        pipelineDesc.PrimitiveTopologyType = a_info.primitiveTopologyType;
        pipelineDesc.NumRenderTargets = a_info.RTVFormats.size();
        for (int i = 0; i < a_info.RTVFormats.size(); ++i) {
            pipelineDesc.RTVFormats[i] = a_info.RTVFormats[i];
        }
        pipelineDesc.SampleDesc = a_info.sampleDesc;
        pipelineDesc.DSVFormat = a_info.DSVFormat;

        DX_CALL(a_device->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(m_pipeline.GetAddressOf())));
    }
    void setAsPipeline(ComPtr<ID3D12GraphicsCommandList> a_commandList) {
        a_commandList->SetPipelineState(m_pipeline.Get());
    }
    //ID3D12PipelineState* getID3D12Pipeline() {
    //    return m_pipeline.Get();
    //}
private:
    ComPtr<ID3D12PipelineState> m_pipeline;
};
}