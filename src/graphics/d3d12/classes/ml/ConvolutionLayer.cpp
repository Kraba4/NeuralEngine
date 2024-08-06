#include "ConvolutionLayer.h"
#include <utils/Float16Compressor.h>

namespace neural::graphics {

void ConvolutionLayer::initialize(ID3D12Device* a_device,
                                    IDMLDevice* a_dmlDevice,
                                    const ConvolutionLayerCreateInfo& a_createInfo) 
{
    /////////////////////////////////  Create Convolution operator  ///////////////////////////////

    const std::array<uint32_t, 4>& inputSizes  = a_createInfo.inputSizes;
    const std::array<uint32_t, 4>& filterSizes = a_createInfo.filterSizes;
    const std::array<uint32_t, 4>  biasSizes   = { 1, a_createInfo.filterSizes[0], 1, 1 };
    const std::array<uint32_t, 4>  outputSizes = {  a_createInfo.inputSizes[0],
                                                    a_createInfo.filterSizes[0],  // from filterSizes
                                                    a_createInfo.inputSizes[2],
                                                    a_createInfo.inputSizes[3]  };

    // Strides are placed here (not just in getBufferTensorDesc) 
    // because DML_BUFFER_TENSOR_DESC refers to them by pointer
    std::array<uint32_t, 4> inputStrides;
    std::array<uint32_t, 4> filterStrides;
    std::array<uint32_t, 4> biasStrides;
    std::array<uint32_t, 4> outputStrides;

    DML_BUFFER_TENSOR_DESC inputBufferTensorDesc  = getBufferTensorDesc(a_createInfo.dataType, inputSizes,  inputStrides);
    DML_BUFFER_TENSOR_DESC filterBufferTensorDesc = getBufferTensorDesc(a_createInfo.dataType, filterSizes, filterStrides);
    DML_BUFFER_TENSOR_DESC biasBufferTensorDesc   = getBufferTensorDesc(a_createInfo.dataType, biasSizes,   biasStrides);
    DML_BUFFER_TENSOR_DESC outputBufferTensorDesc = getBufferTensorDesc(a_createInfo.dataType, outputSizes, outputStrides);

#if DML_MANAGED_WEIGHTS
    filterBufferTensorDesc.Flags = DML_TENSOR_FLAG_OWNED_BY_DML;
    biasBufferTensorDesc.Flags   = DML_TENSOR_FLAG_OWNED_BY_DML;
#endif

    DML_TENSOR_DESC inputTensorDesc  = { DML_TENSOR_TYPE_BUFFER, &inputBufferTensorDesc };
    DML_TENSOR_DESC filterTensorDesc = { DML_TENSOR_TYPE_BUFFER, &filterBufferTensorDesc };
    DML_TENSOR_DESC biasTensorDesc   = { DML_TENSOR_TYPE_BUFFER, &biasBufferTensorDesc };
    DML_TENSOR_DESC outputTensorDesc = { DML_TENSOR_TYPE_BUFFER, &outputBufferTensorDesc };


    // From https://github.com/microsoft/DirectML/tree/master/Samples/DirectMLSuperResolution
    // Describe, create, and compile convolution operator

    // The output size of a convolution operation is given by:
    //  height = (inputHeight - filterHeight + 2*paddingHeight) / filterStride + 1
    //  width  = (inputWidth  - filterWidth  + 2*paddingWidth ) / filterStride + 1
    //
    // We want to preserve the height and width, so assuming stride is 1, we get:
    //  paddingHeight = (filterHeight - 1) / 2
    //  paddingWidth  = (filterWidth  - 1) / 2
    // If padding is fractional, we pad unevenly with ceil/floor.
    UINT paddingHeightTop = static_cast<UINT>(ceil((filterSizes[2] - 1) / 2.0f));
    UINT paddingHeightBottom = static_cast<UINT>(floor((filterSizes[2] - 1) / 2.0f));
    UINT paddingWidthLeft = static_cast<UINT>(ceil((filterSizes[3] - 1) / 2.0f));
    UINT paddingWidthRight = static_cast<UINT>(floor((filterSizes[3] - 1) / 2.0f));

    UINT strides[] = { 1, 1 };
    UINT dilations[] = { 1, 1 };
    UINT startPadding[] = { paddingHeightTop, paddingWidthLeft };
    UINT endPadding[] = { paddingHeightBottom, paddingWidthRight };
    UINT outputPadding[] = { 0, 0 };

    DML_ACTIVATION_RELU_OPERATOR_DESC fusedReluDesc = { 0 };
    DML_OPERATOR_DESC activationDesc = { DML_OPERATOR_ACTIVATION_RELU, &fusedReluDesc };

    DML_CONVOLUTION_OPERATOR_DESC convolutionDesc = {
        .InputTensor = &inputTensorDesc,
        .FilterTensor = &filterTensorDesc,
        .BiasTensor = a_createInfo.useBiasAndActivation ? &biasTensorDesc : nullptr,
        .OutputTensor = &outputTensorDesc,
        .Mode = DML_CONVOLUTION_MODE_CROSS_CORRELATION,  // TODO: what is it?
        .Direction = DML_CONVOLUTION_DIRECTION_FORWARD,
        .DimensionCount = 2,
        .Strides = strides,
        .Dilations = dilations,
        .StartPadding = startPadding,
        .EndPadding = endPadding,
        .OutputPadding = outputPadding,
        .GroupCount = 1,  // TODO: what is it?
        .FusedActivation = a_createInfo.useBiasAndActivation ? &activationDesc : nullptr
    };
    DML_OPERATOR_DESC operatorDesc = { DML_OPERATOR_CONVOLUTION, &convolutionDesc };
    ComPtr<IDMLOperator> op;
    DX_CALL(a_dmlDevice->CreateOperator(&operatorDesc, IID_PPV_ARGS(op.ReleaseAndGetAddressOf())));

    DML_EXECUTION_FLAGS flag = DML_EXECUTION_FLAG_NONE;
    if (a_createInfo.dataType == DML_TENSOR_DATA_TYPE_FLOAT16) { 
        flag = DML_EXECUTION_FLAG_ALLOW_HALF_PRECISION_COMPUTATION;
    }                 
    DX_CALL(a_dmlDevice->CompileOperator(op.Get(), flag, IID_PPV_ARGS(&m_compiledOperator)));
    

    /////////////////////////////////  Create Resources for Layer  ///////////////////////////////

    m_filterWeights = std::make_unique<Buffer>();
    m_filterWeights->initialize(a_device, nullptr, {
        .size = filterBufferTensorDesc.TotalTensorSizeInBytes,
        .elementSize = 1,
        .usageFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
    });

    if (a_createInfo.useBiasAndActivation)
    {
        m_biasWeights = std::make_unique<Buffer>();
        m_biasWeights->initialize(a_device, nullptr, {
            .size = biasBufferTensorDesc.TotalTensorSizeInBytes,
            .elementSize = 1,
            .usageFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
        });
    }

    DML_BINDING_PROPERTIES bindingProperties = m_compiledOperator->GetBindingProperties();
    if (bindingProperties.PersistentResourceSize > 0) 
    {
        m_persistentResource = std::make_unique<Buffer>();
        m_persistentResource->initialize(a_device, nullptr, {
            .size = bindingProperties.PersistentResourceSize,
            .elementSize = 1,
            .usageFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
        });
    }

    if (bindingProperties.TemporaryResourceSize > 0)
    {
        m_temporaryResource = std::make_unique<Buffer>();
        m_temporaryResource->initialize(a_device, nullptr, {
            .size = bindingProperties.TemporaryResourceSize,
            .elementSize = 1,
            .usageFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
        });
    }

    m_dmlDevice = a_dmlDevice;
    m_filterSizes = filterSizes;
    m_inputTotalSize = inputBufferTensorDesc.TotalTensorSizeInBytes;
    m_outputTotalSize = outputBufferTensorDesc.TotalTensorSizeInBytes;
}

DML_BUFFER_TENSOR_DESC ConvolutionLayer::getBufferTensorDesc(DML_TENSOR_DATA_TYPE a_dataType,
                                           const std::array<uint32_t, 4>& a_sizes,
                                           std::array<uint32_t, 4>& a_strides) {
    getStrides(a_sizes, m_tensorLayout, a_strides);
    uint64_t totalSize = DMLCalcBufferTensorSize(a_dataType, a_sizes.size(), a_sizes.data(), a_strides.data());

    DML_BUFFER_TENSOR_DESC bufferTensorDesc = {
        .DataType = a_dataType,
        .Flags = DML_TENSOR_FLAG_NONE,
        .DimensionCount = static_cast<UINT>(a_sizes.size()),
        .Sizes = a_sizes.data(),
        .Strides = a_strides.data(),
        .TotalTensorSizeInBytes = totalSize,
        .GuaranteedBaseOffsetAlignment = 0
    };
    return bufferTensorDesc;
}

void ConvolutionLayer::getStrides(const std::array<uint32_t, 4>& a_sizes, TensorLayout layout, std::array<uint32_t, 4>& a_strides)
{
    switch (layout)
    {
    case TensorLayout::NHWC:
        a_strides[0] = a_sizes[1] * a_sizes[2] * a_sizes[3];
        a_strides[1] = 1;
        a_strides[2] = a_sizes[1] * a_sizes[3];
        a_strides[3] = a_sizes[1];
        break;

    default:
        a_strides[0] = a_sizes[1] * a_sizes[2] * a_sizes[3];
        a_strides[1] = a_sizes[2] * a_sizes[3];
        a_strides[2] = a_sizes[3];
        a_strides[3] = 1;
    }
}

void ConvolutionLayer::uploadWeightsFloat16(DirectX::ResourceUploadBatch& a_uploadBatch, 
                                          const std::vector<float>* a_filterWeights, 
                                          const std::vector<float>* a_scaleWeights,
                                          const std::vector<float>* a_shiftWeights)
{
    const bool useBias = (m_biasWeights->getID3D12Resource() != nullptr);
    assert(!useBias || (a_scaleWeights && a_shiftWeights));

    std::vector<uint16_t> filterWeights;
    std::vector<uint16_t> biasWeights;

    const uint32_t N = m_filterSizes[0];
    const uint32_t C = m_filterSizes[1];
    const uint32_t H = m_filterSizes[2];
    const uint32_t W = m_filterSizes[3];
    
    // From https://github.com/microsoft/DirectML/tree/master/Samples/DirectMLSuperResolution
    for (uint32_t n = 0; n < N; n++)
    {
        switch (m_tensorLayout)
        {
        case TensorLayout::NHWC:
            // We need to convert the weights from NCHW to NHWC.
            for (uint32_t h = 0; h < H; h++)
                for (uint32_t w = 0; w < W; w++)
                    for (uint32_t c = 0; c < C; c++)
                    {
                        // Apply the scale weight now so we don't need a normalization layer
                        uint32_t idx = w + h * W + c * H*W + n * C*H*W;
                        float scaledWeight = useBias ? (*a_filterWeights)[idx] * (*a_scaleWeights)[n] 
                                                     : (*a_filterWeights)[idx];
                        filterWeights.push_back(Float16Compressor::compress(scaledWeight));
                    }
            break;

        default:
            // Weights are already in the right order
            for (uint32_t i = 0; i < C*H*W; i++)
            {
                // Apply the scale weight now so we don't need a normalization layer
                uint32_t idx = n * C*H*W + i;
                float scaledWeight = useBias ? (*a_filterWeights)[idx] * (*a_scaleWeights)[n] 
                                             : (*a_filterWeights)[idx];
                filterWeights.push_back(Float16Compressor::compress(scaledWeight));
            }
        }

        if (useBias)
        {
            // Technically this is initialBias*scale+shift, but the initial bias is 0
            biasWeights.push_back(Float16Compressor::compress((*a_shiftWeights)[n]));
        }
    }

    // Upload to the GPU
    D3D12_SUBRESOURCE_DATA weightsData = {};
    weightsData.pData = filterWeights.data();
    a_uploadBatch.Upload(m_filterWeights->getID3D12Resource(), 0, &weightsData, 1);

    if (useBias)
    {
        weightsData.pData = biasWeights.data();
        a_uploadBatch.Upload(m_biasWeights->getID3D12Resource(), 0, &weightsData, 1);
    }
}

void ConvolutionLayer::createBinding(const DescriptorHeap::Handle& handle) {
    auto bindingProps = m_compiledOperator->GetBindingProperties();

    DML_BINDING_TABLE_DESC tableDesc = {
        .Dispatchable        = m_compiledOperator.Get(),
        .CPUDescriptorHandle = handle.cpu,
        .GPUDescriptorHandle = handle.gpu,
        .SizeInDescriptors   = bindingProps.RequiredDescriptorCount
    };
    DX_CALL(m_dmlDevice->CreateBindingTable(&tableDesc, IID_PPV_ARGS(m_bindingTable.GetAddressOf())));
}

void ConvolutionLayer::bindResources(const BindResourcesDesc& a_bindDesc) {
    DML_BUFFER_BINDING inputBufferBinding = { a_bindDesc.input, 0, a_bindDesc.input->GetDesc().Width };
    DML_BINDING_DESC inputBinding = { DML_BINDING_TYPE_BUFFER, &inputBufferBinding };

    const DML_BUFFER_BINDING emptyBufferBinding = { nullptr, 0, 0 };
    const DML_BINDING_DESC emptyBindingDesc = { DML_BINDING_TYPE_NONE, nullptr };
    
#if DML_MANAGED_WEIGHTS
    // The weights are stored in the persistent resource and shouldn't be bound separately.
    DML_BINDING_DESC inputBindings[] = { inputBinding, emptyBindingDesc, emptyBindingDesc };
#else
    // Bind the weight resources
    DML_BUFFER_BINDING filterBufferBinding = { m_filterWeights->getID3D12Resource(),
                                               0, m_filterWeights->getID3D12Resource()->GetDesc().Width };
    DML_BINDING_DESC filterBinding = { DML_BINDING_TYPE_BUFFER, &filterBufferBinding };

    DML_BUFFER_BINDING biasBufferBinding;
    DML_BINDING_DESC biasBinding;

    if (m_biasWeights.get()) {
        biasBufferBinding = { m_biasWeights->getID3D12Resource(), 
                              0, m_biasWeights->getID3D12Resource()->GetDesc().Width };
        biasBinding = { DML_BINDING_TYPE_BUFFER, &biasBufferBinding };
    }
    else {
        biasBinding = emptyBindingDesc;
    }

    DML_BINDING_DESC inputBindings[] = { inputBinding, filterBinding, biasBinding };
#endif
    m_bindingTable->BindInputs(3, inputBindings);

    DML_BUFFER_BINDING outputBufferBinding = { a_bindDesc.output, 0, a_bindDesc.output->GetDesc().Width };
    DML_BINDING_DESC outputBinding = { DML_BINDING_TYPE_BUFFER, &outputBufferBinding };
    m_bindingTable->BindOutputs(1, &outputBinding);

    if (m_temporaryResource.get() != nullptr) {
        DML_BUFFER_BINDING tempBuffer = { m_temporaryResource.get()->getID3D12Resource(),
                                          0, m_temporaryResource.get()->getID3D12Resource()->GetDesc().Width };
        DML_BINDING_DESC tempBinding = { DML_BINDING_TYPE_BUFFER, &tempBuffer };
        m_bindingTable->BindTemporaryResource(&tempBinding);
    }

    if (m_persistentResource.get() != nullptr) {
        DML_BUFFER_BINDING persistentBuffer = { m_persistentResource.get()->getID3D12Resource(),
                                          0, m_persistentResource.get()->getID3D12Resource()->GetDesc().Width };
        DML_BINDING_DESC persistentBinding = { DML_BINDING_TYPE_BUFFER, &persistentBuffer };
        m_bindingTable->BindPersistentResource(&persistentBinding);
    }
}
}