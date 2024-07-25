#pragma once
#include "DescriptorHeap.h"
#include <graphics/d3d12/CommonGraphicsHeaders.h>

#include <unordered_map>

namespace neural::graphics {
class ConstantBuffer;
class Buffer;
class Texture;

class ResourceManager {
public:
    struct HeapInfo {
        ID3D12Heap* heap = nullptr;
        uint64_t offset = 0;
    };

    struct ConstantBufferCreateInfo {
        uint64_t size = 0;
        uint64_t elementSize = 0;
        uint64_t aligment = 0; //??
        D3D12_RESOURCE_FLAGS usageFlags = D3D12_RESOURCE_FLAG_NONE;
        //D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;
        HeapInfo heapInfo = {};
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

    void initialize(ID3D12Device* a_device, uint32_t a_nFrames,
        uint32_t a_rtvHeapSize, uint32_t a_dsvHeapSize, uint32_t a_cbvHeapSize);

    ConstantBuffer& createConstantBufferInFrame(std::string a_name, uint32_t a_frame, ConstantBufferCreateInfo a_createInfo);
    ConstantBuffer& getConstantBuffer(std::string a_name, uint32_t a_frame) {
        return m_frameResources[a_frame].m_constantBuffers[a_name];
    }
    Buffer& createBufferInUnique(std::string a_name, BufferCreateInfo a_createInfo);

    Texture& createTextureInFrame(std::string a_name, uint32_t a_frame, TextureCreateInfo a_createInfo);
    Texture& createTextureInFrame(std::string a_name, uint32_t a_frame, ID3D12Resource* a_resource);
    Texture& getTexture(std::string a_name, uint32_t a_frame) {
        return m_frameResources[a_frame].m_textures[a_name];
    }
    ID3D12Device* getDevice() {
        return m_device;
    }
    DescriptorHeap* getRTVHeap() {
        return &m_rtvHeap;
    }
    DescriptorHeap* getDSVHeap() {
        return &m_dsvHeap;
    }
    DescriptorHeap* getCBVHeap() {
        return &m_cbvHeap;
    }
private:
    struct ResourceCreateInfo {
        const D3D12_RESOURCE_DESC* resourceDesc;
        D3D12_RESOURCE_STATES initialState;
        const D3D12_CLEAR_VALUE* clearValue = nullptr;
        const HeapInfo* heapInfo = nullptr;
        std::optional<D3D12_HEAP_TYPE> heapType = std::nullopt;
    };
    void createResource(ID3D12Resource** a_resource, ResourceCreateInfo a_createInfo);

    DescriptorHeap m_rtvHeap;
    DescriptorHeap m_dsvHeap;
    DescriptorHeap m_cbvHeap;
    ID3D12Device* m_device;

    struct Resources {
        std::unordered_map<std::string, ConstantBuffer> m_constantBuffers;
        std::unordered_map<std::string, Buffer> m_buffers;
        std::unordered_map<std::string, Texture> m_textures;
    };
    std::unique_ptr<Resources[]> m_frameResources;
    Resources m_uniqueResources;
};
}