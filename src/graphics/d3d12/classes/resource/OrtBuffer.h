#pragma once
#include "BufferAndTexture.h"
#include "DescriptorHeap.h"
#include <graphics/d3d12/CommonGraphicsHeaders.h>

#include <onnxruntime/include/onnxruntime_cxx_api.h>

#include <vector>
#include <memory>

namespace neural::graphics {
struct OrtBufferCreateInfo {
    ONNXTensorElementDataType  dataType = ONNX_TENSOR_ELEMENT_DATA_TYPE_UNDEFINED;
    std::vector<int64_t> dimensions;
};
class OrtBuffer {
public:
    void initialize(ID3D12Device* a_device, DescriptorHeap* a_srvUavHeap, const OrtBufferCreateInfo& a_createInfo);

    DescriptorHeap::Handle& getUnorderedAccessView() { return m_buffer.getUnorderedAccessView(); }
    OrtValue* getOrtValue() const { return m_ortValue.get(); }
    ID3D12Resource* getID3D12Resource() { return m_buffer.getID3D12Resource(); }

private:
    static uint32_t getSizeOfOnnxDataType(ONNXTensorElementDataType a_dataType);
    Buffer m_buffer;
    ComPtr<ID3D12Resource> m_dmlResource;
    std::unique_ptr<OrtValue> m_ortValue;
};
}