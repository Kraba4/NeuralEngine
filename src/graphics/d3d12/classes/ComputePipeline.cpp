#include "ComputePipeline.h"
#include <utils/Utils.h>

namespace neural::graphics {

void ComputePipeline::initialize(ID3D12Device* a_device, std::string_view a_debugName, CreateInfo a_info) {
    D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
    desc.pRootSignature = a_info.rootSignature.getID3D12RootSignature();
    std::vector<char> csByteCode = utils::loadBinary(a_info.computeShaderPath);
    desc.CS = {csByteCode.data(), csByteCode.size()};
    desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    a_device->CreateComputePipelineState(&desc, IID_PPV_ARGS(&m_pipeline));
}
}