#include "DX12RenderEngine.h"
#include <utils/Macros.h>

namespace neural::graphics {
void DX12RenderEngine::initialize(HWND a_window, int a_width, int a_height)
{
	DEBUG_LINE(ComPtr<ID3D12Debug> debugController);
	DEBUG_LINE(DX_CALL(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))));
	DEBUG_LINE(debugController->EnableDebugLayer());

	m_window = a_window;
	m_windowWidth = a_width;
	m_windowHeight = a_height;
	m_currentFrame = k_nSwapChainBuffers;

	createDXGIFactory();
	createDevice();
	createCommandQueue();
	recreateSwapChain();
	createCommandAllocators();
	createFence();

	m_rtvHeap.initialize(m_mainDevice.Get(), k_nSwapChainBuffers, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 64, false);
	m_dsvHeap.initialize(m_mainDevice.Get(), k_nSwapChainBuffers, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 64, false);
	m_cbvHeap.initialize(m_mainDevice.Get(), k_nSwapChainBuffers, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 64, true);
	initializeResources();
	initializePipelines();
	createCommandListAndSendInitialCommands();

	m_screenViewport = { .TopLeftX = 0,
						 .TopLeftY = 0,
						 .Width = static_cast<float>(m_windowWidth),
						 .Height = static_cast<float>(m_windowHeight),
						 .MinDepth = 0,
						 .MaxDepth = 1
	};
	m_screenScissor = { .left = 0,
						.top = 0,
						.right = static_cast<int32_t>(m_windowWidth),
						.bottom = static_cast<int32_t>(m_windowHeight)
	};

	m_eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
	for (int i = 0; i < k_nSwapChainBuffers; ++i) {
		m_frameBufferFenceValue[i] = 1;
	}

	m_settings.camera.setFrustum(DirectX::XMConvertToRadians(45), static_cast<float>(m_windowWidth) / m_windowHeight, 1, 1000);
}

void DX12RenderEngine::createDXGIFactory()
{
	DX_CALL(CreateDXGIFactory2(0, IID_PPV_ARGS(&m_dxgiFactory)));
}

void DX12RenderEngine::createDevice()
{
	DX_CALL(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_mainDevice)));
	NAME_DX_OBJECT(m_mainDevice, L"MainDevice");
}

void DX12RenderEngine::recreateSwapChain()
{
	m_swapChain.Reset();

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {
		.BufferDesc = {.Width = m_windowWidth,
						.Height = m_windowHeight,
						.RefreshRate = {.Numerator = 60, .Denominator = 1 },
						.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
						.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
						.Scaling = DXGI_MODE_SCALING_UNSPECIFIED
					  },
		.SampleDesc = {.Count = 1,
						.Quality = 0
					  },
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.BufferCount = k_nSwapChainBuffers,
		.OutputWindow = m_window,
		.Windowed = true,
		.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
		.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	};

	DX_CALL(m_dxgiFactory->CreateSwapChain(m_commandQueue.Get(), &swapChainDesc, &m_swapChain));
}

void DX12RenderEngine::createCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {
		.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
		.Priority = 0,
		.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
		.NodeMask = 0
	};

	DX_CALL(m_mainDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&m_commandQueue)));
	NAME_DX_OBJECT(m_commandQueue, L"CommandQueue");
}

void DX12RenderEngine::createCommandAllocators()
{
	for (int i = 0; i < k_nSwapChainBuffers; ++i) {
		DX_CALL(m_mainDevice->
			CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i])));
		NAME_DX_OBJECT_INDEXED(m_commandAllocators[i], L"CommandAllocator", i);
	}
}

void DX12RenderEngine::initialCommands()
{
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_vertexInputBuffer.getID3D12Resource(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	m_commandList->ResourceBarrier(1, &barrier);

	m_vertexInputBuffer.uploadData(m_commandList.Get(), mesh.data());
}


void DX12RenderEngine::createCommandListAndSendInitialCommands()
{
	DX_CALL(m_mainDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
	NAME_DX_OBJECT(m_commandList, L"CommandList");
	initialCommands();
	m_commandList->Close();

	ID3D12CommandList* cmdLists[] = {m_commandList.Get()};
	m_commandQueue->ExecuteCommandLists(1, cmdLists);
	m_commandQueue->Signal(m_framesFence.Get(), 1);
}

void DX12RenderEngine::createFence()
{
	m_mainDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_framesFence));
	NAME_DX_OBJECT(m_framesFence, L"FrameFence");
}

void DX12RenderEngine::beginFrame()
{
	const uint64_t currentFrameBufferIndex = m_currentFrame % k_nSwapChainBuffers;

	// wait until the gpu frees the next buffer
	const uint64_t currentFrameBufferFenceValue = m_frameBufferFenceValue[currentFrameBufferIndex];
	if (m_framesFence->GetCompletedValue() < currentFrameBufferFenceValue) {
		DX_CALL(m_framesFence->SetEventOnCompletion(currentFrameBufferFenceValue, m_eventHandle));
		WaitForSingleObject(m_eventHandle, INFINITE);
	}

	// reset command allocator and command list, open command list
	auto& currentCommandAllocator = m_commandAllocators[currentFrameBufferIndex];
	DX_CALL(currentCommandAllocator->Reset());
	DX_CALL(m_commandList->Reset(currentCommandAllocator.Get(), nullptr));

	auto& currentBuffer = m_screenTextures[currentFrameBufferIndex];

	auto currentBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(currentBuffer.getID3D12Resource(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &currentBufferBarrier);

	m_commandList->RSSetViewports(1, &m_screenViewport);
	m_commandList->RSSetScissorRects(1, &m_screenScissor);
}

void DX12RenderEngine::endFrame()
{
	const uint64_t currentFrameBufferIndex = m_currentFrame % k_nSwapChainBuffers;
	auto& currentBuffer = m_screenTextures[currentFrameBufferIndex];
	auto currentBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(currentBuffer.getID3D12Resource(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_commandList->ResourceBarrier(1, &currentBufferBarrier);
	DX_CALL(m_commandList->Close());
	ID3D12CommandList* cmdLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(1, cmdLists);

	m_commandQueue->Signal(m_framesFence.Get(), m_currentFrame);

	m_frameBufferFenceValue[currentFrameBufferIndex] = m_currentFrame;
	++m_currentFrame;
	DX_CALL(m_swapChain->Present(0, 0));
	DX_CALL(m_mainDevice->GetDeviceRemovedReason());
}

void DX12RenderEngine::shutdown()
{
	//m_commandList->Close();
	//m_commandList = nullptr;
	flushFrameBuffers();
	//m_commandQueue = nullptr;
	//m_swapChain->Release();
	//for (int i = 0; i < k_nSwapChainBuffers; ++i) {
	//	//m_commandAllocators[i]->Reset();
	//	m_swapChainBuffers[i].Detach();
	//	//m_swapChainBuffers[i].Reset();
	//	m_swapChainBuffers[i] = nullptr;
	//}
	m_mainDevice->RemoveDevice(); // костыль чтобы не выбрасывались исключения, пока не знаю как починить
}

void DX12RenderEngine::flushFrameBuffers()
{
	for (int i = 0; i < k_nSwapChainBuffers; ++i) {
		// wait until the gpu frees the next buffer
		const uint64_t currentFrameBufferFenceValue = m_frameBufferFenceValue[i];
		if (m_framesFence->GetCompletedValue() < currentFrameBufferFenceValue) {
			DX_CALL(m_framesFence->SetEventOnCompletion(currentFrameBufferFenceValue, m_eventHandle));
			WaitForSingleObject(m_eventHandle, INFINITE);
		}
	}
}
}