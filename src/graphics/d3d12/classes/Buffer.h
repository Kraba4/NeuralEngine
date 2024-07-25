#pragma once
#include "ResourceManager.h"
#include "DescriptorHeap.h"
#include <graphics/d3d12/CommonGraphicsHeaders.h>

#include <unordered_map>

namespace neural::graphics {
class ResourceManager;

class ConstantBuffer {
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
    ~ConstantBuffer() {
        if (m_resource != nullptr) {
            m_resource->Unmap(0, nullptr);
            m_mappedData = nullptr;
            //m_pResourceManager->getCBVHeap()->deferredDeallocate(m_viewHandle);
        }
    }
    DescriptorHeap::Handle getCBV() {
        return m_viewHandle;
    }
    ConstantBuffer() {}; // only ResourceManager can create it
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

class Buffer {
public:
    enum ViewType {
        SHADER_RESOURCE_VIEW,
        UNORDERED_ACCESS_VIEW
    };
    struct ViewParams {
        ViewType viewType;
        bool operator==(const ViewParams& b) const = default;
    };
    struct ViewParamsHasher
    {
        size_t operator()(ViewParams params) const
        {
            uint32_t hash = 0;
            hashPack(hash, params.viewType);
            return hash;
        }
    private:
        template<typename HashT, typename... HashTs>
        inline void hashPack(uint32_t& hash, const HashT& first, HashTs&&... other) const
        {
            auto hasher = std::hash<uint32_t>();
            hash ^= hasher(first) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            (hashPack(hash, std::forward<HashTs>(other)), ...);
        }
    };

    //void uploadData(int a_elementIndex, const void* a_data) {
    //    assert(m_resource);
    //    assert(m_mappedData);
    //    assert(a_elementIndex * m_elementSize < m_size * m_elementSize);
    //    memcpy(&m_mappedData[a_elementIndex * m_elementSize], &a_data, m_elementSize);
    //}
    void mapData() {
        m_resource->Map(0, nullptr, &m_mappedData);
    }
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
    DescriptorHeap::Handle& getShaderResourceView() {
        ViewParams viewParams = { SHADER_RESOURCE_VIEW };
        auto it = m_views.find(viewParams);
        if (it != m_views.end()) {
            return it->second;
        }
        else {
            auto handle = m_pResourceManager->getCBVHeap()->allocate();
            m_pResourceManager->getDevice()->CreateShaderResourceView(m_resource.Get(), nullptr, handle.cpu);
            m_views[viewParams] = handle;
            return handle;
        }
    }
    //DescriptorHeap::Handle& getShaderResourceView(D3D12_BUFFER_SRV bufferDesc) {
    //    D3D12_SHADER_RESOURCE_VIEW_DESC desc;
    //    desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    //    desc.Buffer = bufferDesc;
    //    desc.Shader4ComponentMapping 
    //}

    DescriptorHeap::Handle& getUnorderedAccessView() {
        ViewParams viewParams = { UNORDERED_ACCESS_VIEW };
        auto it = m_views.find(viewParams);
        if (it != m_views.end()) {
            return it->second;
        }
        else {
            auto handle = m_pResourceManager->getCBVHeap()->allocate();
            m_pResourceManager->getDevice()->CreateUnorderedAccessView(m_resource.Get(), 
                                                                       nullptr, nullptr, handle.cpu);
            m_views[viewParams] = handle;
            return handle;
        }
    }

    //DescriptorHeap::Handle& getShaderResourceView(D3D12_BUFFER_UAV bufferDesc) {
    //    
    //}
    D3D12_VERTEX_BUFFER_VIEW getVertexBufferView() const {
        D3D12_VERTEX_BUFFER_VIEW vertexInputBufferView;
        vertexInputBufferView.BufferLocation = m_resource.Get()->GetGPUVirtualAddress();
        vertexInputBufferView.SizeInBytes = m_size * m_elementSize;
        vertexInputBufferView.StrideInBytes = m_elementSize;
        return vertexInputBufferView;
    }

    D3D12_INDEX_BUFFER_VIEW getIndexBufferView() const {
        assert(sizeof(uint32_t) == m_elementSize);
        D3D12_INDEX_BUFFER_VIEW indexBufferView;
        indexBufferView.BufferLocation = m_resource.Get()->GetGPUVirtualAddress();
        indexBufferView.SizeInBytes = m_size * sizeof(uint32_t);
        indexBufferView.Format = DXGI_FORMAT_R32_UINT;
        return indexBufferView;
    }
    ~Buffer() {
        if (m_resource != nullptr) {
            m_resource->Unmap(0, nullptr);
            m_mappedData = nullptr;
            //m_pResourceManager->getCBVHeap()->deferredDeallocate(m_viewHandle);
        }
    }
   
    Buffer() {}; // only ResourceManager can create it

private:
    ComPtr<ID3D12Resource> m_resource;
    std::unordered_map<ViewParams, DescriptorHeap::Handle, ViewParamsHasher> m_views;

    uint32_t m_size;
    uint32_t m_elementSize;
    void* m_mappedData;

    ResourceManager* m_pResourceManager = nullptr;
    friend class ResourceManager;
};
}