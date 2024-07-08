#include "DX12RenderEngine.h"
#include <utils/Macros.h>

#include <iostream>
#include <cmath>

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

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {
		.BufferDesc = { .Width = m_windowWidth,
						.Height = m_windowHeight,
						.RefreshRate = {.Numerator = 60, .Denominator = 1 },
						.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
						.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
						.Scaling = DXGI_MODE_SCALING_UNSPECIFIED
					  },
		.SampleDesc = { .Count = 1, 
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
	CD3DX12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(m_depthBuffer.Get(), 
		                                      D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE),
		CD3DX12_RESOURCE_BARRIER::Transition(m_vertexInputBuffer.Get(),
											  D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST)
	};
	m_commandList->ResourceBarrier(std::size(barriers), barriers);

	m_commandList->CopyBufferRegion(m_vertexInputBuffer.Get(), 0, m_uploadBuffer.getResource(), 0, mesh.size() * sizeof(Vertex));
}

void DX12RenderEngine::createCommandListAndSendInitialCommands()
{
	DX_CALL(m_mainDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		                   m_commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
	initialCommands();
	m_commandList->Close();

	ID3D12CommandList* cmdLists[] = {m_commandList.Get()};
	m_commandQueue->ExecuteCommandLists(1, cmdLists);
	m_commandQueue->Signal(m_framesFence.Get(), 1);
}

ComPtr<ID3D12DescriptorHeap> DX12RenderEngine::createDescriptorHeap(uint32_t a_nDescriptors,
	D3D12_DESCRIPTOR_HEAP_TYPE a_type, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
{
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {
		.Type = a_type,
		.NumDescriptors = a_nDescriptors,
		.Flags = flags,
		.NodeMask = 0,
	};
	ComPtr<ID3D12DescriptorHeap> heap;
	DX_CALL(m_mainDevice->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&heap)));
	return heap;
}

void DX12RenderEngine::createViews()
{
	m_rtvHeap = createDescriptorHeap(2, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_dsvHeap = createDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_cbvHeap = createDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	uint32_t rtvSize = m_mainDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	uint32_t dsvSize = m_mainDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	uint32_t cbvSize = m_mainDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
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
	vertexInputBufferDesc.Width = mesh.size() * sizeof(Vertex);
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
	m_vertexInputBufferView.SizeInBytes = mesh.size() * sizeof(Vertex);
	m_vertexInputBufferView.StrideInBytes = sizeof(Vertex);

	m_uploadBuffer.initialize(m_mainDevice.Get(), mesh.size(), false);
	m_uploadBuffer.uploadData(mesh.data());

	m_constantBuffer.initialize(m_mainDevice.Get(), 1, true, m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
}

void DX12RenderEngine::createFence()
{
	m_mainDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_framesFence));
}

void DX12RenderEngine::initializePipelines()
{
	m_rootSignature.initialize(m_mainDevice.Get(),
		{ {.parameterType = RootSignature::RootParameterType::DescriptorTable,
			.descriptorTable = {.descriptorType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
								 .numDescriptors = 1,
								 .baseShaderRegister = 0}
			},
		});

	m_finalRenderPipeline.initialize(m_mainDevice.Get(), std::string_view("final_render"),
		GraphicsPipeline::CreateInfo{
			.rootSignature = m_rootSignature,
			.inputLayout = {
				{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			},
			.vertexShaderPath = "../resources/shaders/compiled/1.vs.cso",
			.pixelShaderPath  = "../resources/shaders/compiled/1.ps.cso",
			.RTVFormats = {DXGI_FORMAT_R8G8B8A8_UNORM},
			.DSVFormat = DXGI_FORMAT_D32_FLOAT
		});
}

void DX12RenderEngine::initialize(HWND a_window, int a_width, int a_height)
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
	m_currentFrame = 2; 
	createDXGIFactory();
	createDevice();
	createCommandQueue();
	recreateSwapChain();
	createViews();
	createCommandAllocators();
	createFence();
	createCommandListAndSendInitialCommands();
	initializePipelines();
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
		m_frameBufferFenceValue[i] = 1;
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

	auto currentBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(currentBuffer.Get(),
		                                            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &currentBufferBarrier);
}

void DX12RenderEngine::endFrame()
{
	const uint64_t currentFrameBufferIndex = m_currentFrame % k_nSwapChainBuffers;
	auto& currentBuffer = m_swapChainBuffers[currentFrameBufferIndex];
	auto currentBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(currentBuffer.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_commandList->ResourceBarrier(1, &currentBufferBarrier);
	DX_CALL(m_commandList->Close());
	ID3D12CommandList* cmdLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(1, cmdLists);

	m_commandQueue->Signal(m_framesFence.Get(), m_currentFrame);

	m_frameBufferFenceValue[currentFrameBufferIndex] = m_currentFrame;
	++m_currentFrame;
	DX_CALL(m_swapChain->Present(0, 0));
	//DX_CALL(m_mainDevice->GetDeviceRemovedReason());
}

void DX12RenderEngine::render(const Timer& a_timer)
{
	float scale = std::fmod(static_cast<float>(a_timer.getLastTime()), 2.0f);
	m_constantBuffer.uploadData(0, DirectX::XMFLOAT4X4(scale, 0, 0, 0,
													   0, scale, 0, 0,
		                                               0, 0, 1, 0,
		                                               0, 0, 0, 1));
	beginFrame();
	const uint64_t currentFrameBufferIndex = m_currentFrame % k_nSwapChainBuffers;
	uint32_t rtvSize = m_mainDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	auto rtvHeapStartHandler = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	auto dsvHeapStartHandler = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
	float color[] = { 0, 0, 0, 1 };
	auto currentBufferView = D3D12_CPU_DESCRIPTOR_HANDLE(rtvHeapStartHandler.ptr + currentFrameBufferIndex * rtvSize);
	m_commandList->RSSetViewports(1, &m_screenViewport);
	m_commandList->RSSetScissorRects(1, &m_screenScissor);
	m_commandList->ClearRenderTargetView(currentBufferView,
		color, 0, nullptr);
	m_commandList->ClearDepthStencilView(dsvHeapStartHandler, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	m_rootSignature.setAsRootSignature(m_commandList);
	m_rootSignature.bindResources(m_commandList);
	m_finalRenderPipeline.setAsPipeline(m_commandList);
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_cbvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	m_commandList->SetGraphicsRootDescriptorTable(0, m_cbvHeap->GetGPUDescriptorHandleForHeapStart()); // ??? getView

	m_commandList->IASetVertexBuffers(0, 1, &m_vertexInputBufferView);
	m_commandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->OMSetRenderTargets(1, &currentBufferView, true, &dsvHeapStartHandler);
	m_commandList->DrawInstanced(mesh.size(), 1, 0, 0);
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