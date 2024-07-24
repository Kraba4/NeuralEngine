#include "ResourceManager.h"
#include "ConstantBuffer.h"
#include "Texture.h"

#include <utils/Macros.h>

namespace neural::graphics {
void ResourceManager::initialize(ID3D12Device* a_device, uint32_t a_nFrames,
    uint32_t a_rtvHeapSize, uint32_t a_dsvHeapSize, uint32_t a_cbvHeapSize)
{
    assert(a_device);
    assert(a_rtvHeapSize > 0);
    assert(a_dsvHeapSize > 0);
    assert(a_cbvHeapSize > 0);
    assert(a_nFrames > 0);

    m_rtvHeap.initialize(a_device, a_nFrames, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, a_rtvHeapSize, false);
    m_dsvHeap.initialize(a_device, a_nFrames, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, a_dsvHeapSize, false);
    m_cbvHeap.initialize(a_device, a_nFrames, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, a_cbvHeapSize, true);
    m_device = a_device;

    m_frameResources = std::move(std::make_unique<FrameResources[]>(a_nFrames));
}

void ResourceManager::createConstantBufferInFrame(std::string a_name, uint32_t a_frame, BufferCreateInfo a_createInfo)
{
    assert(m_device);
    assert(!m_frameResources[a_frame].m_constantBuffers.contains(a_name)); // name is already taken
    assert(a_createInfo.size > 0);
    assert(a_createInfo.elementSize > 0);

    m_frameResources[a_frame].m_constantBuffers[a_name] = {};
    ConstantBufferAlpha& constantBuffer = m_frameResources[a_frame].m_constantBuffers[a_name];
    constantBuffer.m_pResourceManager = this;
    constantBuffer.m_size = a_createInfo.size;
    constantBuffer.m_elementSize = ConstantBufferAlpha::CalcConstantBufferByteSize(a_createInfo.elementSize);
 
    auto desc = CD3DX12_RESOURCE_DESC::Buffer(a_createInfo.size * a_createInfo.elementSize,
                                              a_createInfo.usageFlags);
    createResource(&constantBuffer.m_resource, {
        .resourceDesc = &desc,
        .initialState = D3D12_RESOURCE_STATE_GENERIC_READ,
        .clearValue = nullptr,
        .heapInfo = &a_createInfo.heapInfo,
        .heapType = D3D12_HEAP_TYPE_UPLOAD,
        });

    constantBuffer.m_resource->Map(0, nullptr, &constantBuffer.m_mappedData);

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
    cbvDesc.BufferLocation = constantBuffer.m_resource->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = constantBuffer.m_elementSize;

    constantBuffer.m_viewHandle = m_cbvHeap.allocate();
    m_device->CreateConstantBufferView(&cbvDesc, constantBuffer.m_viewHandle.cpu);

    std::wstring wName(a_name.begin(), a_name.end());
    NAME_DX_OBJECT_INDEXED(constantBuffer.m_resource, wName, a_frame);
}
void ResourceManager::createStructuredBuffer(std::string a_name, BufferCreateInfo a_createInfo)
{
}
void ResourceManager::createTextureInFrame(std::string a_name, uint32_t a_frame, TextureCreateInfo a_createInfo)
{
    assert(m_device);
    assert(!m_frameResources[a_frame].m_constantBuffers.contains(a_name)); // name is already taken
    assert(a_createInfo.width > 0);
    assert(!(a_createInfo.dimension == TextureDimension::D3D12_RESOURCE_DIMENSION_TEXTURE1D &&
            ((a_createInfo.height != 1))) );
    assert(!(a_createInfo.dimension == TextureDimension::D3D12_RESOURCE_DIMENSION_TEXTURE2D &&
            ((a_createInfo.height <= 0))) );
    assert(!(a_createInfo.dimension == TextureDimension::D3D12_RESOURCE_DIMENSION_TEXTURE3D &&
            ((a_createInfo.height <= 0) || (a_createInfo.depthOrArraySize <= 0))));
    assert(a_createInfo.mipLevels >= 1);
    
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

    m_frameResources[a_frame].m_textures[a_name] = {};
    Texture& texture = m_frameResources[a_frame].m_textures[a_name];
    texture.m_pResourceManager = this;
    texture.m_width = a_createInfo.width;
    texture.m_height = a_createInfo.height;
    texture.m_depthOrArraySize = a_createInfo.depthOrArraySize;
    texture.m_mipLevels = a_createInfo.mipLevels;
    texture.m_format = a_createInfo.format;
    texture.m_dimension = static_cast<D3D12_RESOURCE_DIMENSION>(a_createInfo.dimension);

    createResource(&texture.m_resource, {
        .resourceDesc = &desc,
        .initialState = a_createInfo.initialState,
        .clearValue = a_createInfo.clearValue.has_value() ? &a_createInfo.clearValue.value() : nullptr,
        .heapInfo = &a_createInfo.placementHeap,
        .heapType = a_createInfo.heapType,
        });

    std::wstring wName(a_name.begin(), a_name.end());
    NAME_DX_OBJECT_INDEXED(texture.m_resource, wName, a_frame);
}
void ResourceManager::createTextureInFrame(std::string a_name, uint32_t a_frame, ID3D12Resource* a_resource)
{
    assert(m_device);
    assert(!m_frameResources[a_frame].m_constantBuffers.contains(a_name)); // name is already taken

    m_frameResources[a_frame].m_textures[a_name] = {};
    Texture& texture = m_frameResources[a_frame].m_textures[a_name];
    D3D12_RESOURCE_DESC desc = a_resource->GetDesc();
    texture.m_pResourceManager = this;
    texture.m_width = desc.Width;
    texture.m_height = desc.Height;
    texture.m_depthOrArraySize = desc.DepthOrArraySize;
    texture.m_mipLevels = desc.MipLevels;
    texture.m_format = desc.Format;
    texture.m_dimension = desc.Dimension;
    texture.m_resource = a_resource;

    std::wstring wName(a_name.begin(), a_name.end());
    NAME_DX_OBJECT_INDEXED(texture.m_resource, wName, a_frame);
}
void ResourceManager::createResource(ID3D12Resource** a_resource, ResourceCreateInfo a_createInfo)
{
    if (a_createInfo.heapInfo && a_createInfo.heapInfo->heap) {
        DX_CALL(m_device->CreatePlacedResource(
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
        DX_CALL(m_device->CreateCommittedResource(
            &defaultHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            a_createInfo.resourceDesc,
            a_createInfo.initialState,
            a_createInfo.clearValue,
            IID_PPV_ARGS(a_resource)));
    }
}
}