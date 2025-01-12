#include "ResourceManager.h"

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

    m_frameResources = std::move(std::make_unique<Resources[]>(a_nFrames));

    NAME_DX_OBJECT(m_cbvHeap.getID3D12DescriptorHeap(), L"mainSrvHeap");
}

ConstantBuffer& ResourceManager::createConstantBufferInFrame(int a_bufferId, std::string a_debugName, uint32_t a_frame, const ConstantBufferCreateInfo& a_createInfo)
{
    assert(a_bufferId < m_frameResources[a_frame].m_constantBuffers.size());

    ConstantBuffer& constantBuffer = m_frameResources[a_frame].m_constantBuffers[a_bufferId];
    constantBuffer.initialize(m_device, a_createInfo, &m_cbvHeap);

    std::wstring wName(a_debugName.begin(), a_debugName.end());
    NAME_DX_OBJECT_INDEXED(constantBuffer.m_resource, wName, a_frame);
    return constantBuffer;
}
Buffer& ResourceManager::createBufferInUnique(int a_bufferId, std::string a_debugName, const BufferCreateInfo& a_createInfo)
{
    assert(a_bufferId < m_uniqueResources.m_buffers.size());

    Buffer& buffer = m_uniqueResources.m_buffers[a_bufferId];
    buffer.initialize(m_device, &m_cbvHeap, a_createInfo);

    std::wstring wName(a_debugName.begin(), a_debugName.end());
    NAME_DX_OBJECT(buffer.m_resource, wName);
    
    return buffer;
}
Buffer& ResourceManager::createBufferInFrame(int a_bufferId, std::string a_debugName, uint32_t a_frame, const BufferCreateInfo& a_createInfo)
{
    assert(a_bufferId < m_frameResources[a_frame].m_buffers.size());

    Buffer& buffer = m_frameResources[a_frame].m_buffers[a_bufferId];
    buffer.initialize(m_device, &m_cbvHeap, a_createInfo);

    std::wstring wName(a_debugName.begin(), a_debugName.end());
    NAME_DX_OBJECT(buffer.m_resource, wName);
    
    return buffer;
}
Texture& ResourceManager::createTextureInUnique(int a_textureId, std::string a_debugName, const TextureCreateInfo& a_createInfo)
{
    assert(a_textureId < m_uniqueResources.m_textures.size());

    Texture& texture = m_uniqueResources.m_textures[a_textureId];
    texture.initialize(m_device, a_createInfo, &m_rtvHeap, &m_dsvHeap, &m_cbvHeap);

    std::wstring wName(a_debugName.begin(), a_debugName.end());
    NAME_DX_OBJECT(texture.m_resource, wName);

    return texture;
}

Texture& ResourceManager::createTextureInFrame(int a_textureId, std::string a_debugName, uint32_t a_frame, const TextureCreateInfo& a_createInfo)
{
    assert(a_textureId < m_frameResources[a_frame].m_textures.size());

    Texture& texture = m_frameResources[a_frame].m_textures[a_textureId];
    texture.initialize(m_device, a_createInfo, &m_rtvHeap, &m_dsvHeap, &m_cbvHeap);

    std::wstring wName(a_debugName.begin(), a_debugName.end());
    NAME_DX_OBJECT_INDEXED(texture.m_resource, wName, a_frame);

    return texture;
}
Texture& ResourceManager::createTextureInFrame(int a_textureId, std::string a_debugName, uint32_t a_frame, ID3D12Resource* a_resource)
{
    assert(m_device);
    assert(a_textureId < m_frameResources[a_frame].m_textures.size());

    Texture& texture = m_frameResources[a_frame].m_textures[a_textureId];
    D3D12_RESOURCE_DESC desc = a_resource->GetDesc();
    texture.m_device = m_device;
    texture.m_rtvHeap = &m_rtvHeap;
    texture.m_dsvHeap = &m_dsvHeap;
    texture.m_srvUavHeap = &m_cbvHeap;
    texture.m_width = desc.Width;
    texture.m_height = desc.Height;
    texture.m_depthOrArraySize = desc.DepthOrArraySize;
    texture.m_mipLevels = desc.MipLevels;
    texture.m_format = desc.Format;
    texture.m_dimension = desc.Dimension;
    texture.m_resource = a_resource;

    std::wstring wName(a_debugName.begin(), a_debugName.end());
    NAME_DX_OBJECT_INDEXED(texture.m_resource, wName, a_frame);

    return texture;
}

}