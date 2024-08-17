#pragma once
#include "../DescriptorHeap.h"
#include <graphics/d3d12/CommonGraphicsHeaders.h>

#include <unordered_map>

namespace neural::graphics {
struct HeapInfo {
    ID3D12Heap* heap = nullptr;
    uint64_t offset = 0;
};
struct ResourceCreateInfo {
        const D3D12_RESOURCE_DESC* resourceDesc;
        D3D12_RESOURCE_STATES initialState;
        const D3D12_CLEAR_VALUE* clearValue = nullptr;
        const HeapInfo* heapInfo = nullptr;
        std::optional<D3D12_HEAP_TYPE> heapType = std::nullopt;
};

struct BufferCreateInfo {
    uint64_t size = 0;
    uint64_t elementSize = 0;
    uint64_t aligment = 0; //??
    D3D12_RESOURCE_FLAGS usageFlags = D3D12_RESOURCE_FLAG_NONE;
    D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;
    D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT;
    HeapInfo heapInfo = {};
};
class Buffer {
public:
    enum ViewType {
        SHADER_RESOURCE_VIEW,
        UNORDERED_ACCESS_VIEW,
        CONSTANT_BUFFER_VIEW
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
    public:
    void initialize(ID3D12Device* a_device, DescriptorHeap* a_srvUavHeap, const BufferCreateInfo& a_createInfo);
    void setSrvUavHeap(DescriptorHeap* a_srvUavHeap) {
        m_srvUavHeap = a_srvUavHeap;
    }
    void mapData() {
        m_resource->Map(0, nullptr, &m_mappedData);
    }
    void* getMappedData() {
        return m_mappedData;
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
    DescriptorHeap::Handle& getShaderResourceView();
    //DescriptorHeap::Handle& getShaderResourceView(D3D12_BUFFER_SRV bufferDesc) {
    //    D3D12_SHADER_RESOURCE_VIEW_DESC desc;
    //    desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    //    desc.Buffer = bufferDesc;
    //    desc.Shader4ComponentMapping 
    //}

    DescriptorHeap::Handle& getUnorderedAccessView();

    //DescriptorHeap::Handle& getShaderResourceView(D3D12_BUFFER_UAV bufferDesc) {
    //    
    //}
    D3D12_VERTEX_BUFFER_VIEW getVertexBufferView() const;
    uint32_t getElementSize() const {
        return m_elementSize;
    }
    uint32_t getSize() const {
        return m_size;
    }
    uint32_t getTotalSize() const {
        return m_size * m_elementSize;
    }
    static uint32_t CalcConstantBufferByteSize(uint32_t a_byteSize) {
        return (a_byteSize + 255) & ~255;
    }
    D3D12_INDEX_BUFFER_VIEW getIndexBufferView() const;
    ~Buffer() {
        if (m_resource != nullptr) {
            m_resource->Unmap(0, nullptr);
            m_mappedData = nullptr;
            //m_pResourceManager->getCBVHeap()->deferredDeallocate(m_viewHandle);
        }
    }

protected:
    ComPtr<ID3D12Resource> m_resource;
    std::unordered_map<ViewParams, DescriptorHeap::Handle, ViewParamsHasher> m_views;

    uint32_t m_size;
    uint32_t m_elementSize;
    void* m_mappedData;

    ID3D12Device* m_device;
    DescriptorHeap* m_srvUavHeap;

    friend class ResourceManager;
};


enum TextureDimension {
    D3D12_RESOURCE_DIMENSION_UNKNOWN = 0,
    D3D12_RESOURCE_DIMENSION_TEXTURE1D = 2,
    D3D12_RESOURCE_DIMENSION_TEXTURE2D = 3,
    D3D12_RESOURCE_DIMENSION_TEXTURE3D = 4
};

struct TextureCreateInfo {
    TextureDimension dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
    uint64_t alignment = 0; //??
    uint64_t width = 0;
    uint32_t height = 1;
    uint16_t depthOrArraySize = 1;
    uint16_t mipLevels = 1;
    std::optional<D3D12_CLEAR_VALUE> clearValue = std::nullopt;
    DXGI_SAMPLE_DESC multisampleDesc = { .Count = 1, .Quality = 0 };
    D3D12_TEXTURE_LAYOUT textureLayout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    D3D12_RESOURCE_FLAGS usageFlags = D3D12_RESOURCE_FLAG_NONE;
    D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;
    D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT;
    HeapInfo placementHeap = {};
};
class Texture {
public:
    void initialize(ID3D12Device* a_device, const TextureCreateInfo& a_createInfo,
     DescriptorHeap* a_rtvHeap, DescriptorHeap* a_dsvHeap, DescriptorHeap* a_srvUavHeap);
    ID3D12Resource* getID3D12Resource() const {
        assert(m_resource);
        return m_resource.Get();
    }
    uint64_t getWidth() const {
        return m_width;
    }
    uint32_t getHeight() const {
        return m_height;
    }
    uint16_t getDepthOrArraySize() const {
        return m_depthOrArraySize;
    }
    uint16_t getMipLevels() const {
        return m_mipLevels;
    }
    DXGI_FORMAT getFormat() const {
        return m_format;
    }

    struct ViewParams;
    DescriptorHeap::Handle getRTV();
    DescriptorHeap::Handle getRTV(uint32_t a_baseMip);
    DescriptorHeap::Handle getDSV();

    //DescriptorHeap::Handle getDSV(DSVParams) {
    //    ViewParams viewParams = { D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 0 };
    //    auto iterator = m_views.find(viewParams);
    //    if (iterator != m_views.end()) {
    //        return iterator->second;
    //    }
    //    else {
    //        D3D12_DEPTH_STENCIL_VIEW_DESC desc;
    //        desc.Format = m_format;
    //        assert(false); // will finish it when needed
    //        // ..
    //        auto handle = m_pResourceManager->getDSVHeap()->allocate();
    //        m_pResourceManager->getDevice()->CreateDepthStencilView(m_resource.Get(), nullptr, handle.cpu);
    //        m_views[viewParams] = handle;
    //        return handle;
    //    }
    //}

    ~Texture() {
        //if (m_resource) {
        //    for (auto& iterator : m_views) {
        //        m_pResourceManager->getRTVHeap()->deferredDeallocate(iterator.second);
        //    }
        //}
    }

    struct ViewParams {
        D3D12_DESCRIPTOR_HEAP_TYPE viewType;
        uint32_t baseMip;
        bool operator==(const ViewParams& b) const = default;
    };
    struct ViewParamsHasher
    {
        size_t operator()(ViewParams params) const
        {
            uint32_t hash = 0;
            hashPack(hash, params.viewType, params.baseMip);
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

private:

    std::unordered_map<ViewParams, DescriptorHeap::Handle, ViewParamsHasher> m_views;
    ComPtr<ID3D12Resource> m_resource;
    uint64_t m_width;
    uint32_t m_height;
    uint16_t m_depthOrArraySize;
    uint16_t m_mipLevels;
    D3D12_RESOURCE_DIMENSION m_dimension;
    DXGI_FORMAT m_format;
    
    ID3D12Device* m_device;
    DescriptorHeap* m_rtvHeap;
    DescriptorHeap* m_dsvHeap;
    DescriptorHeap* m_srvUavHeap;

    friend class ResourceManager;
};
}