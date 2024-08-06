#pragma once
#include <graphics/d3d12/CommonGraphicsHeaders.h>
#include <graphics/d3d12/classes/resource/BufferAndTexture.h>

#include <DirectML.h>
#include <DirectMLX.h>
#include <directx_tool_kit/Inc/ResourceUploadBatch.h>
#include <directx_tool_kit/Inc/DescriptorHeap.h>

#include <cstdint>
#include <vector>
#include <array>
#include <unordered_map>
#include <memory>

namespace neural::graphics {
struct ConvolutionLayerCreateInfo {
    DML_TENSOR_DATA_TYPE dataType;
    std::array<uint32_t, 4> inputSizes;
    std::array<uint32_t, 4> filterSizes;
    bool useBiasAndActivation;
};
class ConvolutionLayer {
public:
    enum class TensorLayout
    {
        Default,
        NHWC
    };

    void initialize(ID3D12Device* a_device,
                    IDMLDevice* a_dmlDevice,
                    const ConvolutionLayerCreateInfo& a_createInfo);

    void createBinding(const DescriptorHeap::Handle& a_handle);

    struct BindResourcesDesc {
        ID3D12Resource* input;
        ID3D12Resource* output;
    };
    void bindResources(const BindResourcesDesc& a_bindDesc);
    void uploadWeightsFloat16(DirectX::ResourceUploadBatch& a_uploadBatch, 
                            const std::vector<float>* a_filterWeights, 
                            const std::vector<float>* a_scaleWeights,
                            const std::vector<float>* a_shiftWeights);

    template<typename T>
    void uploadWeights(DirectX::ResourceUploadBatch& a_uploadBatch, 
                     const std::vector<T>* a_filterWeights, 
                     const std::vector<T>* a_scaleWeights,
                     const std::vector<T>* a_shiftWeights) 
    {
        bool useBias = (m_biasWeights->getID3D12Resource() != nullptr);
        assert(!useBias || (a_scaleWeights && a_shiftWeights));

        std::vector<T> filterWeights;
        std::vector<T> biasWeights;

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
                            T scaledWeight = useBias ? (*a_filterWeights)[idx] * (*a_scaleWeights)[n]
                                                     : (*a_filterWeights)[idx];
                            filterWeights.push_back(scaledWeight);
                        }
                break;

            default:
                // Weights are already in the right order
                for (uint32_t i = 0; i < C*H*W; i++)
                {
                    // Apply the scale weight now so we don't need a normalization layer
                    uint32_t idx = n * C*H*W + i;
                    T scaledWeight = useBias ? (*a_filterWeights)[idx] * (*a_scaleWeights)[n]
                                             : (*a_filterWeights)[idx];
                    filterWeights.push_back(scaledWeight);
                }
            }

            if (useBias)
            {
                // Technically this is initialBias*scale+shift, but the initial bias is 0
                biasWeights.push_back((*a_shiftWeights)[n]);
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

    uint64_t getInputTotalSize() {
        return m_inputTotalSize;
    }
    uint64_t getOutputTotalSize() {
        return m_outputTotalSize;
    }

    Buffer* getFilterWeights() {
        return m_filterWeights.get();
    }

    Buffer* getBiasWeights() {
        return m_biasWeights.get();
    }

    Buffer* getPersistentBuffer() {
        return m_persistentResource.get();
    }

    IDMLCompiledOperator* getCompiledOperator() {
        return m_compiledOperator.Get();
    }

    IDMLBindingTable* getBinding() {
        return m_bindingTable.Get();
    }
private:
    DML_BUFFER_TENSOR_DESC getBufferTensorDesc(DML_TENSOR_DATA_TYPE a_dataType,
                                               const std::array<uint32_t, 4>& a_sizes,
                                               std::array<uint32_t, 4>& a_strides);

    static void getStrides(const std::array<uint32_t, 4>& a_sizes, TensorLayout layout, std::array<uint32_t, 4>& a_strides);

    IDMLDevice* m_dmlDevice;
    ComPtr<IDMLCompiledOperator> m_compiledOperator;
    ComPtr<IDMLBindingTable>     m_bindingTable;
    TensorLayout m_tensorLayout = TensorLayout::Default;
    std::unique_ptr<Buffer> m_filterWeights = nullptr;
    std::unique_ptr<Buffer> m_biasWeights = nullptr;
    std::unique_ptr<Buffer> m_persistentResource = nullptr;
    std::unique_ptr<Buffer> m_temporaryResource = nullptr;
    std::array<uint32_t, 4> m_filterSizes;
    uint64_t m_inputTotalSize;
    uint64_t m_outputTotalSize;
};

class ConvolutionLayersContainer {
public:
    void initialize(ID3D12Device* a_device, IDMLDevice* a_dmlDevice, uint32_t a_layersCount) {
        m_device = a_device;
        m_dmlDevice = a_dmlDevice;
        m_layers.resize(a_layersCount);
    }

    void createOperatorInitializer();
    uint32_t getDescriptorCount();
    void createOperatorInitializerBinding(DirectX::DescriptorHeap* a_descriptorHeap, uint32_t a_startIndex);

    void getInitializerBindings() {

    }

    void getInitializerBindings(std::shared_ptr<DirectX::DescriptorHeap>  a_descriptorHeap, uint32_t a_startIndex) 
    {
        m_descriptorHeapStartIndex = a_startIndex;
        m_descriptorHeap = a_descriptorHeap;
    }

    ConvolutionLayer& operator [](int idx) {
        return m_layers[idx];
    }

    size_t size() {
        return m_layers.size();
    }
    // ConvolutionLayer operator [](int idx) const {
    //     return m_layers[idx];
    // }
    IDMLOperatorInitializer* getInitializer() {
        return m_operatorInitializer.Get();
    }
    IDMLBindingTable* getInitializerBinding() {
        return m_initBindingTable.Get();
    }
private:
    ID3D12Device*                             m_device;
    IDMLDevice*                               m_dmlDevice;

    std::vector<ConvolutionLayer>             m_layers;
    ComPtr<IDMLOperatorInitializer>           m_operatorInitializer;
    std::unique_ptr<Buffer>                   m_operatorInitializerTemporaryResource;
    ComPtr<IDMLBindingTable>                  m_initBindingTable;

    int32_t                                   m_descriptorHeapStartIndex = -1;
    std::shared_ptr<DirectX::DescriptorHeap>  m_descriptorHeap;
    uint32_t                                  m_descriptorCount = 0;
};
}