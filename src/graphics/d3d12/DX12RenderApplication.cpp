#include "DX12RenderEngine.h"
#include <utils/Macros.h>

#include <iostream>
#include <cmath>

namespace neural::graphics {
void DX12RenderEngine::initializeResources()
{
	for (int i = 0; i < k_nSwapChainBuffers; ++i) {
		ID3D12Resource* swapchainBuffer;
		DX_CALL(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&swapchainBuffer)));
		m_screenTextures[i].initialize(m_mainDevice.Get(), swapchainBuffer);
		NAME_DX_OBJECT_INDEXED(m_screenTextures[i].getID3D12Resource(), L"SwapchainBuffer", i);

		m_screenTextures[i].setHeapForRenderTargetViews(&m_rtvHeap);
		m_screenTextures[i].addRenderTargetView("default");
	}

	for (int i = 0; i < k_nSwapChainBuffers; ++i)
	{
		m_depthTextures[i].initialize(m_mainDevice.Get(),
			{
				.resourceDesc = {
					.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
					.Alignment = 0,
					.Width = m_windowWidth,
					.Height = m_windowHeight,
					.DepthOrArraySize = 1,
					.MipLevels = 1,
					.Format = DXGI_FORMAT_D32_FLOAT,
					.SampleDesc = {.Count = 1, .Quality = 0 },
					.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
					.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
				},
				.initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE,
				.clearValue = D3D12_CLEAR_VALUE {
					.Format = DXGI_FORMAT_D32_FLOAT,
					.DepthStencil = {.Depth = 1.0f, .Stencil = 0}
				}
			});
		NAME_DX_OBJECT_INDEXED(m_depthTextures[i].getID3D12Resource(), L"DepthBuffer", i);

		m_depthTextures[i].setHeapForDepthStencilViews(&m_dsvHeap);
		m_depthTextures[i].addDepthStencilView("default");
	}

	for (int i = 0; i < k_nSwapChainBuffers; ++i)
	{
		m_constantBuffer[i].initialize(m_mainDevice.Get(), &m_cbvHeap, 1);
		NAME_DX_OBJECT_INDEXED(m_constantBuffer[i].getID3D12Resource(), L"ConstantBuffer", i);
	}

	const aiScene* scene = m_assetImporter.ReadFile(RESOURCES"/models/Cat_Sitting.fbx",
		aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);
	const aiMesh* mesh = scene->mMeshes[0];

	for (int i = 0; i < mesh->mNumVertices; ++i) {
		m_mainVertices.push_back({
			{ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z },
			{ mesh->mNormals[i].x,  mesh->mNormals[i].y,  mesh->mNormals[i].z },
			{ 1, 1}
			});
	}

	for (int i = 0; i < mesh->mNumFaces; ++i) {
		const auto face = mesh->mFaces[i];
		assert(face.mNumIndices == 3);
		m_mainIndices.push_back(face.mIndices[0]);
		m_mainIndices.push_back(face.mIndices[1]);
		m_mainIndices.push_back(face.mIndices[2]);
	}

	m_vertexInputBuffer.initialize(m_mainDevice.Get(), {
			.resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(m_mainVertices[0]) * m_mainVertices.size()),
		});
	NAME_DX_OBJECT(m_vertexInputBuffer.getID3D12Resource(), L"VertexInputBuffer");

	m_indexInputBuffer.initialize(m_mainDevice.Get(),{
			.resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(m_mainIndices[0]) * m_mainIndices.size())
		});
	NAME_DX_OBJECT(m_indexInputBuffer.getID3D12Resource(), L"IndexInputBuffer");

	m_vertexInputBuffer.initializeUpload(m_mainDevice.Get());
	m_indexInputBuffer.initializeUpload(m_mainDevice.Get());
}

void DX12RenderEngine::initializePipelines()
{
	m_rootSignature.initialize(m_mainDevice.Get(),
		{
			{
				.parameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,
				.constants = {.baseShaderRegister = 0, .num32BitValues = 1}
			},
			{
				.parameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
				.descriptor = {.baseShaderRegister = 1}
			},
		});
	NAME_DX_OBJECT(m_rootSignature.getID3D12RootSignature(), L"RootSignature");

	m_finalRenderPipeline.initialize(m_mainDevice.Get(), std::string_view("final_render"),
		GraphicsPipeline::CreateInfo{
			.rootSignature = m_rootSignature,
			.inputLayout = {
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			},
			.vertexShaderPath = D3D12_ROOT"/shaders/compiled/1.vs.cso",
			.pixelShaderPath  = D3D12_ROOT"/shaders/compiled/1.ps.cso",
			.RTVFormats = {DXGI_FORMAT_R8G8B8A8_UNORM},
			.DSVFormat = DXGI_FORMAT_D32_FLOAT
		});
	NAME_DX_OBJECT(m_finalRenderPipeline.getID3D12Pipeline(), L"RenderPipeline");
}

void DX12RenderEngine::initialCommands()
{
	D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(m_vertexInputBuffer.getID3D12Resource(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST),
		CD3DX12_RESOURCE_BARRIER::Transition(m_indexInputBuffer.getID3D12Resource(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST)
	};
	m_commandList->ResourceBarrier(_countof(barriers), barriers);

	m_vertexInputBuffer.uploadData(m_commandList.Get(), m_mainVertices.data());
	m_indexInputBuffer.uploadData(m_commandList.Get(), m_mainIndices.data());
}


void DX12RenderEngine::render(const Timer& a_timer)
{
	beginFrame();
	const uint64_t currentFrameBufferIndex = m_currentFrame % k_nSwapChainBuffers;
	m_settings.camera.updateViewMatrix();
	m_cbCameraParams.LightPosition = { 0, 20, 0 };
	if (m_settings.enableRotating) {
		m_settings.rotatingTimeX += a_timer.getLastDeltaTime() * m_settings.rotateSpeedX;
		m_settings.rotatingTimeY += a_timer.getLastDeltaTime() * m_settings.rotateSpeedY;
	}
	//float angleX = std::fmod(static_cast<float>(m_settings.rotatingTimeX), DirectX::XM_2PI);
	//float angleY = std::fmod(static_cast<float>(m_settings.rotatingTimeY), DirectX::XM_2PI);
	float angleX = DirectX::XMConvertToRadians(-90);
	float angleY = std::fmod(static_cast<float>(m_settings.rotatingTimeY), DirectX::XM_2PI);
	DirectX::XMMATRIX rotation = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationX(angleX),
		DirectX::XMMatrixRotationY(angleY));
	DirectX::XMMATRIX cubeWorld = DirectX::XMMatrixMultiplyTranspose(
		rotation, DirectX::XMMatrixTranslation(0, 3, 10));
	cubeWorld = DirectX::XMMatrixMultiply(cubeWorld, DirectX::XMMatrixScaling(0.5, 0.5, 0.5));
	DirectX::XMStoreFloat4x4(&m_cbCameraParams.WorldMatrix, cubeWorld);
	DirectX::XMStoreFloat4x4(&m_cbCameraParams.ViewProjMatrix, DirectX::XMMatrixMultiplyTranspose(m_settings.camera.getView(), m_settings.camera.getProj()));
	m_constantBuffer[currentFrameBufferIndex].uploadData(&m_cbCameraParams);

	auto currentBufferView = m_screenTextures[currentFrameBufferIndex].getRenderTargetView("default", 0);
	auto& currentDepthBufferView = m_depthTextures[currentFrameBufferIndex].getDepthStencilView("default");
	float color[] = { 0, 0, 0, 1 };
	m_commandList->ClearRenderTargetView(currentBufferView.cpu, color, 0, nullptr);
	m_commandList->ClearDepthStencilView(currentDepthBufferView.cpu,
		                                 D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	m_commandList->SetGraphicsRootSignature(m_rootSignature.getID3D12RootSignature());
	m_commandList->SetPipelineState(m_finalRenderPipeline.getID3D12Pipeline());

	//ID3D12DescriptorHeap* descriptorHeaps[] = { m_cbvHeap.getID3D12DescriptorHeap()};
	//m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	//m_commandList->SetGraphicsRootDescriptorTable(0, m_constantBuffer[currentFrameBufferIndex].getConstantBufferView().gpu);
	
	m_commandList->SetGraphicsRoot32BitConstant(0, 0, 0);
	m_commandList->SetGraphicsRootConstantBufferView(1,
		m_constantBuffer[currentFrameBufferIndex].getID3D12Resource()->GetGPUVirtualAddress());

	auto vertexBufferView = m_vertexInputBuffer.getVertexBufferView(sizeof(Vertex));
	auto indexBufferView = m_indexInputBuffer.getIndexBufferView();
	m_commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	m_commandList->IASetIndexBuffer(&indexBufferView);
	m_commandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_commandList->OMSetRenderTargets(1, &currentBufferView.cpu, true, &currentDepthBufferView.cpu);
	m_commandList->DrawIndexedInstanced(m_mainIndices.size(), 1, 0, 0, 0);

	//DirectX::XMStoreFloat4x4(&cbCameraParams.WorldMatrix, DirectX::XMMatrixIdentity());

	//m_commandList->SetGraphicsRoot32BitConstant(0, 1, 0);
	//m_commandList->DrawInstanced(6, 1, mesh.size() - 6, 0);
	endFrame();
}
}