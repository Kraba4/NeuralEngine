#pragma once
#include "graphics/IRenderEngine.h"
#include "GraphicsPipeline.h"
#include "RootSignature.h"
#include "UploadBuffer.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <d3dx12.h>

#include <memory>

using Microsoft::WRL::ComPtr;
namespace neural::graphics {
class DX12RenderEngine : public IRenderEngine {
	struct Vertex {
		DirectX::XMFLOAT2 position;
	};
	static constexpr int k_nSwapChainBuffers = 2;
	HWND m_window;
	int m_windowWidth;
	int m_windowHeight;
	ComPtr<ID3D12Device8> m_mainDevice{ nullptr };
	ComPtr<IDXGIFactory7> m_dxgiFactory{ nullptr };
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[k_nSwapChainBuffers];
	ComPtr<ID3D12GraphicsCommandList>  m_commandList;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<IDXGISwapChain> m_swapChain;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
	ComPtr<ID3D12Resource> m_swapChainBuffers[k_nSwapChainBuffers];
	ComPtr<ID3D12Resource> m_depthBuffer;
	ComPtr<ID3D12Resource> m_vertexInputBuffer;
	ComPtr<ID3D12Resource> m_uploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexInputBufferView;
	ComPtr<ID3D12Fence> m_framesFence;
	uint64_t m_currentFrame;
	uint64_t m_frameBufferFenceValue[k_nSwapChainBuffers];
	D3D12_VIEWPORT m_screenViewport;
	D3D12_RECT m_screenScissor;
	HANDLE m_eventHandle;
	BYTE* m_mappedVertexes;
	std::vector<Vertex> mesh = { {{-0.5f, 0.f}}, {{0.f, 0.f}}, {{0.f, -0.5f}},
								 {{0.f, 0.f}}, {{0.f, 1.f}}, {{0.5f, 0.f}} };
	GraphicsPipeline m_finalRenderPipeline;
	RootSignature m_rootSignature;
	UploadBuffer<DirectX::XMFLOAT4X4> m_constantBuffer;
	void createDXGIFactory();
	void createDevice();
	void recreateSwapChain();
	void createCommandQueue();
	void createCommandAllocators();
	void initialCommands();
	void createCommandListAndSendInitialCommands();
	ComPtr<ID3D12DescriptorHeap> createDescriptorHeap(uint32_t a_nDescriptors,
		D3D12_DESCRIPTOR_HEAP_TYPE a_type, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	void createViews();
	void createFence();
	void flushFrameBuffers();
	void beginFrame();
	void endFrame();
	void initializePipelines();
public:
	void initialize(HWND a_window, int a_width, int a_height) override;
	void render(const Timer& a_timer) override;
	void shutdown() override;
};
}