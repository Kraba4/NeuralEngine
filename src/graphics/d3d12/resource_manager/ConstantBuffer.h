#pragma once
#include "ResourceManager.h"
#include <graphics/d3d12/DescriptorHeap.h>

#include <unordered_map>

namespace neural::graphics {
class ResourceManager;

class ConstantBufferAlpha {
    ResourceManager* m_pResourceManager = nullptr;
    friend class ResourceManager;
public:
    //void uploadData(int a_elementIndex, const void* a_data) {
    //    assert(m_resource);
    //    assert(m_mappedData);
    //    assert(a_elementIndex * m_elementSize < m_size * m_elementSize);
    //    memcpy(&m_mappedData[a_elementIndex * m_elementSize], &a_data, m_elementSize);
    //}

    void uploadData(void* a_data) {
        assert(m_resource);
        assert(a_data);
        assert(m_mappedData);
        memcpy(m_mappedData, a_data, m_size * m_elementSize);
    }

    ID3D12Resource* getID3D12Resource() const {
        assert(m_resource);
        return m_resource.Get();
    }
    DescriptorHeap::Handle& getConstantBufferView() {
        assert(m_resource);
        return m_viewHandle;
    }
    ~ConstantBufferAlpha() {
        if (m_resource != nullptr) {
            m_resource->Unmap(0, nullptr);
            m_mappedData = nullptr;
            m_pResourceManager->getCBVHeap()->deferredDeallocate(m_viewHandle);
        }
    }
    DescriptorHeap::Handle getCBV() {
        return m_viewHandle;
    }
    ConstantBufferAlpha() {}; // only ResourceManager can create it
private:

    static uint32_t CalcConstantBufferByteSize(uint32_t a_byteSize) {
        return (a_byteSize + 255) & ~255;
    }

    ComPtr<ID3D12Resource> m_resource;
    DescriptorHeap::Handle m_viewHandle;

    uint32_t m_size;
    uint32_t m_elementSize;
    void* m_mappedData;
};
}