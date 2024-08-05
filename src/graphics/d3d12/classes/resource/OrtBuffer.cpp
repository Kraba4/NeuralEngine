#include "OrtBuffer.h"
#include <onnxruntime/include/dml_provider_factory.h>

namespace neural::graphics {
uint32_t OrtBuffer::getSizeOfOnnxDataType(ONNXTensorElementDataType a_dataType) {
    switch (a_dataType) {
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT8:
            return sizeof(uint8_t);
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8:
            return sizeof(int8_t);

        case ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT16:
            return sizeof(uint16_t);
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT16:
            return sizeof(int16_t);
            
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT32:
            return sizeof(uint32_t);
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32:
            return sizeof(int32_t);

        case ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT64:
            return sizeof(uint64_t);
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64:
            return sizeof(int64_t);

        case ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16:
            return sizeof(uint16_t);
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT:
            return sizeof(float);
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_DOUBLE:
            return sizeof(double);

        default:
            assert(false);  // type not supported
            return 0;
    }
}
void OrtBuffer::initialize(ID3D12Device* a_device, DescriptorHeap* a_srvUavHeap,
                            const OrtBufferCreateInfo& a_createInfo) 
{
    // Create DirectX Resource
    uint32_t dataSize = 1;
    for (uint32_t dimension : a_createInfo.dimensions) {
        dataSize *= dimension;
    }
    m_buffer.initialize(a_device, a_srvUavHeap, {
        .size = dataSize,
        .elementSize = getSizeOfOnnxDataType(a_createInfo.dataType),
        .usageFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        .initialState = D3D12_RESOURCE_STATE_COPY_DEST
    });

    // Get Api 
    const OrtApi* ortApi = &Ort::GetApi();
    const OrtDmlApi* ortDmlApi;
    ORT_CALL(ortApi->GetExecutionProviderApi("DML", ORT_API_VERSION, reinterpret_cast<const void **>(&ortDmlApi)));

    // Create Dml Resource
    ortDmlApi->CreateGPUAllocationFromD3DResource(m_buffer.getID3D12Resource(), &m_dmlResource);
    
    // Create Ort Value (Tensor)
    OrtValue* ortValue;
    Ort::MemoryInfo memoryInfo("DML", OrtAllocatorType::OrtDeviceAllocator, 0, OrtMemType::OrtMemTypeDefault);
    ORT_CALL(ortApi->CreateTensorWithDataAsOrtValue(memoryInfo, m_dmlResource.Get(), dataSize,
                                                    a_createInfo.dimensions.data(), a_createInfo.dimensions.size(),
                                                    a_createInfo.dataType, &ortValue));
    m_ortValue = std::unique_ptr<OrtValue>(ortValue);
};
}
