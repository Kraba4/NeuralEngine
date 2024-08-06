#include "ConvolutionLayer.h"

namespace neural::graphics {
void ConvolutionLayersContainer::createOperatorInitializerBinding(DirectX::DescriptorHeap* a_descriptorHeap,
                                                                   uint32_t a_startIndex) {
    auto bindingProps = m_operatorInitializer->GetBindingProperties();
    assert(bindingProps.PersistentResourceSize == 0);

    DML_BINDING_TABLE_DESC tableDesc = {
        m_operatorInitializer.Get(),
        a_descriptorHeap->GetCpuHandle(a_startIndex),
        a_descriptorHeap->GetGpuHandle(a_startIndex),
        bindingProps.RequiredDescriptorCount
    };
    DX_CALL(m_dmlDevice->CreateBindingTable(&tableDesc, IID_PPV_ARGS(&m_initBindingTable)));

    const DML_BUFFER_BINDING emptyBufferBinding = { nullptr, 0, 0 };
    const DML_BINDING_DESC emptyBindingDesc = { DML_BINDING_TYPE_NONE, nullptr };

#if DML_MANAGED_WEIGHTS
    // Bind the weight tensors at initialization instead of at execution. This lets DirectML reformat them
    // and improve performance on some hardware.
    std::vector<std::array<DML_BUFFER_BINDING, 3>> bufferBindings;
    std::vector<DML_BUFFER_ARRAY_BINDING>          bufferArrayBindings;
    std::vector<DML_BINDING_DESC>                  bindingDescs;

    bufferBindings.reserve(m_layers.size());
    bufferArrayBindings.reserve(m_layers.size());
    bindingDescs.reserve(m_layers.size());

    for (int i = 0; i < m_layers.size(); ++i) {
        Buffer* filter = m_layers[i].getFilterWeights();
        Buffer* bias   = m_layers[i].getBiasWeights();

        DML_BUFFER_BINDING biasBufferBinding = emptyBufferBinding;
        if (bias) {
            biasBufferBinding = {bias->getID3D12Resource(), 0, bias->getID3D12Resource()->GetDesc().Width};
        }
        bufferBindings.push_back({ 
            emptyBufferBinding, 
            {filter->getID3D12Resource(), 0, filter->getID3D12Resource()->GetDesc().Width},
            biasBufferBinding
        });

        bufferArrayBindings.push_back({
            .BindingCount = static_cast<uint32_t>(bufferBindings[i].size()),
            .Bindings     = bufferBindings[i].data()
        });

        bindingDescs.push_back({
                .Type = DML_BINDING_TYPE_BUFFER_ARRAY,
                .Desc = &bufferArrayBindings[i]
        });
    }
    m_initBindingTable->BindInputs(bindingDescs.size(), bindingDescs.data());
#else
    m_initBindingTable->BindInputs(0, nullptr);
#endif

    // If the operator requires a persistent resource, it must be bound as output for the initializer.
    std::vector<DML_BUFFER_BINDING> persistentBufferBindings(m_layers.size());
    std::vector<DML_BINDING_DESC>   persistentBindings(m_layers.size());
    for (int i = 0; i < m_layers.size(); ++i) {
        if (m_layers[i].getPersistentBuffer() != nullptr) {
            ID3D12Resource* persistentResource = m_layers[i].getPersistentBuffer()->getID3D12Resource();
            persistentBufferBindings[i]  = { persistentResource, 0, persistentResource->GetDesc().Width };
            persistentBindings[i] = { DML_BINDING_TYPE_BUFFER, &persistentBufferBindings[i] };
        }
        else {
            persistentBindings[i] = emptyBindingDesc;
        }
    }

    m_initBindingTable->BindOutputs(persistentBindings.size(), persistentBindings.data());


    if (bindingProps.TemporaryResourceSize > 0)
    {
        m_operatorInitializerTemporaryResource = std::make_unique<Buffer>();
        m_operatorInitializerTemporaryResource->initialize(m_device, nullptr, 
            {
                .size = bindingProps.TemporaryResourceSize,
                .elementSize = 1,
                .usageFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
            }
        );

        auto resource = m_operatorInitializerTemporaryResource->getID3D12Resource();
        DML_BUFFER_BINDING tempBuffer = { resource, 0, resource->GetDesc().Width };
        DML_BINDING_DESC tempBinding = { DML_BINDING_TYPE_BUFFER, &tempBuffer };
        m_initBindingTable->BindTemporaryResource(&tempBinding);
    }
}

void ConvolutionLayersContainer::createOperatorInitializer()
{
    // Create OperatorInitializer
    std::vector<IDMLCompiledOperator*> compiledOperators;
    compiledOperators.reserve(m_layers.size());
    for (int i = 0; i < m_layers.size(); ++i) {
        compiledOperators.push_back(m_layers[i].getCompiledOperator());
    }
    DX_CALL(m_dmlDevice->CreateOperatorInitializer(compiledOperators.size(), compiledOperators.data(),
                                                    IID_PPV_ARGS(m_operatorInitializer.GetAddressOf())));
}

uint32_t ConvolutionLayersContainer::getDescriptorCount()
{
    assert(m_operatorInitializer);  // need create operator initializer

    if (m_descriptorCount == 0) {
        // Get required descriptor count
        auto bindingProps = m_operatorInitializer->GetBindingProperties();

        uint32_t requiredDescriptorCount = bindingProps.RequiredDescriptorCount;

        for (size_t i = 0; i < m_layers.size(); i++)
        {
            bindingProps = m_layers[i].getCompiledOperator()->GetBindingProperties();
            requiredDescriptorCount = std::max(requiredDescriptorCount, bindingProps.RequiredDescriptorCount);
        }
        m_descriptorCount = requiredDescriptorCount;
    }
    return m_descriptorCount;
}
}