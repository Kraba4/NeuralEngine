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

	
	DirectX::XMFLOAT3 color1 = { 255.0f / 255, 190.0f / 255, 11.0f / 255 };
	DirectX::XMFLOAT3 color2 = { 251.0f / 255, 86.0f / 255, 7.0f / 255 };
	DirectX::XMFLOAT3 color3 = { 255.0f / 255, 0.0f / 255, 110.0f / 255 };
	DirectX::XMFLOAT3 color4 = { 131.0f / 255, 56.0f / 255, 236.0f / 255 };
	DirectX::XMFLOAT3 color5 = { 1, 1, 1};
	DirectX::XMFLOAT3 color6 = { 0.0f, 0.9f, 0.0f };

	DirectX::XMFLOAT3 colorPlane = { 1, 1, 1 };
	std::vector<Vertex> mesh = {
		//cube
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
	
		//plane
	{{-500.0f, 0.0f, -500.f}, {0, 1.0f, 0}, colorPlane},  {{500.f, 0.0f, -500.f}, {0, 1.0f, 0}, colorPlane}, {{500.f, 0.f, 500.f}, {0, 1.0f, 0}, colorPlane},
	{{-500.f, 0.0f, -500.f}, {0, 1.0f, 0}, colorPlane},  {{500.f, 0.0f, 500.f}, {0, 1.0f, 0}, colorPlane}, {{-500.f, 0.f, 500.f}, {0, 1.0f, 0}, colorPlane},

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
};
}