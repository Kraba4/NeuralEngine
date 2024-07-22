#include "GraphicsPipeline.h"

namespace neural::graphics {

void GraphicsPipeline::initialize(ID3D12Device* a_device, std::string_view a_debugName, CreateInfo a_info) {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc;
    ZeroMemory(&pipelineDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    if (a_info.inputLayout.size() > 0) {
        pipelineDesc.InputLayout = { a_info.inputLayout.data(), static_cast<uint32_t>(a_info.inputLayout.size()) };
    }
    pipelineDesc.pRootSignature = a_info.rootSignature.getID3D12RootSignature();
    std::vector<char> vsByteCode = utils::loadBinary(a_info.vertexShaderPath);
    pipelineDesc.VS = { vsByteCode.data(), vsByteCode.size() };
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
}