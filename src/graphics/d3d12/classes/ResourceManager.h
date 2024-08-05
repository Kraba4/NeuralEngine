#pragma once
#include "DescriptorHeap.h"
#include "resource/BufferAndTexture.h"
#include "resource/ConstantBuffer.h"
#include "resource/OrtBuffer.h"
#include <graphics/d3d12/CommonGraphicsHeaders.h>

#include <unordered_map>

namespace neural::graphics {

class ResourceManager {
public:
    void initialize(ID3D12Device* a_device, uint32_t a_nFrames,
        uint32_t a_rtvHeapSize, uint32_t a_dsvHeapSize, uint32_t a_cbvHeapSize);

    OrtBuffer& createOrtBufferInFrame(std::string a_name, uint32_t a_frame, 
                                        const OrtBufferCreateInfo& a_createInfo);
    Ort::IoBinding& createOrtBindingInFrame(std::string a_name, uint32_t a_frame,
                                             Ort::Session* a_session,
                                             const OrtBuffer& a_input, const OrtBuffer& a_output);
    ConstantBuffer& createConstantBufferInFrame(std::string a_name, uint32_t a_frame, 
                                                const ConstantBufferCreateInfo& a_createInfo);
    ConstantBuffer& getConstantBuffer(std::string a_name, uint32_t a_frame) {
        return m_frameResources[a_frame].m_constantBuffers[a_name];
    }
    Buffer& createBufferInUnique(std::string a_name, const BufferCreateInfo& a_createInfo);

    Texture& createTextureInFrame(std::string a_name, uint32_t a_frame, const TextureCreateInfo& a_createInfo);
    Texture& createTextureInFrame(std::string a_name, uint32_t a_frame, ID3D12Resource* a_resource);
    Texture& getTexture(std::string a_name, uint32_t a_frame) {
        return m_frameResources[a_frame].m_textures[a_name];
    }
    Ort::IoBinding& getOrtBinding(std::string a_name, uint32_t a_frame) {
        return m_frameResources[a_frame].m_ortBindings[a_name];
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
    DescriptorHeap m_rtvHeap;
    DescriptorHeap m_dsvHeap;
    DescriptorHeap m_cbvHeap;
    ID3D12Device* m_device;

    struct Resources {
        std::unordered_map<std::string, ConstantBuffer> m_constantBuffers;
        std::unordered_map<std::string, Buffer> m_buffers;
        std::unordered_map<std::string, Texture> m_textures;
        std::unordered_map<std::string, OrtBuffer> m_ortBuffers;
        std::unordered_map<std::string, Ort::IoBinding> m_ortBindings;
    };
    std::unique_ptr<Resources[]> m_frameResources;
    Resources m_uniqueResources;
};
}