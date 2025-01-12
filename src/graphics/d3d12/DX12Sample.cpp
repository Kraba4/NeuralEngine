#include "DX12RenderEngine.h"
#include "DX12SampleNames.h"
#include <utils/Macros.h>

#include <par_shapes.h>
#include <iostream>
#include <cmath>

namespace neural::graphics {

void DX12RenderEngine::initializeFrameResources(uint32_t a_frameIndex)
{
    m_resourceManager.resizeFrameResources(a_frameIndex, FCB_SIZE, 0, FT_SIZE);

    ID3D12Resource* swapchainBuffer;
    DX_CALL(m_swapChain->GetBuffer(a_frameIndex, IID_PPV_ARGS(&swapchainBuffer)));
    m_resourceManager.createTextureInFrame(FT_Main, "MainRT", a_frameIndex, swapchainBuffer);
    
    m_resourceManager.createTextureInFrame(FT_MainDepth, "MainDepth", a_frameIndex, {
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

    auto& colorMap = m_resourceManager.createTextureInFrame(FT_GBufferColor, "ColorMap", a_frameIndex, {
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

    m_resourceManager.createTextureInFrame(FT_GBufferNormal, "NormalMap", a_frameIndex, {
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

    m_resourceManager.createTextureInFrame(FT_GBufferToCamera, "ToCameraMap", a_frameIndex, {
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

    m_resourceManager.createConstantBufferInFrame(FCB_ProjViewLight, "CbProjViewLight", a_frameIndex, {
        .size = 1,
        .elementSize = sizeof(m_generalCB)
    });

    m_dmlModel[a_frameIndex].initialize(m_mainDevice.Get(), m_dmlDevice.Get(), m_resourceManager.getCBVHeap(),
                          m_windowWidth, m_windowHeight);
    m_dmlModel[a_frameIndex].setInitializationBindings();
}

void DX12RenderEngine::initializeUniqueResources()
{
    m_resourceManager.resizeUniqueResources(0, UB_SIZE, UT_SIZE);

    m_sceneManager.initialize(m_mainDevice.Get(), M_SIZE);
    m_sceneManager.loadMeshFromFile(M_Cat, "cat_mesh", RESOURCES"/models/Cat_Sitting.fbx",
        { .rotation = {90, -90, 0}, .scale = 0.5 });
    m_sceneManager.loadMeshFromFile(M_Bird, "bird_mesh", RESOURCES"/models/Bird.obj",
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
    m_sceneManager.loadMesh(M_Plane, "plane_mesh", planeVertices, planeIndices, { .scale = 100 });
    m_sceneManager.loadMesh(M_Wall, "wall_mesh", planeVertices, planeIndices, { .scale = 10 });
    par_shapes_mesh* par_sphere = par_shapes_create_subdivided_sphere(2);
    // for (int i = 0; i < par_sphere->npoints * 3; ++i) {
    //     par_sphere->points[i] = par_sphere->points[i] * 2.0f - 1.0f;
    // }
    m_sceneManager.loadParMesh(M_Sphere, "sphere_mesh", par_sphere, false);
    par_shapes_free_mesh(par_sphere);

    DirectX::XMFLOAT4X4 objectWorldMatrix;
    DirectX::XMStoreFloat4x4(&objectWorldMatrix, DirectX::XMMatrixMultiply(
                                                 DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(-90)),
                                                 DirectX::XMMatrixTranslation(-7, 0, 0)));
    m_sceneManager.newObject("wall1", M_Wall, {0.3, 0.3, 0.3, 1}, objectWorldMatrix);

    DirectX::XMStoreFloat4x4(&objectWorldMatrix,DirectX::XMMatrixMultiply(
                                                 DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(90)),
                                                 DirectX::XMMatrixTranslation(7, 0, 0)));
    m_sceneManager.newObject("wall2", M_Wall, {0.3, 0.3, 0.3, 1}, objectWorldMatrix);

    DirectX::XMStoreFloat4x4(&objectWorldMatrix, DirectX::XMMatrixMultiply(
                                                 DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(-90)),
                                                 DirectX::XMMatrixTranslation(0, 0, 7)));
    m_sceneManager.newObject("wall3", M_Wall, {0.3, 0.3, 0.3, 1}, objectWorldMatrix);


    DirectX::XMStoreFloat4x4(&objectWorldMatrix, DirectX::XMMatrixTranslation(0, -1.5, 0));
    m_sceneManager.newObject("ground", M_Plane, {0.3, 0.3, 0.3, 1}, objectWorldMatrix);

    DirectX::XMStoreFloat4x4(&objectWorldMatrix, DirectX::XMMatrixTranslation(-5, 0, 0));
    m_sceneManager.newObject("green", m_settings.selectedMesh, {0, 0.7, 0.1, 1}, objectWorldMatrix);

    DirectX::XMStoreFloat4x4(&objectWorldMatrix, DirectX::XMMatrixTranslation(5, 0, 0));
    m_sceneManager.newObject("yellow", m_settings.selectedMesh, {0.7, 0.7, 0, 1}, objectWorldMatrix);

    DirectX::XMStoreFloat4x4(&objectWorldMatrix, DirectX::XMMatrixTranslation(0, 0, 5));
    m_sceneManager.newObject("red", m_settings.selectedMesh, {0.7, 0, 0.1, 1}, objectWorldMatrix);

    DirectX::XMStoreFloat4x4(&objectWorldMatrix, DirectX::XMMatrixTranslation(0, 0, -5));
    m_sceneManager.newObject("blue", m_settings.selectedMesh, {0.1, 0, 0.7, 1}, objectWorldMatrix);

    DirectX::XMStoreFloat4x4(&objectWorldMatrix, DirectX::XMMatrixTranslation(0, 0, 10));
    m_sceneManager.newObject("white", m_settings.selectedMesh, {1, 1, 1, 1}, objectWorldMatrix);

    m_selectedMatrix = &m_sceneManager.getObjectByName("yellow").worldMatrix;

    //// Setup LightGrid
    {
        m_generalCB.GridPos_ElementScale = {-7.2f, -1.8f, -7.f,  0.6f};
        m_generalCB.Width_Height_Depth_Distance = {4, 2, 4, 4.6f};

        const int gridWidth = m_generalCB.Width_Height_Depth_Distance.x;
        const int gridHeight = m_generalCB.Width_Height_Depth_Distance.y;
        const int gridDepth = m_generalCB.Width_Height_Depth_Distance.z;
        const int numLightProbes = 4 * 2 * 4; //gridWidth * gridHeight * gridDepth;
        constexpr int cubemapFaceSize = 32;
        constexpr int numCubeSides = 6;
        m_lightGrid.initialize(cubemapFaceSize, numLightProbes, DXGI_FORMAT_R8G8B8A8_UNORM,
                            m_resourceManager, UT_LightGridCubemaps, UT_LightGridDepthCubemaps, UT_LightGridIrradianceCubemaps);

        Camera cubeCamera;
        cubeCamera.setFrustum(DirectX::XMConvertToRadians(90), 1.f, 0.1f, 1000);
        m_generalCB.CubeProjMatrix = cubeCamera.getProj4x4f();

        //// idk why need reverse lookatdirections
        DirectX::XMFLOAT3 lookAtDirections[numCubeSides] = {{-1, 0, 0}, {1, 0, 0},
                                                            {0, -1, 0}, {0, 1, 0},
                                                            {0, 0, 1}, {0, 0, -1}};
        DirectX::XMFLOAT3 upDirections[numCubeSides]    =  {{0, 1, 0}, {0, 1, 0},
                                                            {0, 0, 1}, {0, 0, -1},
                                                            {0, 1, 0}, {0, 1, 0}};
        for (int i = 0; i < numCubeSides; ++i) {
            cubeCamera.setFrustum(DirectX::XMConvertToRadians(90), 1.f, 0.5f, 1000);
            cubeCamera.setPosition({0, 0, 0});
            cubeCamera.lookAt(DirectX::XMFLOAT3{0, 0, 0}, lookAtDirections[i], upDirections[i]);
            m_generalCB.CubeFaceViewMatrix[i] = cubeCamera.getView4x4f();
        }
    }
}

void DX12RenderEngine::initializePipelines()
{
    m_rootSignatures.resize(RS_SIZE);
    m_graphicsPSO.resize(GP_SIZE);

    m_rootSignatures[RS_Basic].initialize(m_mainDevice.Get(),
        {
            {
                .parameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,
                .constants = {.baseShaderRegister = 0, .num32BitValues = 20}
            },
            {
                .parameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .descriptor = {.baseShaderRegister = 1}
            },
            {
                .parameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
                .descriptorTableRanges = {
                    RootSignature::DescriptorRange {
                        .rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                        .baseShaderRegister = 2,
                        .numDescriptors = 1
                    }
                }
            }
        },
        {
           CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                                          D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                          D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                          D3D12_TEXTURE_ADDRESS_MODE_WRAP)
        });
    NAME_DX_OBJECT(m_rootSignatures[RS_Basic].getID3D12RootSignature(), L"RootSignature");

    m_graphicsPSO[GP_Final].initialize(m_mainDevice.Get(), std::string_view("final_render"),
        GraphicsPipeline::CreateInfo{
            .rootSignature = m_rootSignatures[RS_Basic],
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
    NAME_DX_OBJECT(m_graphicsPSO[GP_Final].getID3D12Pipeline(), L"FinalRenderPipeline");

    m_graphicsPSO[GP_Basic].initialize(m_mainDevice.Get(), std::string_view("basic_render"),
        GraphicsPipeline::CreateInfo{
            .rootSignature = m_rootSignatures[RS_Basic],
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
    NAME_DX_OBJECT(m_graphicsPSO[GP_Basic].getID3D12Pipeline(), L"BasicRenderPipeline");

    m_graphicsPSO[GP_BakeLightGrid].initialize(m_mainDevice.Get(), std::string_view("bake_lightgrid_pipeline"),
        GraphicsPipeline::CreateInfo {
            .rootSignature = m_rootSignatures[RS_Basic],
            .inputLayout = {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
            },
            .vertexShaderPath = D3D12_ROOT"/shaders/compiled/bake_lightgrid.vs.cso",
            .geometryShaderPath = D3D12_ROOT"/shaders/compiled/bake_lightgrid.gs.cso",
            .pixelShaderPath = D3D12_ROOT"/shaders/compiled/bake_lightgrid.ps.cso",
            .RTVFormats = {DXGI_FORMAT_R8G8B8A8_UNORM},
            .DSVFormat = DXGI_FORMAT_D32_FLOAT
        });
    NAME_DX_OBJECT(m_graphicsPSO[GP_BakeLightGrid].getID3D12Pipeline(), L"BakeLightgridPipeline");
    
    D3D12_DEPTH_STENCIL_DESC depthDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    depthDesc.DepthEnable = FALSE;
    depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    m_graphicsPSO[GP_CalcIrradiance].initialize(m_mainDevice.Get(), std::string_view("irradiance_lightgrid_pipeline"),
        GraphicsPipeline::CreateInfo {
            .rootSignature = m_rootSignatures[RS_Basic],
            .vertexShaderPath = D3D12_ROOT"/shaders/compiled/irradiance_lightgrid.vs.cso",
            .geometryShaderPath = D3D12_ROOT"/shaders/compiled/irradiance_lightgrid.gs.cso",
            .pixelShaderPath = D3D12_ROOT"/shaders/compiled/irradiance_lightgrid.ps.cso",
            .depthStencilState = depthDesc,
            .RTVFormats = {DXGI_FORMAT_R8G8B8A8_UNORM},
            .DSVFormat = DXGI_FORMAT_D32_FLOAT
        });
    NAME_DX_OBJECT(m_graphicsPSO[GP_CalcIrradiance].getID3D12Pipeline(), L"IrradianceLightgridPipeline");

    m_graphicsPSO[GP_DrawLightGrid].initialize(m_mainDevice.Get(), std::string_view("draw_lightgrid_pipeline"),
        GraphicsPipeline::CreateInfo {
            .rootSignature = m_rootSignatures[RS_Basic],
            .inputLayout = {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
            },
            .vertexShaderPath = D3D12_ROOT"/shaders/compiled/draw_lightgrid.vs.cso",
            .pixelShaderPath = D3D12_ROOT"/shaders/compiled/draw_lightgrid.ps.cso",
            .RTVFormats = {DXGI_FORMAT_R8G8B8A8_UNORM},
            .DSVFormat = DXGI_FORMAT_D32_FLOAT
        });
    NAME_DX_OBJECT(m_graphicsPSO[GP_DrawLightGrid].getID3D12Pipeline(), L"DrawLightgridPipeline");
}

void DX12RenderEngine::initialCommands()
{
    m_sceneManager.uploadMeshesOnGPU(m_commandList.Get(), &m_resourceManager, 
                                     UB_Vertex, UB_Index, UB_VertexUpload, UB_IndexUpload);
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

void doScreenShotFunc(uint64_t a_frame, uint64_t a_nSwapChainBuffers,ID3D12Fence* a_framesFence, HANDLE& a_eventHandle,
                      ResourceManager& a_rm, ID3D12CommandQueue* a_commandQueue, RenderSettings& a_settings);

void DX12RenderEngine::drawScene(int probeIndex = 0) {
    for (const SceneManager::Object& object : m_sceneManager.getObjects()) {
        ObjectRC objectConstants;
        DirectX::XMStoreFloat4x4(&objectConstants.transposedWorldMatrix, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&object.worldMatrix)));
        objectConstants.ObjectColor_ProbeIndex = object.color;
        objectConstants.ObjectColor_ProbeIndex.w = probeIndex;
        m_commandList->SetGraphicsRoot32BitConstants(0, ObjectRC::num32BitValues(), &objectConstants, 0);

        const auto& meshInfo = m_sceneManager.getMeshInfo(object.meshIndex);
        m_commandList->DrawIndexedInstanced(meshInfo.indexCount, 1, meshInfo.startIndex, meshInfo.startVertex, 0);
    }
}
void DX12RenderEngine::render(const Timer& a_timer)
{
    const uint32_t frameIndex = beginFrame();

    const bool isFinalPipeline = m_settings.ml || m_settings.doScreenShot;
    float clearColor[] = { 0, 0, 0, 1 };

    //// Setup RenderTargets
    {
        auto& depthBuffer = m_resourceManager.getTexture(FT_MainDepth, frameIndex);
        auto& mainRT =  m_resourceManager.getTexture(FT_Main, frameIndex);

        auto& colorMap =  m_resourceManager.getTexture(FT_GBufferColor, frameIndex);
        auto& normalMap =  m_resourceManager.getTexture(FT_GBufferNormal, frameIndex);
        auto& toCameraMap =  m_resourceManager.getTexture(FT_GBufferToCamera, frameIndex);

        mainRT.applyBarrier(m_commandList.Get(), {
            .from = D3D12_RESOURCE_STATE_PRESENT,
            .to = D3D12_RESOURCE_STATE_RENDER_TARGET});

        m_commandList->RSSetViewports(1, &m_screenViewport);
        m_commandList->RSSetScissorRects(1, &m_screenScissor);
        m_commandList->ClearRenderTargetView(mainRT.getRTV().cpu, clearColor, 0, nullptr);
        m_commandList->ClearDepthStencilView(depthBuffer.getDSV().cpu, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

        D3D12_CPU_DESCRIPTOR_HANDLE renderTargets[] = {
            mainRT.getRTV().cpu,
            colorMap.getRTV().cpu,
            normalMap.getRTV().cpu,
            toCameraMap.getRTV().cpu
        };
        D3D12_CPU_DESCRIPTOR_HANDLE depthViewCPU = depthBuffer.getDSV().cpu;
        m_commandList->OMSetRenderTargets(isFinalPipeline ? _countof(renderTargets) : 1, renderTargets, true, &depthViewCPU);
    }

    //// Bind Objects pipeline 
    {
        m_commandList->SetGraphicsRootSignature(m_rootSignatures[RS_Basic].getID3D12RootSignature());

        if (isFinalPipeline) {
            m_commandList->ClearRenderTargetView(m_resourceManager.getTexture(FT_GBufferColor, frameIndex).getRTV().cpu,
                                                clearColor, 0, nullptr);
            m_commandList->ClearRenderTargetView(m_resourceManager.getTexture(FT_GBufferNormal, frameIndex).getRTV().cpu,
                                                clearColor, 0, nullptr);
            m_commandList->ClearRenderTargetView(m_resourceManager.getTexture(FT_GBufferToCamera, frameIndex).getRTV().cpu,
                                                clearColor, 0, nullptr);
            m_commandList->SetPipelineState(m_graphicsPSO[GP_Final].getID3D12Pipeline());
        } else {
            m_commandList->SetPipelineState(m_graphicsPSO[GP_Basic].getID3D12Pipeline());
        }
    }

    //// Set ConstantBuffer
    {
        ID3D12DescriptorHeap* descriptorHeaps[] = { m_resourceManager.getCBVHeap()->getID3D12DescriptorHeap()};
        m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

        if (m_settings.selectedCamera != -1) {
            if (m_isOriginalCamera) {
                m_isOriginalCamera = false;
                m_originalCamera = m_settings.camera;
            }
            Camera cubeCamera;
            cubeCamera.setFrustum(DirectX::XMConvertToRadians(90), 1.f, 1, 1000);
            m_generalCB.CubeProjMatrix = cubeCamera.getProj4x4f();
            constexpr int numCubeSides = 6;
            //// idk why need reverse lookatdirections
            DirectX::XMFLOAT3 lookAtDirections[numCubeSides] = {{-1, 0, 0}, {1, 0, 0},
                                                                {0, -1, 0}, {0, 1, 0},
                                                                {0, 0, -1}, {0, 0, 1}};
            DirectX::XMFLOAT3 upDirections[numCubeSides]    =  {{0, 1, 0}, {0, 1, 0},
                                                                {0, 0, 1}, {0, 0, -1},
                                                                {0, 1, 0}, {0, 1, 0}};
            int i = m_settings.selectedCamera;
            cubeCamera.setFrustum(DirectX::XMConvertToRadians(90), 1.f, 1, 1000);
            cubeCamera.setPosition({0, 0, 0});
            cubeCamera.lookAt(DirectX::XMFLOAT3{0, 0, 0},  lookAtDirections[i], upDirections[i]);
            cubeCamera.updateViewMatrix();
            m_generalCB.CubeFaceViewMatrix[i] = cubeCamera.getView4x4f();

            m_settings.camera = cubeCamera;
        } else {
            m_isOriginalCamera = true;
            if (m_originalCamera.has_value()) {
                m_settings.camera = m_originalCamera.value();
                m_originalCamera.reset();
            }
        }

        m_generalCB.LightPosition_CubeFaceSize = { 0, 20, 0,  (float)m_lightGrid.getFaceSize() };
        m_settings.camera.updateViewMatrix();
        DirectX::XMStoreFloat4x4(&m_generalCB.ViewProjMatrix, DirectX::XMMatrixMultiplyTranspose(m_settings.camera.getView(), m_settings.camera.getProj()));
        
        m_resourceManager.getConstantBuffer(FCB_ProjViewLight, frameIndex).uploadData(&m_generalCB);
    
        m_commandList->SetGraphicsRootConstantBufferView(1, m_resourceManager.getConstantBuffer(FCB_ProjViewLight, frameIndex).getID3D12Resource()->GetGPUVirtualAddress());
        m_commandList->SetGraphicsRootDescriptorTable(2, m_lightGrid.getIrradianceSRV().gpu);
    }

    //// Set Meshes
    {
        const auto& vertexBufferView = m_sceneManager.getVertexBufferView();
        const auto& indexBufferView = m_sceneManager.getIndexBufferView();
        m_commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
        m_commandList->IASetIndexBuffer(&indexBufferView);
        m_commandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        m_sceneManager.getObjectByName("green").meshIndex = m_settings.selectedMesh;
    }

    //// Draw Objects
    drawScene();

    //// Bake light grid if need
    if (m_settings.bakeLightGrid) {
        m_settings.bakeLightGrid = false;

        m_lightGrid.getRadianceTexture().applyBarrier(m_commandList.Get(), {
            .from = D3D12_RESOURCE_STATE_GENERIC_READ,
            .to = D3D12_RESOURCE_STATE_RENDER_TARGET
        });

        m_commandList->ClearRenderTargetView(m_lightGrid.getRadianceRTV().cpu, clearColor, 0, nullptr);
        m_commandList->ClearDepthStencilView(m_lightGrid.getRadianceDSV().cpu, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
        
        m_commandList->RSSetViewports(1, &m_lightGrid.getViewport());
        m_commandList->RSSetScissorRects(1, &m_lightGrid.getScissor());

        D3D12_CPU_DESCRIPTOR_HANDLE renderTargets[] = { m_lightGrid.getRadianceRTV().cpu };
        D3D12_CPU_DESCRIPTOR_HANDLE depthDsv = m_lightGrid.getRadianceDSV().cpu;
        m_commandList->OMSetRenderTargets(1, renderTargets, true, &depthDsv);

        m_commandList->SetPipelineState(m_graphicsPSO[GP_BakeLightGrid].getID3D12Pipeline());
        
        const int width = m_generalCB.Width_Height_Depth_Distance.x;
        const int height = m_generalCB.Width_Height_Depth_Distance.y;
        const int depth = m_generalCB.Width_Height_Depth_Distance.z;
        const int numProbes = width * height * depth;
        for (int i = 0; i < numProbes; ++i) {
            drawScene(i);
        }

        m_lightGrid.getRadianceTexture().applyBarrier(m_commandList.Get(), {
            .from = D3D12_RESOURCE_STATE_RENDER_TARGET,
            .to = D3D12_RESOURCE_STATE_GENERIC_READ
        });

        m_commandList->ClearRenderTargetView(m_lightGrid.getIrradianceRTV().cpu, clearColor, 0, nullptr);
        renderTargets[0] = { m_lightGrid.getIrradianceRTV().cpu };
        m_commandList->OMSetRenderTargets(1, renderTargets, true, nullptr);

        m_commandList->SetGraphicsRootDescriptorTable(2, m_lightGrid.getRadianceSRV().gpu);
        m_commandList->SetPipelineState(m_graphicsPSO[GP_CalcIrradiance].getID3D12Pipeline());

        for (int i = 0; i < numProbes; ++i) {
            ObjectRC objectConstants;
            objectConstants.ObjectColor_ProbeIndex.w = i;
            m_commandList->SetGraphicsRoot32BitConstants(0, ObjectRC::num32BitValues(), &objectConstants, 0);

            m_commandList->DrawInstanced(3, 1, 0, 0);
        }
    }

    //// Draw LightGrid
    if (m_settings.showLightGrid)
    {
        m_lightGrid.getIrradianceTexture().applyBarrier(m_commandList.Get(), {
            .from = D3D12_RESOURCE_STATE_RENDER_TARGET,
            .to = D3D12_RESOURCE_STATE_GENERIC_READ
        });
        D3D12_CPU_DESCRIPTOR_HANDLE renderTargets[] = {
            m_resourceManager.getTexture(FT_Main, frameIndex).getRTV().cpu
        };
        D3D12_CPU_DESCRIPTOR_HANDLE depthViewCPU = m_resourceManager.getTexture(FT_MainDepth, frameIndex).getDSV().cpu;
        m_commandList->OMSetRenderTargets(1, renderTargets, true, &depthViewCPU);

        m_commandList->RSSetViewports(1, &m_screenViewport);
        m_commandList->RSSetScissorRects(1, &m_screenScissor);

        if (m_settings.showProbeEnvironment) {
            m_commandList->SetGraphicsRootDescriptorTable(2, m_lightGrid.getRadianceSRV().gpu);
        } else {
            m_commandList->SetGraphicsRootDescriptorTable(2, m_lightGrid.getIrradianceSRV().gpu);
        }
        m_commandList->SetPipelineState(m_graphicsPSO[GP_DrawLightGrid].getID3D12Pipeline());
        // m_commandList->SetGraphicsRoot32BitConstants(0, LightGridRC::num32BitValues(), &m_lightGridRC, 0);

        const int width = m_generalCB.Width_Height_Depth_Distance.x;
        const int height = m_generalCB.Width_Height_Depth_Distance.y;
        const int depth = m_generalCB.Width_Height_Depth_Distance.z;

        const auto& meshInfo = m_sceneManager.getMeshInfo(M_Sphere);
        m_commandList->DrawIndexedInstanced(meshInfo.indexCount, width * depth * height, meshInfo.startIndex, meshInfo.startVertex, 0);
    }
    
    //// Render GUI
    if (m_settings.showGUI && !m_settings.doScreenShot) {
        renderGUI();
    }

    //// Compute ml model (just test now)
    if (m_settings.ml) {
        ID3D12DescriptorHeap* descriptorHeapsDml[] = { m_dmlModel[frameIndex].getID3D12DescriptorHeap() };
        m_commandList->SetDescriptorHeaps(_countof(descriptorHeapsDml), descriptorHeapsDml);
        m_dmlModel[frameIndex].dispatch(m_dmlCommandRecorder.Get(), m_commandList.Get());
    }

    //// Apply barrier and send command buffer on gpu
    m_resourceManager.getTexture(FT_Main, frameIndex).applyBarrier(m_commandList.Get(), 
        {
            .from = D3D12_RESOURCE_STATE_RENDER_TARGET,
            .to = D3D12_RESOURCE_STATE_PRESENT
        });
    endFrame();

    //// Save GBuffer from current render result 
    if (m_settings.doScreenShot) {
        doScreenShotFunc(m_currentFrame - 1, k_nSwapChainBuffers,
                         m_framesFence.Get(), m_eventHandle,
                         m_resourceManager, m_commandQueue.Get(), m_settings);
    }
}

void doScreenShotFunc(uint64_t a_frame, uint64_t a_nSwapChainBuffers,
                      ID3D12Fence* a_framesFence, HANDLE& a_eventHandle,
                      ResourceManager& a_rm, ID3D12CommandQueue* a_commandQueue, RenderSettings& a_settings)
{
    // wait until the gpu draw current buffer
    const uint64_t currentFrameBufferFenceValue = a_frame;
    if (a_framesFence->GetCompletedValue() < currentFrameBufferFenceValue) {
        DX_CALL(a_framesFence->SetEventOnCompletion(currentFrameBufferFenceValue, a_eventHandle));
        WaitForSingleObject(a_eventHandle, INFINITE);
    }

    const uint64_t currentFrameBufferIndex = a_frame % a_nSwapChainBuffers;
    auto colorRT =  a_rm.getTexture(FT_GBufferColor, currentFrameBufferIndex).getID3D12Resource();
    auto normalRT =  a_rm.getTexture(FT_GBufferNormal, currentFrameBufferIndex).getID3D12Resource();
    auto toCameraRT =  a_rm.getTexture(FT_GBufferToCamera, currentFrameBufferIndex).getID3D12Resource();

    std::wstring suffix = std::to_wstring(a_settings.screenshotCounter); suffix += L".dds";
    ++a_settings.screenshotCounter; 
    std::wstring colorFileName    = MODEL_DATA_ROOT L"/colors/color"; colorFileName += suffix;
    std::wstring normalFileName   = MODEL_DATA_ROOT L"/normals/normal"; normalFileName += suffix;
    std::wstring toCameraFileName = MODEL_DATA_ROOT L"/toCameras/toCamera"; toCameraFileName += suffix;

    DirectX::SaveDDSTextureToFile(
        a_commandQueue, 
        colorRT, colorFileName.c_str(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RENDER_TARGET);

    DirectX::SaveDDSTextureToFile(
        a_commandQueue, 
        normalRT, normalFileName.c_str(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RENDER_TARGET);

    DirectX::SaveDDSTextureToFile(
        a_commandQueue, 
        toCameraRT, toCameraFileName.c_str(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RENDER_TARGET);

    a_settings.doScreenShot = false;
}
}
