#include "BufferAndTexture.h"
namespace neural::graphics {

namespace {
void createResource(ID3D12Device* a_device, ID3D12Resource** a_resource, const ResourceCreateInfo& a_createInfo)
{
    if (a_createInfo.heapInfo && a_createInfo.heapInfo->heap) {
        DX_CALL(a_device->CreatePlacedResource(
            a_createInfo.heapInfo->heap,
            a_createInfo.heapInfo->offset,
            a_createInfo.resourceDesc,
            a_createInfo.initialState,
            a_createInfo.clearValue,
            IID_PPV_ARGS(a_resource)));
    }
    else {
        assert(a_createInfo.heapType.has_value());
        auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(a_createInfo.heapType.value());
        DX_CALL(a_device->CreateCommittedResource(
            &defaultHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            a_createInfo.resourceDesc,
            a_createInfo.initialState,
            a_createInfo.clearValue,
            IID_PPV_ARGS(a_resource)));
    }
}
}  // anonymous namespace

#pragma region Buffer
///////////////////////////////          BUFFER          //////////////////////////////////////
void Buffer::initialize(ID3D12Device* a_device, DescriptorHeap* a_srvUavHeap, const BufferCreateInfo& a_createInfo)
{
    assert(a_device);
    assert(a_createInfo.size > 0);
    assert(a_createInfo.elementSize > 0);

    m_device = a_device;
    m_size = a_createInfo.size;
    m_elementSize = a_createInfo.elementSize;
    m_srvUavHeap = a_srvUavHeap;
    
    auto desc = CD3DX12_RESOURCE_DESC::Buffer(a_createInfo.size * a_createInfo.elementSize,
        a_createInfo.usageFlags);
    createResource(a_device, &m_resource, {
        .resourceDesc = &desc,
        .initialState = a_createInfo.initialState,
        .clearValue = nullptr,
        .heapInfo = &a_createInfo.heapInfo,
        .heapType = a_createInfo.heapType,
        });
}
DescriptorHeap::Handle& Buffer::getShaderResourceView() {
    ViewParams viewParams = { SHADER_RESOURCE_VIEW };
    auto it = m_views.find(viewParams);
    if (it != m_views.end()) {
        return it->second;
    }
    else {
        auto handle = m_srvUavHeap->allocate();
        m_device->CreateShaderResourceView(m_resource.Get(), nullptr, handle.cpu);
        m_views[viewParams] = handle;
        return handle;
    }
}
DescriptorHeap::Handle& Buffer::getUnorderedAccessView() {
    ViewParams viewParams = { UNORDERED_ACCESS_VIEW };
    auto it = m_views.find(viewParams);
    if (it != m_views.end()) {
        return it->second;
    }
    else {
        auto handle = m_srvUavHeap->allocate();
        m_device->CreateUnorderedAccessView(m_resource.Get(), nullptr, nullptr, handle.cpu);
        m_views[viewParams] = handle;
        return handle;
    }
}
D3D12_VERTEX_BUFFER_VIEW Buffer::getVertexBufferView() const {
    D3D12_VERTEX_BUFFER_VIEW vertexInputBufferView;
    vertexInputBufferView.BufferLocation = m_resource.Get()->GetGPUVirtualAddress();
    vertexInputBufferView.SizeInBytes = m_size * m_elementSize;
    vertexInputBufferView.StrideInBytes = m_elementSize;
    return vertexInputBufferView;
}
D3D12_INDEX_BUFFER_VIEW Buffer::getIndexBufferView() const {
    assert(sizeof(uint32_t) == m_elementSize);
    D3D12_INDEX_BUFFER_VIEW indexBufferView;
    indexBufferView.BufferLocation = m_resource.Get()->GetGPUVirtualAddress();
    indexBufferView.SizeInBytes = m_size * sizeof(uint32_t);
    indexBufferView.Format = DXGI_FORMAT_R32_UINT;
    return indexBufferView;
}
#pragma endregion

#pragma region Texture
///////////////////////////////          TEXTURE          //////////////////////////////////////
void Texture::initialize(ID3D12Device* a_device, const TextureCreateInfo& a_createInfo,
    DescriptorHeap* a_rtvHeap, DescriptorHeap* a_dsvHeap, DescriptorHeap* a_srvUavHeap)
{
    assert(a_device);
    assert(a_srvUavHeap || a_rtvHeap || a_dsvHeap);
    assert(a_createInfo.width > 0);
    assert(!(a_createInfo.dimension == TextureDimension::D3D12_RESOURCE_DIMENSION_TEXTURE1D &&
            ((a_createInfo.height != 1))) );
    assert(!(a_createInfo.dimension == TextureDimension::D3D12_RESOURCE_DIMENSION_TEXTURE2D &&
            ((a_createInfo.height <= 0))) );
    assert(!(a_createInfo.dimension == TextureDimension::D3D12_RESOURCE_DIMENSION_TEXTURE3D &&
            ((a_createInfo.height <= 0) || (a_createInfo.depthOrArraySize <= 0))));
    assert(a_createInfo.mipLevels >= 1);

    m_device = a_device;
    m_rtvHeap = a_rtvHeap;
    m_dsvHeap = a_dsvHeap;
    m_srvUavHeap = a_srvUavHeap;

    m_width = a_createInfo.width;
    m_height = a_createInfo.height;
    m_depthOrArraySize = a_createInfo.depthOrArraySize;
    m_mipLevels = a_createInfo.mipLevels;
    m_format = a_createInfo.format;
    m_dimension = static_cast<D3D12_RESOURCE_DIMENSION>(a_createInfo.dimension);

    D3D12_RESOURCE_DESC desc = {
                .Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(a_createInfo.dimension),
                .Alignment = a_createInfo.alignment,
                .Width = a_createInfo.width,
                .Height = a_createInfo.height,
                .DepthOrArraySize = a_createInfo.depthOrArraySize,
                .MipLevels = a_createInfo.mipLevels,
                .Format = a_createInfo.format,
                .SampleDesc = a_createInfo.multisampleDesc,
                .Layout = a_createInfo.textureLayout,
                .Flags = a_createInfo.usageFlags
    };
    createResource(m_device, &m_resource, {
        .resourceDesc = &desc,
        .initialState = a_createInfo.initialState,
        .clearValue = a_createInfo.clearValue.has_value() ? &a_createInfo.clearValue.value() : nullptr,
        .heapInfo = &a_createInfo.placementHeap,
        .heapType = a_createInfo.heapType,
        });
}
DescriptorHeap::Handle Texture::getRTV() {
    assert(m_device);
    assert(m_rtvHeap);
    ViewParams viewParams = { D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 0 };
    auto iterator = m_views.find(viewParams);
    if (iterator != m_views.end()) {
        return iterator->second;
    }
    else {
        auto handle = m_rtvHeap->allocate();
        m_device->CreateRenderTargetView(m_resource.Get(), nullptr, handle.cpu);
        m_views[viewParams] = handle;
        return handle;
    }
}
DescriptorHeap::Handle Texture::getRTV(uint32_t a_baseMip) {
    assert(m_device);
    assert(m_rtvHeap);
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
        auto handle = m_rtvHeap->allocate();
        m_device->CreateRenderTargetView(m_resource.Get(), &desc, handle.cpu);
        m_views[viewParams] = handle;
        return handle;
    }
}
DescriptorHeap::Handle Texture::getDSV() {
    assert(m_device);
    assert(m_dsvHeap);
    ViewParams viewParams = { D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 0 }; // DSVParams need
    auto iterator = m_views.find(viewParams);
    if (iterator != m_views.end()) {
        return iterator->second;
    }
    else {
        auto handle = m_dsvHeap->allocate();
        m_device->CreateDepthStencilView(m_resource.Get(), nullptr, handle.cpu);
        m_views[viewParams] = handle;
        return handle;
    }
}
#pragma endregion
}