#pragma once
#include "graphics/IRenderEngine.h"
#include "GraphicsPipeline.h"
#include "RootSignature.h"
#include "UploadBuffer.h"
#include "DescriptorHeap.h"
#include "Resource.h"

#include "CommonGraphicsHeaders.h"
#include <DirectXMath.h>

#include <memory>

using Microsoft::WRL::ComPtr;
namespace neural::graphics {

class DX12RenderEngine : public IRenderEngine {
public:
	struct Vertex {
		DirectX::XMFLOAT2 position;
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
	DescriptorHeap::Handle m_cbvHandles[k_nSwapChainBuffers];

	Resource m_screenTextures[k_nSwapChainBuffers];
	Resource m_depthTextures[k_nSwapChainBuffers];
	UploadBuffer<DirectX::XMFLOAT4X4> m_constantBuffer[k_nSwapChainBuffers];

	std::vector<Vertex> mesh = { {{-0.5f, 0.f}}, {{0.f, 0.f}}, {{0.f, -0.5f}},
							 {{0.f, 0.f}}, {{0.f, 1.f}}, {{0.5f, 0.f}} };
	UploadBuffer<Vertex> m_uploadBuffer;
	ComPtr<ID3D12Resource> m_vertexInputBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexInputBufferView;

	ComPtr<ID3D12Fence> m_framesFence;
	uint64_t m_currentFrame;
	uint64_t m_frameBufferFenceValue[k_nSwapChainBuffers];
	HANDLE m_eventHandle;

	D3D12_VIEWPORT m_screenViewport;
	D3D12_RECT m_screenScissor;

	GraphicsPipeline m_finalRenderPipeline;
	RootSignature m_rootSignature;
};
}