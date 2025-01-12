#include "LightGrid.h"

namespace neural::graphics {
void LightGrid::initialize(int a_cubeFaceSize, int a_numCubes, DXGI_FORMAT a_format,
                           ResourceManager& a_resourceManager, int a_textureId, int a_depthTextureId,
                           int a_irradianceTextureId) {
    m_resourceManager = &a_resourceManager;
    m_format = a_format;
    m_numCubes = a_numCubes;
    m_cubemapsTextureId = a_textureId;
    m_cubemapsDepthTextureId = a_depthTextureId;
    m_irradianceTextureId = a_irradianceTextureId;
    m_faceSize = a_cubeFaceSize;
    
    constexpr int numCubeSides = 6;
    m_resourceManager->createTextureInUnique(a_textureId, "Light Grid cubemaps radiance", {
        .format = m_format,
        .width = (uint64_t)a_cubeFaceSize,
        .height = (uint32_t)a_cubeFaceSize,
        .depthOrArraySize = static_cast<uint16_t>(numCubeSides * (m_numCubes)),
        .clearValue = D3D12_CLEAR_VALUE {
            .Format = m_format,
            .Color = {0, 0, 0, 1}
        },
        .usageFlags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
        .initialState = D3D12_RESOURCE_STATE_RENDER_TARGET,
    });

     m_resourceManager->createTextureInUnique(m_irradianceTextureId, "Light Grid cubemaps irradiance", {
        .format = m_format,
        .width = (uint64_t)a_cubeFaceSize,
        .height = (uint32_t)a_cubeFaceSize,
        .depthOrArraySize = static_cast<uint16_t>(numCubeSides * (m_numCubes)),
        .clearValue = D3D12_CLEAR_VALUE {
            .Format = m_format,
            .Color = {0, 0, 0, 1}
        },
        .usageFlags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
        .initialState = D3D12_RESOURCE_STATE_RENDER_TARGET,
    });

    m_resourceManager->createTextureInUnique(a_depthTextureId, "Light Grid cubemaps depth", {
        .format = DXGI_FORMAT_D32_FLOAT,
        .width = (uint64_t)a_cubeFaceSize,
        .height = (uint32_t)a_cubeFaceSize,
        .depthOrArraySize = static_cast<uint16_t>(numCubeSides * (m_numCubes)),
        .clearValue = D3D12_CLEAR_VALUE {
            .Format = DXGI_FORMAT_D32_FLOAT,
            .DepthStencil = {.Depth = 1.0f, .Stencil = 0}
        },
        .usageFlags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
        .initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE,
    });

    m_viewport = { .TopLeftX = 0,
                         .TopLeftY = 0,
                         .Width = static_cast<float>(a_cubeFaceSize),
                         .Height = static_cast<float>(a_cubeFaceSize),
                         .MinDepth = 0,
                         .MaxDepth = 1
    };
    m_scissor = { .left = 0,
                        .top = 0,
                        .right = static_cast<int32_t>(a_cubeFaceSize),
                        .bottom = static_cast<int32_t>(a_cubeFaceSize)
    };
}

DescriptorHeap::Handle LightGrid::getRadianceSRV() {
    if (!m_radianceSrv.has_value()) {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc;
        desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        desc.Format = m_format;
        desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
        desc.TextureCubeArray = {
            .MostDetailedMip = 0,
            .MipLevels = 1,
            .First2DArrayFace = 0,
            .NumCubes = (UINT)m_numCubes,
            .ResourceMinLODClamp = 0.0f
        };
        m_radianceSrv = m_resourceManager->getUniqueTexture(m_cubemapsTextureId).createSRV(desc);
    }
    return m_radianceSrv.value();
}

DescriptorHeap::Handle LightGrid::getIrradianceSRV() {
    if (!m_irradianceSrv.has_value()) {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc;
        desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        desc.Format = m_format;
        desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
        desc.TextureCubeArray = {
            .MostDetailedMip = 0,
            .MipLevels = 1,
            .First2DArrayFace = 0,
            .NumCubes = (UINT)m_numCubes,
            .ResourceMinLODClamp = 0.0f
        };
        m_irradianceSrv = m_resourceManager->getUniqueTexture(m_irradianceTextureId).createSRV(desc);
    }
    return m_irradianceSrv.value();
}

DescriptorHeap::Handle LightGrid::getRadianceRTV() {
    return m_resourceManager->getUniqueTexture(m_cubemapsTextureId).getRTV();
}

DescriptorHeap::Handle LightGrid::getIrradianceRTV() {
    return m_resourceManager->getUniqueTexture(m_irradianceTextureId).getRTV();
}

DescriptorHeap::Handle LightGrid::getRadianceDSV() {
    return m_resourceManager->getUniqueTexture(m_cubemapsDepthTextureId).getDSV();
}
}