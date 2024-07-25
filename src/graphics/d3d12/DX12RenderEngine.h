#pragma once
#include "graphics/IRenderEngine.h"
#include "classes/GraphicsPipeline.h"
#include "classes/RootSignature.h"
#include "classes/DescriptorHeap.h"
//#include "Resource.h"
#include <a_main/Camera.h>
#include "classes/SceneManager.h"
#include "classes/Resources.h"

#include "CommonGraphicsHeaders.h"
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_dx12.h>
#include <ImGuizmo.h>

#include <memory>

using Microsoft::WRL::ComPtr;
namespace neural::graphics {

class DX12RenderEngine : public IRenderEngine {
public:
    void initialize(HWND a_window, int a_width, int a_height) override;
    void render(const Timer& a_timer) override;
    void shutdown() override;
private:
    void createDXGIFactory();
    void createDevice();
    void recreateSwapChain();
    void createCommandQueue();
    void createCommandAllocators();
    void initialCommands();
    void createCommandListAndSendInitialCommands();
    void createFence();
    void initializeDX12ImGui();
    void flushFrameBuffers();
    void beginFrame();
    void endFrame();
    void initializePipelines();
    void initializeFrameResources(uint32_t a_frameIndex);
    void initializeUniqueResources();
    void renderGUI();
 
    static constexpr uint32_t k_nSwapChainBuffers = 3;
    static_assert(k_nSwapChainBuffers >= 2);

    static constexpr DXGI_FORMAT k_swapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    HWND m_window;
    uint32_t m_windowWidth;
    uint32_t m_windowHeight;

    ComPtr<ID3D12Device8> m_mainDevice{ nullptr };
    ComPtr<IDXGIFactory7> m_dxgiFactory{ nullptr };
    ComPtr<ID3D12CommandAllocator> m_commandAllocators[k_nSwapChainBuffers];
    ComPtr<ID3D12GraphicsCommandList>  m_commandList;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<IDXGISwapChain> m_swapChain;

    //DescriptorHeap m_rtvHeap;
    //DescriptorHeap m_dsvHeap;
    //DescriptorHeap m_cbvHeap;

    //DefaultResource m_screenTextures[k_nSwapChainBuffers];
    //DefaultResource m_depthTextures[k_nSwapChainBuffers];
    struct CBCameraParams {
        DirectX::XMFLOAT4X4 WorldMatrix;
        DirectX::XMFLOAT4X4 ViewProjMatrix;
        DirectX::XMFLOAT3 LightPosition;
    } m_cbCameraParams;
    //ConstantBuffer<CBCameraParams> m_constantBuffer[k_nSwapChainBuffers];

    GraphicsPipeline m_finalRenderPipeline;
    RootSignature m_rootSignature;

    ComPtr<ID3D12Fence> m_framesFence;
    uint64_t m_currentFrame;
    uint64_t m_frameBufferFenceValue[k_nSwapChainBuffers];
    HANDLE m_eventHandle;

    D3D12_VIEWPORT m_screenViewport;
    D3D12_RECT m_screenScissor;

    SceneManager m_sceneManager;
    ResourceManager m_resourceManager;

    ImGuizmo::OPERATION m_currentGizmoOperation;
    ImGuizmo::MODE m_currentGizmoMode;
    DirectX::XMFLOAT4X4* m_selectedMatrix;

    DirectX::XMFLOAT4X4 m_worldMatrix;
};
}