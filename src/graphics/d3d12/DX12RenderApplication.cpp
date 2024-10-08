#include "DX12RenderEngine.h"
#include <utils/Macros.h>

#include <iostream>
#include <cmath>

namespace neural::graphics {

void DX12RenderEngine::initializeFrameResources(uint32_t a_frameIndex)
{
    ID3D12Resource* swapchainBuffer;
    DX_CALL(m_swapChain->GetBuffer(a_frameIndex, IID_PPV_ARGS(&swapchainBuffer)));
    m_resourceManager.createTextureInFrame("mainRT", a_frameIndex, swapchainBuffer);

    m_resourceManager.createTextureInFrame("mainDepth", a_frameIndex, {
            .format = DXGI_FORMAT_D32_FLOAT,
            .width = m_windowWidth,
            .height = m_windowHeight,
            .clearValue = D3D12_CLEAR_VALUE {
                .Format = DXGI_FORMAT_D32_FLOAT,
                .DepthStencil = {.Depth = 1.0f, .Stencil = 0}
            },
            .usageFlags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
            .initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE,
    });

    auto& colorMap = m_resourceManager.createTextureInFrame("colorMap", a_frameIndex, {
            .format = DXGI_FORMAT_R32G32B32A32_FLOAT,
            .width = m_windowWidth,
            .height = m_windowHeight,
            .clearValue = D3D12_CLEAR_VALUE {
                .Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
                .Color = {0, 0, 0, 1}
            },
            .usageFlags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
            .initialState = D3D12_RESOURCE_STATE_RENDER_TARGET,
    });
   
    m_resourceManager.createTextureInFrame("normalMap", a_frameIndex, {
            .format = DXGI_FORMAT_R32G32B32A32_FLOAT,
            .width = m_windowWidth,
            .height = m_windowHeight,
            .clearValue = D3D12_CLEAR_VALUE {
                .Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
                .Color = {0, 0, 0, 1}
            },
            .usageFlags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
            .initialState = D3D12_RESOURCE_STATE_RENDER_TARGET,
    });
    
    m_resourceManager.createTextureInFrame("toCameraMap", a_frameIndex, {
            .format = DXGI_FORMAT_R32G32B32A32_FLOAT,
            .width = m_windowWidth,
            .height = m_windowHeight,
            .clearValue = D3D12_CLEAR_VALUE {
                .Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
                .Color = {0, 0, 0, 1}
            },
            .usageFlags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
            .initialState = D3D12_RESOURCE_STATE_RENDER_TARGET,
    });

    // createReadbackBufferForTexture(m_resourceManager, "colorMap", a_frameIndex);
    // createReadbackBufferForTexture(m_resourceManager, "normalMap", a_frameIndex);
    // createReadbackBufferForTexture(m_resourceManager, "toCameraMap", a_frameIndex);

    m_resourceManager.createConstantBufferInFrame("cb", a_frameIndex, {
        .size = 1,
        .elementSize = sizeof(CBCameraParams)
    });

    m_dmlModel[a_frameIndex].initialize(m_mainDevice.Get(), m_dmlDevice.Get(), m_resourceManager.getCBVHeap(),
                          m_windowWidth, m_windowHeight);
    m_dmlModel[a_frameIndex].setInitializationBindings();
}

void DX12RenderEngine::initializeUniqueResources()
{
    m_sceneManager.initialize(m_mainDevice.Get());
    m_sceneManager.loadMeshFromFile("cat", RESOURCES"/models/Cat_Sitting.fbx",
        { .rotation = {90, -90, 0}, .scale = 0.5 });
    m_sceneManager.loadMeshFromFile("bird", RESOURCES"/models/Bird.obj",
        { .rotation = {0, 0, 0}, .scale = 0.2 });
    std::vector<SceneManager::Vertex> planeVertices = {
        {{-1, 0, 1}, {0, 1, 0}, {0,0}},
        {{1, 0, 1}, {0, 1, 0}, {0,0}},
        {{-1, 0, -1}, {0, 1, 0}, {0,0}},
        {{1, 0, -1}, {0, 1, 0}, {0,0}},
    };
    std::vector<uint32_t> planeIndices = {
        0, 1, 2,  1, 3, 2
    };
    m_sceneManager.loadMesh("flat", planeVertices, planeIndices, { .scale = 100 });
}

void DX12RenderEngine::initializePipelines()
{
    m_rootSignature.initialize(m_mainDevice.Get(),
        {
            {
                .parameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,
                .constants = {.baseShaderRegister = 0, .num32BitValues = 1}
            },
            {
                .parameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .descriptor = {.baseShaderRegister = 1}
            },
        });
    NAME_DX_OBJECT(m_rootSignature.getID3D12RootSignature(), L"RootSignature");

    m_finalRenderPipeline.initialize(m_mainDevice.Get(), std::string_view("final_render"),
        GraphicsPipeline::CreateInfo{
            .rootSignature = m_rootSignature,
            .inputLayout = {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
            },
            .vertexShaderPath = D3D12_ROOT"/shaders/compiled/1.vs.cso",
            .pixelShaderPath = D3D12_ROOT"/shaders/compiled/1.ps.cso",
            .RTVFormats = {DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32G32B32A32_FLOAT,
                           DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT},
            .DSVFormat = DXGI_FORMAT_D32_FLOAT
        });
    NAME_DX_OBJECT(m_finalRenderPipeline.getID3D12Pipeline(), L"FinalRenderPipeline");

    m_basicRenderPipeline.initialize(m_mainDevice.Get(), std::string_view("basic_render"),
        GraphicsPipeline::CreateInfo{
            .rootSignature = m_rootSignature,
            .inputLayout = {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
            },
            .vertexShaderPath = D3D12_ROOT"/shaders/compiled/basic.vs.cso",
            .pixelShaderPath = D3D12_ROOT"/shaders/compiled/basic.ps.cso",
            .RTVFormats = {DXGI_FORMAT_R8G8B8A8_UNORM},
            .DSVFormat = DXGI_FORMAT_D32_FLOAT
        });
    NAME_DX_OBJECT(m_basicRenderPipeline.getID3D12Pipeline(), L"BasicRenderPipeline");
}

void DX12RenderEngine::initialCommands()
{
    m_sceneManager.uploadMeshesOnGPU(m_commandList.Get(), &m_resourceManager);
    for (int i = 0; i < k_nSwapChainBuffers; ++i) {
        m_dmlModel[i].dispatchInitialization(m_dmlCommandRecorder.Get(), m_commandList.Get());
    }
}

void DX12RenderEngine::afterInitialCommands()
{
    for (int i = 0; i < k_nSwapChainBuffers; ++i) {
        m_dmlModel[i].setExecutionBindings();
    }
}

void DX12RenderEngine::render(const Timer& a_timer)
{
    beginFrame();
    const uint64_t frameIndex = m_currentFrame % k_nSwapChainBuffers;
    m_settings.camera.updateViewMatrix();
    m_cbCameraParams.LightPosition = { 0, 20, 0 };

    DirectX::XMStoreFloat4x4(&m_cbCameraParams.WorldMatrix,
        DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&m_worldMatrix)));
    DirectX::XMStoreFloat4x4(&m_cbCameraParams.ViewProjMatrix, 
        DirectX::XMMatrixMultiplyTranspose(m_settings.camera.getView(), m_settings.camera.getProj()));
    m_resourceManager.getConstantBuffer("cb", frameIndex).uploadData(&m_cbCameraParams);

    ID3D12DescriptorHeap* descriptorHeaps[] = { m_resourceManager.getCBVHeap()->getID3D12DescriptorHeap()};
    m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    auto currentBufferView = m_resourceManager.getTexture("mainRT", frameIndex).getRTV();
    auto currentDepthBufferView = m_resourceManager.getTexture("mainDepth", frameIndex).getDSV();
    float color[] = { 0, 0, 0, 1 };

    m_commandList->ClearRenderTargetView(currentBufferView.cpu, color, 0, nullptr);
    m_commandList->ClearDepthStencilView(currentDepthBufferView.cpu,
        D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    m_commandList->SetGraphicsRootSignature(m_rootSignature.getID3D12RootSignature());

    const bool isFinalPipeline = m_settings.ml || m_settings.doScreenShot;
    if (isFinalPipeline) {
        m_commandList->ClearRenderTargetView(m_resourceManager.getTexture("colorMap", frameIndex).getRTV().cpu,
                                            color, 0, nullptr);
        m_commandList->ClearRenderTargetView(m_resourceManager.getTexture("normalMap", frameIndex).getRTV().cpu,
                                            color, 0, nullptr);
        m_commandList->ClearRenderTargetView(m_resourceManager.getTexture("toCameraMap", frameIndex).getRTV().cpu,
                                            color, 0, nullptr);
        m_commandList->SetPipelineState(m_finalRenderPipeline.getID3D12Pipeline());
    } else {
        m_commandList->SetPipelineState(m_basicRenderPipeline.getID3D12Pipeline());
    }

    // m_commandList->SetGraphicsRootDescriptorTable(0, m_constantBuffer[currentFrameBufferIndex].getConstantBufferView().gpu);

    m_commandList->SetGraphicsRoot32BitConstant(0, 0, 0);
    m_commandList->SetGraphicsRootConstantBufferView(1,
        m_resourceManager.getConstantBuffer("cb", frameIndex).getID3D12Resource()->GetGPUVirtualAddress());

    const auto& vertexBufferView = m_sceneManager.getVertexBufferView();
    const auto& indexBufferView = m_sceneManager.getIndexBufferView();
    m_commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
    m_commandList->IASetIndexBuffer(&indexBufferView);
    m_commandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    auto& mainRT =  m_resourceManager.getTexture("mainRT", frameIndex);

    auto& colorMap =  m_resourceManager.getTexture("colorMap", frameIndex);
    // auto& colorMapReadback =  m_resourceManager.getBuffer("colorMapReadback", frameIndex);

    auto& normalMap =  m_resourceManager.getTexture("normalMap", frameIndex);
    // auto& normalMapReadback =  m_resourceManager.getBuffer("normalMapReadback", frameIndex);

    auto& toCameraMap =  m_resourceManager.getTexture("toCameraMap", frameIndex);
    // auto& toCameraMapReadback =  m_resourceManager.getBuffer("toCameraMapReadback", frameIndex);

    D3D12_CPU_DESCRIPTOR_HANDLE renderTargets[] = {
        mainRT.getRTV().cpu,
        colorMap.getRTV().cpu,
        normalMap.getRTV().cpu,
        toCameraMap.getRTV().cpu
    };
    m_commandList->OMSetRenderTargets(isFinalPipeline ? _countof(renderTargets) : 1, renderTargets, true, &currentDepthBufferView.cpu);

    const auto& meshInfo = m_sceneManager.getMeshInfo(m_settings.meshName.c_str());
    m_commandList->DrawIndexedInstanced(meshInfo.indexCount, 1, meshInfo.startIndex, meshInfo.startVertex, 0);

    m_commandList->SetGraphicsRoot32BitConstant(0, 1, 0);
    const auto& meshFlat = m_sceneManager.getMeshInfo("flat");
    m_commandList->DrawIndexedInstanced(meshFlat.indexCount, 1, meshFlat.startIndex, meshFlat.startVertex, 0);

    // if (m_settings.doScreenShot)
    // {
    //     copyTextureToReadbackBuffer(m_commandList.Get(), colorMap, colorMapReadback);
    //     copyTextureToReadbackBuffer(m_commandList.Get(), normalMap, normalMapReadback);
    //     copyTextureToReadbackBuffer(m_commandList.Get(), toCameraMap, toCameraMapReadback);

    //     m_settings.doScreenShot = false;
    //     m_screenshotWaitFences.push(m_currentFrame);
    // }

    if (m_settings.showGUI && !m_settings.doScreenShot) {
        renderGUI();
    }
    if (m_settings.ml) {
        ID3D12DescriptorHeap* descriptorHeapsDml[] = { m_dmlModel[frameIndex].getID3D12DescriptorHeap() };
        m_commandList->SetDescriptorHeaps(_countof(descriptorHeapsDml), descriptorHeapsDml);
        m_dmlModel[frameIndex].dispatch(m_dmlCommandRecorder.Get(), m_commandList.Get());
    }
    endFrame();
}
}
