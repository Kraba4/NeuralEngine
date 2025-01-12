#pragma once
#include "BufferAndTexture.h"
#include "../ResourceManager.h"
#include "../../CommonGraphicsHeaders.h"

#include <optional>

namespace neural::graphics {
class LightGrid {
    D3D12_VIEWPORT m_viewport;
    D3D12_RECT m_scissor;
    std::optional<DescriptorHeap::Handle> m_radianceSrv;
    std::optional<DescriptorHeap::Handle> m_irradianceSrv;
    int m_numCubes;
    int m_faceSize;
    DXGI_FORMAT m_format;
    ResourceManager* m_resourceManager;
    int m_cubemapsTextureId; // texture atlas of all cubemaps
    int m_cubemapsDepthTextureId; // depth texture atlas of all cubemaps
    int m_irradianceTextureId; // irradiance texture atlas of all cubemaps
public:
    void initialize(int a_cubeFaceSize, int a_numCubes, DXGI_FORMAT a_format,
                    ResourceManager& a_resourceManager, int a_textureId, int a_depthTextureId, int a_irradianceTextureId);
    DescriptorHeap::Handle getRadianceSRV();
    DescriptorHeap::Handle getRadianceRTV();
    DescriptorHeap::Handle getIrradianceSRV();
    DescriptorHeap::Handle getIrradianceRTV();
    DescriptorHeap::Handle getRadianceDSV();
    const D3D12_VIEWPORT& getViewport() { return m_viewport; };
    const D3D12_RECT& getScissor() { return m_scissor; };
    Texture& getRadianceTexture() { return m_resourceManager->getUniqueTexture(m_cubemapsTextureId);}
    Texture& getIrradianceTexture() { return m_resourceManager->getUniqueTexture(m_irradianceTextureId);}
    Texture& getDepthTexture() { return m_resourceManager->getUniqueTexture(m_cubemapsDepthTextureId);}
    int getFaceSize() { return m_faceSize; }
};
}