#pragma once
#include "graphics/IRenderEngine.h"
#include "GraphicsPipeline.h"
#include "RootSignature.h"
#include "DescriptorHeap.h"
#include "Resource.h"
#include <a_main/Camera.h>

#include "CommonGraphicsHeaders.h"
#include <DirectXMath.h>
#include <DirectXColors.h>

#include <memory>

using Microsoft::WRL::ComPtr;
namespace neural::graphics {

class DX12RenderEngine : public IRenderEngine {
public:
	struct Vertex {
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT3 color;
	};
	void initialize(HWND a_window, int a_width, int a_height) override;
	void render(const Timer& a_timer) override;
	void shutdown() override;
	void processInputs(const AppInput& a_appInput) override;
private:
	void createDXGIFactory();
	void createDevice();
	void recreateSwapChain();
	void createCommandQueue();
	void createCommandAllocators();
	void initialCommands();
	void createCommandListAndSendInitialCommands();
	void createFence();
	void flushFrameBuffers();
	void beginFrame();
	void endFrame();
	void initializePipelines();
	void initializeResources();

	static constexpr int k_nSwapChainBuffers = 3;
	static_assert(k_nSwapChainBuffers >= 2);

	HWND m_window;
	uint32_t m_windowWidth;
	uint32_t m_windowHeight;

	ComPtr<ID3D12Device8> m_mainDevice{ nullptr };
	ComPtr<IDXGIFactory7> m_dxgiFactory{ nullptr };
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[k_nSwapChainBuffers];
	ComPtr<ID3D12GraphicsCommandList>  m_commandList;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<IDXGISwapChain> m_swapChain;

	DescriptorHeap m_rtvHeap;
	DescriptorHeap m_dsvHeap;
	DescriptorHeap m_cbvHeap;

	
	DirectX::XMFLOAT3 color1 = { 0.f, 0.960784376f, 0.f };
	DirectX::XMFLOAT3 color2 = { 1, 1, 1 };
	DirectX::XMFLOAT3 color3 = { 1.f, 0.498039246f, 0.313725501f };
	DirectX::XMFLOAT3 color4 = { 1.f, 0.843137324f, 0.f };
	DirectX::XMFLOAT3 color5 = { 0.254901975f, 0.411764741f, 0.882353008f };
	DirectX::XMFLOAT3 color6 = { 0.f, 0.972549081f, 0.9f };

	std::vector<Vertex> mesh = {
	{{-0.5f, -0.5f, 0.5f}, {0, 0, 1.0f}, color1}, {{0.5f, 0.5f, 0.5f}, {0, 0, 1.0f}, color1}, {{0.5f, -0.5f, 0.5f}, {0, 0, 1.0f}, color1},
	{{-0.5f, 0.5f, 0.5f}, {0, 0, 1.0f}, color1}, {{0.5f, 0.5f, 0.5f}, {0, 0, 1.0f}, color1}, {{-0.5f, -0.5f, 0.5f}, {0, 0, 1.0f}, color1},

	{{0.5f, 0.5f, 0.5f}, {1.0f, 0, 0.0f}, color2}, {{0.5f, 0.5f, -0.5f}, {1.0f, 0, 0.0f}, color2}, {{0.5f, -0.5f, 0.5f}, {1.0f, 0, 0.0f}, color2},
	{{0.5f, -0.5f, 0.5f}, {1.0f, 0, 0.0f}, color2}, {{0.5f, 0.5f, -0.5f}, {1.0f, 0, 0.0f}, color2}, {{0.5f, -0.5f, -0.5f}, {1.0f, 0, 0.0f}, color2},
	
	{{-0.5f, 0.5f, -0.5f}, {-1.0f, 0, 0.0f}, color3}, {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0, 0.0f}, color3}, {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0, 0.0f}, color3},
	{{-0.5f, 0.5f, -0.5f}, {-1.0f, 0, 0.0f}, color3}, {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0, 0.0f}, color3}, {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0, 0.0f}, color3},
	
	{{0.5f, 0.5f, -0.5f}, {0, 0, -1.0f}, color4},  {{-0.5f, -0.5f, -0.5f}, {0, 0, -1.0f}, color4}, {{0.5f, -0.5f, -0.5f}, {0, 0, -1.0f}, color4},
	{{0.5f, 0.5f, -0.5f}, {0, 0, -1.0f}, color4}, {{-0.5f, 0.5f, -0.5f}, {0, 0, -1.0f}, color4}, {{-0.5f, -0.5f, -0.5f}, {0, 0, -1.0f}, color4},

	{{-0.5f, 0.5f, -0.5f}, {0, 1.0f, 0}, color5},  {{0.5f, 0.5f, -0.5f}, {0, 1.0f, 0}, color5}, {{0.5f, 0.5f, 0.5f}, {0, 1.0f, 0}, color5},
	{{-0.5f, 0.5f, -0.5f}, {0, 1.0f, 0}, color5},  {{0.5f, 0.5f, 0.5f}, {0, 1.0f, 0}, color5}, {{-0.5f, 0.5f, 0.5f}, {0, 1.0f, 0}, color5},

	{{-0.5f, -0.5f, -0.5f}, {0, -1.0f, 0}, color6},  {{0.5f, -0.5f, 0.5f}, {0, -1.0f, 0}, color6}, {{0.5f, -0.5f, -0.5f}, {0, -1.0f, 0}, color6},
	{{-0.5f, -0.5f, -0.5f}, {0, -1.0f, 0}, color6},  {{-0.5f, -0.5f, 0.5f}, {0, -1.0f, 0}, color6}, {{0.5f, -0.5f, 0.5f}, {0, -1.0f, 0}, color6},
	};

	DefaultResource m_screenTextures[k_nSwapChainBuffers];
	DefaultResource m_depthTextures[k_nSwapChainBuffers];
	struct CBCameraParams {
		DirectX::XMFLOAT4X4 WorldMatrix;
		DirectX::XMFLOAT4X4 ViewProjMatrix;
		DirectX::XMFLOAT3 LightPosition;
	} cbCameraParams;
	ConstantBuffer<CBCameraParams> m_constantBuffer[k_nSwapChainBuffers];
	DefaultResource m_vertexInputBuffer;

	GraphicsPipeline m_finalRenderPipeline;
	RootSignature m_rootSignature;

	ComPtr<ID3D12Fence> m_framesFence;
	uint64_t m_currentFrame;
	uint64_t m_frameBufferFenceValue[k_nSwapChainBuffers];
	HANDLE m_eventHandle;

	D3D12_VIEWPORT m_screenViewport;
	D3D12_RECT m_screenScissor;

	Camera m_camera;
	float angleX;
	float angleY;
	bool enableRotating = true;
	double rotatingTime = 0;
	double rotateSpeed = 1;
};
}