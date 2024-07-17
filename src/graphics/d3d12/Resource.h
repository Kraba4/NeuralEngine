#pragma once

#include "CommonGraphicsHeaders.h"
#include <utils/Macros.h>
#include "DescriptorHeap.h"

#include <optional>
#include <unordered_map>
#include <vector>

namespace neural::graphics {
class Resource {
	// dont use this substrings in views names
	static constexpr auto k_metaNameRTV = "r@v*";
	static constexpr auto k_metaNameSRV = "s@v*";
	static constexpr auto k_metaNameDSV = "d@v*";
	static constexpr auto k_metaNameCBV = "c@v*";
public:
	struct HeapInfo {
		ID3D12Heap* heap = nullptr;
		uint64_t offset;
	};

	struct CreateInfo {
		D3D12_RESOURCE_DESC resourceDesc;
		D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;
		std::optional<D3D12_CLEAR_VALUE> clearValue;
		std::optional<HeapInfo> heapInfo;                       // has value means need use CreatePlacedResource
	};

	void setHeapForRenderTargetViews(DescriptorHeap* a_descriptorHeap) {
		assert(a_descriptorHeap);
		m_rtvHeap = a_descriptorHeap;
	}
	void setHeapForShaderResourceViews(DescriptorHeap* a_descriptorHeap) {
		assert(a_descriptorHeap);
		m_srvHeap = a_descriptorHeap;
	}
	void setHeapForDepthStencilViews(DescriptorHeap* a_descriptorHeap) {
		assert(a_descriptorHeap);
		m_dsvHeap = a_descriptorHeap;
	}

	void addRenderTargetView  (std::string a_viewName, std::optional<D3D12_RENDER_TARGET_VIEW_DESC> a_rtvDesc = std::nullopt);
	void addShaderResourceView(std::string a_viewName, std::optional<D3D12_SHADER_RESOURCE_VIEW_DESC> a_srvDesc = std::nullopt);
	void addDepthStencilView  (std::string a_viewName, std::optional<D3D12_DEPTH_STENCIL_VIEW_DESC> a_dsvDesc = std::nullopt);
	void addConstantBufferView(std::string a_viewName, uint32_t a_elementSize);

	DescriptorHeap::Handle& getRenderTargetView(std::string a_viewName, uint32_t a_mipLevel) {
		a_viewName = k_metaNameRTV + std::to_string(a_mipLevel) + a_viewName;
		assert(m_handles.contains(a_viewName));
		return m_handles[a_viewName];
	}
	DescriptorHeap::Handle& getShaderResourceView(std::string a_viewName) {
		a_viewName = k_metaNameSRV + a_viewName;
		assert(m_handles.contains(a_viewName));
		return m_handles[a_viewName];
	}
	DescriptorHeap::Handle& getDepthStencilView(std::string a_viewName) {
		a_viewName = k_metaNameDSV + a_viewName;
		assert(m_handles.contains(a_viewName));
		return m_handles[a_viewName];
	}
	D3D12_VERTEX_BUFFER_VIEW getVertexBufferView(uint32_t a_elementSize) {
		D3D12_VERTEX_BUFFER_VIEW vertexInputBufferView;
		vertexInputBufferView.BufferLocation = m_resource.Get()->GetGPUVirtualAddress();
		vertexInputBufferView.SizeInBytes = m_resource.Get()->GetDesc().Width;
		vertexInputBufferView.StrideInBytes = a_elementSize;
		return vertexInputBufferView;
	}
	ID3D12Resource* getID3D12Resource() {
		return m_resource.Get();
	}

	virtual ~Resource() {};

	virtual void initialize(ID3D12Device* a_device, ID3D12Resource* a_resource) = 0;
	virtual void initialize(ID3D12Device* a_device, const CreateInfo& a_info) = 0;
protected:
	void initializeInternal(ID3D12Device* a_device, const CreateInfo& a_info);

	// TODO: too many memory need mb rewrite later
	std::unordered_map<std::string, DescriptorHeap::Handle> m_handles;


	ComPtr<ID3D12Resource> m_resource;

	DescriptorHeap* m_rtvHeap = nullptr;
	DescriptorHeap* m_srvHeap = nullptr;
	DescriptorHeap* m_dsvHeap = nullptr;

	ID3D12Device* m_device;
	D3D12_HEAP_TYPE m_heapType;
};

class DefaultResource;
template<typename T>
class ConstantBuffer;

class UploadResource : public Resource {
public:
	UploadResource() {
		m_heapType = D3D12_HEAP_TYPE_UPLOAD;
		m_dataSize = 0;
		m_mappedData = nullptr;
	}
	void initialize(ID3D12Device* a_device, ID3D12Resource* a_resource) override {
		m_device = a_device;
		m_resource = a_resource;
	}
	void initialize(ID3D12Device* a_device, const CreateInfo& a_info) override {
		assert(a_device);
		initializeInternal(a_device, a_info);
		m_dataSize = m_resource.Get()->GetDesc().Width; // TODO work only for buffer
		DX_CALL(m_resource.Get()->Map(0, nullptr, &m_mappedData));
	}
	void uploadData(void* a_data) {
		assert(m_dataSize != 0);
		assert(m_device);
		assert(m_resource);
		assert(m_mappedData);
		memcpy(m_mappedData, a_data, m_dataSize);
	}

	~UploadResource() {
		if (m_resource != nullptr)
			m_resource->Unmap(0, nullptr);
		m_mappedData = nullptr;
	}

private:
	friend class DefaultResource;
	template<typename T>
	friend class ConstantBuffer;
	uint32_t m_dataSize;
	void* m_mappedData;
};

class DefaultResource : public Resource {
public:
	DefaultResource() {
		m_heapType = D3D12_HEAP_TYPE_DEFAULT;
		DEBUG_LINE(m_uploadHelperCreated = false);
	}
	void initialize(ID3D12Device* a_device, ID3D12Resource* a_resource) override {
		m_device = a_device;
		m_resource = a_resource;
	}
	void initialize(ID3D12Device* a_device, const CreateInfo& a_info) override {
		assert(a_device);
		initializeInternal(a_device, a_info);
	}
	void initializeUpload(ID3D12Device* a_device, std::optional<HeapInfo> a_heapInfo = std::nullopt) {
		DEBUG_LINE(m_uploadHelperCreated = true);
		m_uploadHelper.initialize(a_device, {
			.resourceDesc = m_resource.Get()->GetDesc(),
			.initialState = D3D12_RESOURCE_STATE_GENERIC_READ,
			.heapInfo = a_heapInfo
			});
	}
	void uploadData(ID3D12GraphicsCommandList* a_commandList, void* a_data) {
		assert(m_device);
		assert(a_commandList);
		assert(m_uploadHelper.m_device == m_device);
		m_uploadHelper.uploadData(a_data);
		a_commandList->CopyBufferRegion(m_resource.Get(), 0, m_uploadHelper.m_resource.Get(), 0, m_uploadHelper.m_dataSize);
	}

	~DefaultResource() {}
private:
	UploadResource m_uploadHelper;
	DEBUG_LINE(bool m_uploadHelperCreated);
};

template<typename T>
class ConstantBuffer {
public:
	void initialize(ID3D12Device* a_device, DescriptorHeap* a_cbvHeap,
		            uint32_t a_size, std::optional<Resource::HeapInfo> a_heapInfo = std::nullopt) {
		assert(a_device);

		m_size = a_size;
		m_elementSize = CalcConstantBufferByteSize(sizeof(T));

		m_uploadResource.initialize(a_device, {
			.resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_elementSize * a_size),
			.initialState = D3D12_RESOURCE_STATE_GENERIC_READ,
			.heapInfo = a_heapInfo
			});

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = m_uploadResource.getID3D12Resource()->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = m_elementSize;

		m_viewHandle = a_cbvHeap->allocate();
		a_device->CreateConstantBufferView(&cbvDesc, m_viewHandle.cpu);
	}

	void uploadData(int a_elementIndex, const T& a_data) {
		memcpy(&m_uploadResource.m_mappedData[a_elementIndex * m_elementSize], &a_data, sizeof(T));
	}

	void uploadData(T* a_data) {
		memcpy(m_uploadResource.m_mappedData, a_data, m_size * m_elementSize);
	}

	ID3D12Resource* getID3D12Resource() const {
		return m_uploadResource.m_resource.Get();
	}
	DescriptorHeap::Handle& getConstantBufferView() {
		return m_viewHandle;
	}
private: 
	uint32_t CalcConstantBufferByteSize(uint32_t a_byteSize) {
		return (a_byteSize + 255) & ~255;
	}
	uint32_t m_elementSize;
	uint32_t m_size;
	UploadResource m_uploadResource;
	DescriptorHeap::Handle m_viewHandle;
};
}