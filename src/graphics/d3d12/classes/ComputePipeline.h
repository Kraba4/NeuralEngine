#pragma once

#include <utils/Macros.h>
#include <utils/Utils.h>
#include "RootSignature.h"

#include <graphics/d3d12/CommonGraphicsHeaders.h>

#include <string_view>
#include <vector>
#include <cassert>

namespace neural::graphics {

class ComputePipeline {
public:
    struct CreateInfo {
        RootSignature rootSignature;
        std::string computeShaderPath;
    };

    void initialize(ID3D12Device* a_device, std::string_view a_debugName, CreateInfo a_info);

    ID3D12PipelineState* getID3D12Pipeline() {
        return m_pipeline.Get();
    }

private:
    ComPtr<ID3D12PipelineState> m_pipeline;
};
}