#pragma once
#include "graphics/IRenderEngine.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;
namespace neural::graphics {
class DX12RenderEngine : public IRenderEngine {
	struct Vertex {
		int a;
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
	ComPtr<ID3D12Resource> m_swapChainBuffers[k_nSwapChainBuffers];
	ComPtr<ID3D12Resource> m_depthBuffer;
	ComPtr<ID3D12Resource> m_vertexInputBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexInputBufferView;
	ComPtr<ID3D12Fence> m_framesFence;
	uint64_t m_currentFrame;
	uint64_t m_frameBufferFenceValue[k_nSwapChainBuffers];
	D3D12_VIEWPORT m_screenViewport;
	D3D12_RECT m_screenScissor;
	HANDLE m_eventHandle;
	void createDXGIFactory();
	void createDevice();
	void recreateSwapChain();
	void createCommandQueue();
	void createCommandAllocators();
	void initialCommands();
	void createCommandListAndSendInitialCommands();
	ComPtr<ID3D12DescriptorHeap> createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE a_type, uint32_t a_nDescriptors);
	void createViews();
	void createFence();
	void flushFrameBuffers();
	void beginFrame();
	void endFrame();
public:
	void init(HWND a_window, int a_width, int a_height) override;
	void render(double a_dt) override;
	void shutdown() override;
};
}