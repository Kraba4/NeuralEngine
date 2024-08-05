#pragma once
#include "BufferAndTexture.h"
#include "../DescriptorHeap.h"
#include <graphics/d3d12/CommonGraphicsHeaders.h>

#include <unordered_map>

namespace neural::graphics {

struct ConstantBufferCreateInfo {
    uint64_t size = 0;
    uint64_t elementSize = 0;
    uint64_t aligment = 0; //??
    D3D12_RESOURCE_FLAGS usageFlags = D3D12_RESOURCE_FLAG_NONE;
    //D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;
    HeapInfo heapInfo = {};
};
class ConstantBuffer : private Buffer {
    friend class ResourceManager;
public:
    //void uploadData(int a_elementIndex, const void* a_data) {
    //    assert(m_resource);
    //    assert(m_mappedData);
    //    assert(a_elementIndex * m_elementSize < m_size * m_elementSize);
    //    memcpy(&m_mappedData[a_elementIndex * m_elementSize], &a_data, m_elementSize);
    //}
    void initialize(ID3D12Device* a_device, DescriptorHeap* a_cbvHeap,
                    const ConstantBufferCreateInfo& a_createInfo);
    void uploadData(void* a_data) {
        Buffer::uploadData(a_data);
    }

    ID3D12Resource* getID3D12Resource() const {
        return Buffer::getID3D12Resource();
    }
    DescriptorHeap::Handle& getConstantBufferView();
private:

    static uint32_t CalcConstantBufferByteSize(uint32_t a_byteSize) {
        return (a_byteSize + 255) & ~255;
    }

    // Buffer m_buffer;
};
}