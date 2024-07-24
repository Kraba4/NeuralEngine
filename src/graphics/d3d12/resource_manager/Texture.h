#pragma once 
#include "ResourceManager.h"
#include <graphics/d3d12/DescriptorHeap.h>

#include <unordered_map>

namespace neural::graphics {
class Texture {
public:
    ID3D12Resource* getID3D12Resource() const {
        assert(m_resource);
        return m_resource.Get();
    }
    uint64_t getWidth() {
        return m_width;
    }
    uint32_t getHeight() {
        return m_height;
    }
    uint16_t getDepthOrArraySize() {
        return m_depthOrArraySize;
    }
    uint16_t getMipLevels() {
        return m_mipLevels;
    }
    DXGI_FORMAT getFormat() {
        m_format;
    }

    DXGI_FORMAT Format;
    D3D12_RTV_DIMENSION ViewDimension;
    union
    {
        D3D12_BUFFER_RTV Buffer;
        D3D12_TEX1D_RTV Texture1D;
        D3D12_TEX1D_ARRAY_RTV Texture1DArray;
        D3D12_TEX2D_RTV Texture2D;
        D3D12_TEX2D_ARRAY_RTV Texture2DArray;
        D3D12_TEX2DMS_RTV Texture2DMS;
        D3D12_TEX2DMS_ARRAY_RTV Texture2DMSArray;
        D3D12_TEX3D_RTV Texture3D;
    };

    struct ViewParams;
    DescriptorHeap::Handle getRTV() {
        ViewParams viewParams = { D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 0 };
        auto iterator = m_views.find(viewParams);
        if (iterator != m_views.end()) {
            return iterator->second;
        }
        else {
            auto handle = m_pResourceManager->getRTVHeap()->allocate();
            m_pResourceManager->getDevice()->CreateRenderTargetView(m_resource.Get(), nullptr, handle.cpu);
            m_views[viewParams] = handle;
            return handle;
        }
    }
    DescriptorHeap::Handle getRTV(uint32_t a_baseMip) {
        ViewParams viewParams = { D3D12_DESCRIPTOR_HEAP_TYPE_RTV, a_baseMip };
        auto iterator = m_views.find(viewParams);
        if (iterator != m_views.end()) {
            return iterator->second;
        }
        else {
            D3D12_RENDER_TARGET_VIEW_DESC desc;
            desc.Format = m_format;
            switch (m_dimension)
            {
            case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
                if (m_depthOrArraySize > 1) {
                    desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
                    desc.Texture1DArray.MipSlice = viewParams.baseMip;
                    assert(false); // will finish it when needed
                    // desc.Texture1DArray.ArraySize = ..
                    // desc.Texture1DArray.FirstArraySlice = ..
                }
                else {
                    desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
                    desc.Texture1D.MipSlice = viewParams.baseMip;
                }
                break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
                if (m_depthOrArraySize > 1) {
                    desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                    assert(false); // will finish it when needed
                    // ..
                }
                else {
                    desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                    desc.Texture2D.MipSlice = viewParams.baseMip;
                    desc.Texture2D.PlaneSlice = 0; // TODO: maybe will needed
                }
                break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
                desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
                desc.Texture3D.MipSlice = viewParams.baseMip;
                assert(false); // will finish it when needed
                // ..
                break;
            }
            auto handle = m_pResourceManager->getRTVHeap()->allocate();
            m_pResourceManager->getDevice()->CreateRenderTargetView(m_resource.Get(), &desc, handle.cpu);
            m_views[viewParams] = handle;
            return handle;
        }
    }

    DescriptorHeap::Handle getDSV() {
        ViewParams viewParams = { D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 0 }; // DSVParams need
        auto iterator = m_views.find(viewParams);
        if (iterator != m_views.end()) {
            return iterator->second;
        }
        else {
            auto handle = m_pResourceManager->getDSVHeap()->allocate();
            m_pResourceManager->getDevice()->CreateDepthStencilView(m_resource.Get(), nullptr, handle.cpu);
            m_views[viewParams] = handle;
            return handle;
        }
    }

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
        if (m_resource) {
            for (auto& iterator : m_views) {
                m_pResourceManager->getRTVHeap()->deferredDeallocate(iterator.second);
            }
        }
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
    Texture() {};
private:

    std::unordered_map<ViewParams, DescriptorHeap::Handle, ViewParamsHasher> m_views;
    ComPtr<ID3D12Resource> m_resource;
    uint64_t m_width;
    uint32_t m_height;
    uint16_t m_depthOrArraySize;
    uint16_t m_mipLevels;
    D3D12_RESOURCE_DIMENSION m_dimension;
    DXGI_FORMAT m_format;
    ResourceManager* m_pResourceManager = nullptr;
    friend class ResourceManager;
};
}