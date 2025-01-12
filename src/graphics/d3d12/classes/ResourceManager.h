#pragma once
#include "DescriptorHeap.h"
#include "resource/BufferAndTexture.h"
#include "resource/ConstantBuffer.h"
#include <graphics/d3d12/CommonGraphicsHeaders.h>

#include <unordered_map>

namespace neural::graphics {

class ResourceManager {
public:
    void initialize(ID3D12Device* a_device, uint32_t a_nFrames,
        uint32_t a_rtvHeapSize, uint32_t a_dsvHeapSize, uint32_t a_cbvHeapSize);

    void resizeUniqueResources(int a_numUniqueConstantBuffers, int a_numUniqueBuffers, int a_numUniqueTextures) {
        m_uniqueResources.m_constantBuffers.resize(a_numUniqueConstantBuffers);
        m_uniqueResources.m_buffers.resize(a_numUniqueBuffers);
        m_uniqueResources.m_textures.resize(a_numUniqueTextures);
    }
    void resizeFrameResources(int a_frameIndex,
                              int a_numFrameConstantBuffers, int a_numFrameBuffers, int a_numFrameTextures) {
        m_frameResources[a_frameIndex].m_constantBuffers.resize(a_numFrameConstantBuffers);
        m_frameResources[a_frameIndex].m_buffers.resize(a_numFrameBuffers);
        m_frameResources[a_frameIndex].m_textures.resize(a_numFrameTextures);
    }
    ConstantBuffer& createConstantBufferInFrame(int a_bufferId, std::string a_debugName, uint32_t a_frame, 
                                                const ConstantBufferCreateInfo& a_createInfo);
    ConstantBuffer& getConstantBuffer(int a_bufferId, uint32_t a_frame) {
        assert(a_bufferId < m_frameResources[a_frame].m_constantBuffers.size());
        return m_frameResources[a_frame].m_constantBuffers[a_bufferId];
    }
    Buffer& createBufferInUnique(int a_bufferId, std::string a_debugName, const BufferCreateInfo& a_createInfo);
    Buffer& createBufferInFrame(int a_bufferId, std::string a_debugName, uint32_t a_frame, const BufferCreateInfo& a_createInfo);
    Buffer& getBuffer(int a_bufferId, uint32_t a_frame) {
        assert(a_bufferId < m_frameResources[a_frame].m_buffers.size());
        return m_frameResources[a_frame].m_buffers[a_bufferId];
    }
    Buffer& getUniqueBuffer(int a_bufferId) {
        assert(a_bufferId < m_uniqueResources.m_buffers.size());
        return m_uniqueResources.m_buffers[a_bufferId];
    }
    Texture& createTextureInUnique(int a_textureId, std::string a_debugName, const TextureCreateInfo& a_createInfo);
    Texture& createTextureInFrame(int a_textureId, std::string a_debugName, uint32_t a_frame, const TextureCreateInfo& a_createInfo);
    Texture& createTextureInFrame(int a_textureId, std::string a_debugName, uint32_t a_frame, ID3D12Resource* a_resource);
    Texture& getTexture(int a_textureId, uint32_t a_frame) {
        assert(a_textureId < m_frameResources[a_frame].m_textures.size());
        return m_frameResources[a_frame].m_textures[a_textureId];
    }
    Texture& getUniqueTexture(int a_textureId) {
        assert(a_textureId < m_uniqueResources.m_textures.size());
        return m_uniqueResources.m_textures[a_textureId];
    }
    ID3D12Device* getDevice() const {
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
    ID3D12Device* getDevice() {
        return m_device;
    }
private:
    DescriptorHeap m_rtvHeap;
    DescriptorHeap m_dsvHeap;
    DescriptorHeap m_cbvHeap;
    ID3D12Device* m_device;

    struct Resources {
        std::vector<ConstantBuffer> m_constantBuffers;
        std::vector<Buffer> m_buffers;
        std::vector<Texture> m_textures;
    };
    std::unique_ptr<Resources[]> m_frameResources;
    Resources m_uniqueResources;
};
}