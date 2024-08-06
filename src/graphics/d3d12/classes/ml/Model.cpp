#include "Model.h"
#include <iostream>

namespace neural::graphics {
void Model::initialize(ID3D12Device* a_device, IDMLDevice* a_dmlDevice, DescriptorHeap* a_srvUavHeap,
                       uint32_t a_inputWidth, uint32_t a_inputHeight) {
    m_device = a_device;
    m_dmlDevice = a_dmlDevice;

    m_convolutionLayers.initialize(m_device, m_dmlDevice, 2);

    DML_TENSOR_DATA_TYPE dataType = DML_TENSOR_DATA_TYPE_FLOAT16;
    m_convolutionLayers[0].initialize(m_device, m_dmlDevice, {
        .dataType = dataType,
        .inputSizes = {1, 3, a_inputHeight, a_inputWidth},
        .filterSizes = {6, 3, 2, 2},
        .useBiasAndActivation = true
    });

    m_convolutionLayers[1].initialize(m_device, m_dmlDevice, {
        .dataType = dataType,
        .inputSizes = {1, 6, a_inputHeight, a_inputWidth},
        .filterSizes = {3, 6, 2, 2},
        .useBiasAndActivation = false
    });


    m_input.initialize(a_device, nullptr, {
        .size = m_convolutionLayers[0].getInputTotalSize(),
        .elementSize = 1,
        .usageFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
    });
    m_output.initialize(a_device, nullptr, {
        .size = m_convolutionLayers[1].getOutputTotalSize(),
        .elementSize = 1,
        .usageFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
    });
    m_intermediate.initialize(a_device, nullptr, {
        .size = m_convolutionLayers[0].getOutputTotalSize(),
        .elementSize = 1,
        .usageFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
    });
}

void Model::setInitializationBindings() {
    m_convolutionLayers.createOperatorInitializer();
    m_descriptorHeap = std::make_unique<DirectX::DescriptorHeap>(m_device,
                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
                        m_convolutionLayers.getDescriptorCount() * m_convolutionLayers.size());
    m_convolutionLayers.createOperatorInitializerBinding(m_descriptorHeap.get(), 0);
}

void Model::setExecutionBindings() {
    for (int i = 0; i < m_convolutionLayers.size(); ++i) {
        m_convolutionLayers[i].createBinding({
            .cpu = m_descriptorHeap->GetCpuHandle(i * m_convolutionLayers.getDescriptorCount()),
            .gpu = m_descriptorHeap->GetGpuHandle(i * m_convolutionLayers.getDescriptorCount())});
    }
    m_convolutionLayers[0].bindResources({.input = m_input.getID3D12Resource(),
                                          .output = m_intermediate.getID3D12Resource()});
    m_convolutionLayers[1].bindResources({.input = m_intermediate.getID3D12Resource(),
                                          .output = m_output.getID3D12Resource()});
}
    
void Model::dispatchInitialization(IDMLCommandRecorder* a_dmlCommandRecorder, 
                                   ID3D12GraphicsCommandList* a_commandList)
{
    a_dmlCommandRecorder->RecordDispatch(a_commandList, m_convolutionLayers.getInitializer(),
                                         m_convolutionLayers.getInitializerBinding());
}

void Model::dispatch(IDMLCommandRecorder* a_dmlCommandRecorder, 
                                   ID3D12GraphicsCommandList* a_commandList)
{
    a_dmlCommandRecorder->RecordDispatch(a_commandList, m_convolutionLayers[0].getCompiledOperator(),
                                         m_convolutionLayers[0].getBinding());
    a_dmlCommandRecorder->RecordDispatch(a_commandList, m_convolutionLayers[1].getCompiledOperator(),
                                         m_convolutionLayers[1].getBinding());
}
}