#include "DX12RenderEngine.h"
#include <utils/Macros.h>

#include <iostream>

namespace neural::graphics {
void DX12RenderEngine::createDXGIFactory()
{
	DX_CALL(CreateDXGIFactory2(0, IID_PPV_ARGS(&m_dxgiFactory)));
}
void DX12RenderEngine::createDevice()
{
	DX_CALL(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_mainDevice)));
}
void DX12RenderEngine::recreateSwapChain()
{
	m_swapChain.Reset();

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swapChainDesc.OutputWindow = m_window;
	swapChainDesc.Windowed = true;
	swapChainDesc.BufferDesc.Width = m_windowWidth;
	swapChainDesc.BufferDesc.Height = m_windowHeight;
	swapChainDesc.BufferDesc.RefreshRate = { .Numerator = 60, .Denominator = 1 };
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc = { .Count = 1, .Quality = 0 };
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DX_CALL(m_dxgiFactory->CreateSwapChain(m_commandQueue.Get(), &swapChainDesc, &m_swapChain));
}
void DX12RenderEngine::createCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc.Priority = 0;
	commandQueueDesc.NodeMask = 0;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	DX_CALL(m_mainDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&m_commandQueue)));
}
void DX12RenderEngine::createCommandAllocators()
{
	for (int i = 0; i < k_nSwapChainBuffers; ++i) {
		DX_CALL(m_mainDevice->
			CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i])));
	}
}
void DX12RenderEngine::initialCommands()
{
	D3D12_RESOURCE_TRANSITION_BARRIER transitionBarrier;
	transitionBarrier.pResource   = m_depthBuffer.Get();
	transitionBarrier.StateBefore = D3D12_RESOURCE_STATE_COMMON;
	transitionBarrier.StateAfter  = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	transitionBarrier.Subresource = 0;

	D3D12_RESOURCE_BARRIER depthBufferBarrier;
	depthBufferBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	depthBufferBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	depthBufferBarrier.Transition = transitionBarrier;
	m_commandList->ResourceBarrier(1, &depthBufferBarrier);
}
void DX12RenderEngine::createCommandListAndSendInitialCommands()
{
	DX_CALL(m_mainDevice->
		CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
	//initialCommands();
	m_commandList->Close();

	//ID3D12CommandList* cmdLists[] = {m_commandList.Get()};
	//m_commandQueue->ExecuteCommandLists(1, cmdLists);
}
ComPtr<ID3D12DescriptorHeap> DX12RenderEngine::createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE a_type, uint32_t a_nDescriptors)
{
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc;
	descriptorHeapDesc.NodeMask = 0;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descriptorHeapDesc.Type = a_type;
	descriptorHeapDesc.NumDescriptors = a_nDescriptors;
	ComPtr<ID3D12DescriptorHeap> heap;
	DX_CALL(m_mainDevice->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&heap)));
	return heap;
}
void DX12RenderEngine::createViews()
{
	m_rtvHeap = createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2);
	m_dsvHeap = createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
	uint32_t rtvSize = m_mainDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	uint32_t dsvSize = m_mainDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapStartHandler = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (int i = 0; i < k_nSwapChainBuffers; ++i) {
		DX_CALL(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_swapChainBuffers[i])));
		m_mainDevice->CreateRenderTargetView(m_swapChainBuffers[i].Get(),
			                                 nullptr, D3D12_CPU_DESCRIPTOR_HANDLE(rtvHeapStartHandler.ptr + i * rtvSize));
		
	}
	
	D3D12_RESOURCE_DESC depthBufferDesc;
	depthBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthBufferDesc.Alignment = 0;
	depthBufferDesc.Width = m_windowWidth;
	depthBufferDesc.Height = m_windowHeight;
	depthBufferDesc.DepthOrArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.SampleDesc = { .Count = 1, .Quality = 0 };
	depthBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthBufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil = { .Depth = 1.0f, .Stencil = 0 };

	D3D12_HEAP_PROPERTIES heapProperties;
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	DX_CALL(m_mainDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &depthBufferDesc,
		                                          D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_PPV_ARGS(&m_depthBuffer)));
	m_mainDevice->CreateDepthStencilView(m_depthBuffer.Get(), nullptr, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_RESOURCE_DESC vertexInputBufferDesc;
	vertexInputBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexInputBufferDesc.Alignment = 0;
	vertexInputBufferDesc.Width = 3 * sizeof(Vertex);
	vertexInputBufferDesc.Height = 1;
	vertexInputBufferDesc.DepthOrArraySize = 1;
	vertexInputBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	vertexInputBufferDesc.MipLevels = 1;
	vertexInputBufferDesc.SampleDesc = { .Count = 1, .Quality = 0 };;
	vertexInputBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	vertexInputBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;


	D3D12_HEAP_PROPERTIES heapProperties2;
	heapProperties2.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProperties2.CreationNodeMask = 1;
	heapProperties2.VisibleNodeMask = 1;
	heapProperties2.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties2.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	DX_CALL(m_mainDevice->CreateCommittedResource(&heapProperties2, D3D12_HEAP_FLAG_NONE, &vertexInputBufferDesc,
		D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(m_vertexInputBuffer.GetAddressOf())));
	
	m_vertexInputBufferView.BufferLocation = m_vertexInputBuffer->GetGPUVirtualAddress();
	m_vertexInputBufferView.SizeInBytes = 3 * sizeof(Vertex);
	m_vertexInputBufferView.StrideInBytes = sizeof(Vertex);
}
void DX12RenderEngine::createFence()
{
	m_mainDevice->CreateFence(m_currentFrame, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_framesFence));
}
void DX12RenderEngine::init(HWND a_window, int a_width, int a_height)
{
#if defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		DX_CALL(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif
	m_window       = a_window;
	m_windowWidth  = a_width;
	m_windowHeight = a_height;
	m_currentFrame = 0; 
	createDXGIFactory();
	createDevice();
	createCommandQueue();
	recreateSwapChain();
	createViews();
	createCommandAllocators();
	createCommandListAndSendInitialCommands();
	createFence();
	m_screenViewport.TopLeftX = 0;
	m_screenViewport.TopLeftY = 0;
	m_screenViewport.Width = static_cast<float>(m_windowWidth);
	m_screenViewport.Height = static_cast<float>(m_windowHeight);
	m_screenViewport.MinDepth = 0;
	m_screenViewport.MaxDepth = 1;
	
	m_screenScissor.left = 0;
	m_screenScissor.top = 0;
	m_screenScissor.right = m_windowWidth;
	m_screenScissor.bottom = m_windowHeight;

	m_eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
	for (int i = 0; i < k_nSwapChainBuffers; ++i) {
		m_frameBufferFenceValue[i] = 0;
	}

	
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

	auto& currentBuffer = m_swapChainBuffers[currentFrameBufferIndex];

	D3D12_RESOURCE_TRANSITION_BARRIER transitionBarrier;
	transitionBarrier.pResource = currentBuffer.Get();
	transitionBarrier.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	transitionBarrier.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	transitionBarrier.Subresource = 0;

	D3D12_RESOURCE_BARRIER bufferBarrier;
	bufferBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	bufferBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	bufferBarrier.Transition = transitionBarrier;
	m_commandList->ResourceBarrier(1, &bufferBarrier);

	m_commandList->RSSetViewports(1, &m_screenViewport);
	m_commandList->RSSetScissorRects(1, &m_screenScissor);
}

void DX12RenderEngine::endFrame()
{
	const uint64_t currentFrameBufferIndex = m_currentFrame % k_nSwapChainBuffers;
	auto& currentBuffer = m_swapChainBuffers[currentFrameBufferIndex];
	D3D12_RESOURCE_TRANSITION_BARRIER transitionBarrier;
	transitionBarrier.pResource = currentBuffer.Get();
	transitionBarrier.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	transitionBarrier.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	transitionBarrier.Subresource = 0;

	D3D12_RESOURCE_BARRIER bufferBarrier;
	bufferBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	bufferBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	bufferBarrier.Transition = transitionBarrier;
	m_commandList->ResourceBarrier(1, &bufferBarrier);
	DX_CALL(m_commandList->Close());
	ID3D12CommandList* cmdLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(1, cmdLists);

	m_commandQueue->Signal(m_framesFence.Get(), m_currentFrame);

	m_frameBufferFenceValue[currentFrameBufferIndex] = m_currentFrame;
	++m_currentFrame;
	DX_CALL(m_swapChain->Present(0, 0));
	//DX_CALL(m_mainDevice->GetDeviceRemovedReason());
}

void DX12RenderEngine::render(double a_dt)
{
	beginFrame();
	const uint64_t currentFrameBufferIndex = m_currentFrame % k_nSwapChainBuffers;
	uint32_t rtvSize = m_mainDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	auto rtvHeapStartHandler = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	auto dsvHeapStartHandler = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
	float color[] = { 1, 0, 0, 1 };
	auto currentBufferView = D3D12_CPU_DESCRIPTOR_HANDLE(rtvHeapStartHandler.ptr + currentFrameBufferIndex * rtvSize);
	m_commandList->ClearRenderTargetView(currentBufferView,
		color, 0, nullptr);
	m_commandList->OMSetRenderTargets(1, &currentBufferView, true, &dsvHeapStartHandler);
	endFrame();
}

void DX12RenderEngine::shutdown()
{
	flushFrameBuffers();
	for (int i = 0; i < k_nSwapChainBuffers; ++i) {
		m_commandAllocators[i]->Reset();
	}
	m_mainDevice->RemoveDevice(); // костыль чтобы не выбрасывались исключения
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