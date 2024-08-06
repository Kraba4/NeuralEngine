#pragma once

#include "ConvolutionLayer.h"
#include <graphics/d3d12/CommonGraphicsHeaders.h>
#include <graphics/d3d12/classes/resource/BufferAndTexture.h>
#include <graphics/d3d12/classes/DescriptorHeap.h>

#include <DirectML.h>
#include <DirectMLX.h>
#include <directx_tool_kit/Inc/ResourceUploadBatch.h>
#include <directx_tool_kit/Inc/DescriptorHeap.h>

#include <array>
#include <memory>

namespace neural::graphics {
class Model {
public:
    void initialize(ID3D12Device* a_device, IDMLDevice* m_dmlDevice, DescriptorHeap* a_srvUavHeap,
                    uint32_t a_inputWidth, uint32_t a_inputHeight);
    void uploadWeights();

    void setInitializationBindings();
    void setExecutionBindings();

    void dispatchInitialization(IDMLCommandRecorder* a_dmlCommandRecorder, ID3D12GraphicsCommandList* a_commandList);
    void dispatch(IDMLCommandRecorder* a_dmlCommandRecorder, ID3D12GraphicsCommandList* a_commandList);

    Buffer& getInputBuffer() {
        return m_input;
    }
    Buffer& getOutputBuffer() {
        return m_output;
    }
    ID3D12DescriptorHeap* getID3D12DescriptorHeap() {
        return m_descriptorHeap->Heap();
    }
private:
    ID3D12Device* m_device;
    IDMLDevice*   m_dmlDevice;
    ConvolutionLayersContainer m_convolutionLayers;
    std::unique_ptr<DirectX::DescriptorHeap> m_descriptorHeap;
    Buffer m_input;
    Buffer m_output;
    Buffer m_intermediate;
};
}