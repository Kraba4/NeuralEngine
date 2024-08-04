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
}

ConstantBuffer& ResourceManager::createConstantBufferInFrame(std::string a_name, uint32_t a_frame, const ConstantBufferCreateInfo& a_createInfo)
{
    assert(!m_frameResources[a_frame].m_constantBuffers.contains(a_name)); // name is already taken

    m_frameResources[a_frame].m_constantBuffers[a_name] = {};
    ConstantBuffer& constantBuffer = m_frameResources[a_frame].m_constantBuffers[a_name];
    constantBuffer.initialize(m_device, a_createInfo, &m_cbvHeap);

    std::wstring wName(a_name.begin(), a_name.end());
    NAME_DX_OBJECT_INDEXED(constantBuffer.m_resource, wName, a_frame);
    return constantBuffer;
}
Buffer& ResourceManager::createBufferInUnique(std::string a_name, const BufferCreateInfo& a_createInfo)
{
    assert(!m_uniqueResources.m_buffers.contains(a_name)); // name is already taken

    m_uniqueResources.m_buffers[a_name] = {};
    Buffer& buffer = m_uniqueResources.m_buffers[a_name];
    buffer.initialize(m_device, a_createInfo, &m_cbvHeap);

    std::wstring wName(a_name.begin(), a_name.end());
    NAME_DX_OBJECT(buffer.m_resource, wName);
    
    return buffer;
}
Texture& ResourceManager::createTextureInFrame(std::string a_name, uint32_t a_frame, const TextureCreateInfo& a_createInfo)
{
    assert(!m_frameResources[a_frame].m_constantBuffers.contains(a_name)); // name is already taken

    m_frameResources[a_frame].m_textures[a_name] = {};
    Texture& texture = m_frameResources[a_frame].m_textures[a_name];
    texture.initialize(m_device, a_createInfo, &m_rtvHeap, &m_dsvHeap, &m_cbvHeap);

    std::wstring wName(a_name.begin(), a_name.end());
    NAME_DX_OBJECT_INDEXED(texture.m_resource, wName, a_frame);

    return texture;
}
Texture& ResourceManager::createTextureInFrame(std::string a_name, uint32_t a_frame, ID3D12Resource* a_resource)
{
    assert(m_device);
    assert(!m_frameResources[a_frame].m_constantBuffers.contains(a_name)); // name is already taken

    m_frameResources[a_frame].m_textures[a_name] = {};
    Texture& texture = m_frameResources[a_frame].m_textures[a_name];
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

    std::wstring wName(a_name.begin(), a_name.end());
    NAME_DX_OBJECT_INDEXED(texture.m_resource, wName, a_frame);

    return texture;
}

}