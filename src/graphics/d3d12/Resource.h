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
public:
	struct HeapInfo {
		ID3D12Heap* heap = nullptr;
		uint64_t offset;
	};

	struct CreateInfo {
		D3D12_RESOURCE_DESC resourceDesc;
		D3D12_RESOURCE_STATES initialState;
		std::optional<D3D12_CLEAR_VALUE> clearValue;
		std::optional<HeapInfo> heapInfo;                       // has value means need use CreatePlacedResource
	};

	void initialize(ID3D12Device* a_device, ID3D12Resource* a_resource);

	void initialize(ID3D12Device* a_device, const CreateInfo& a_info);

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

	ID3D12Resource* getID3D12Resource() {
		return m_resource.Get();
	}
private:
	// TODO: too many memory need mb rewrite later
	std::unordered_map<std::string, DescriptorHeap::Handle> m_handles;


	ComPtr<ID3D12Resource> m_resource;

	DescriptorHeap* m_rtvHeap = nullptr;
	DescriptorHeap* m_srvHeap = nullptr;
	DescriptorHeap* m_dsvHeap = nullptr;

	ID3D12Device* m_device;
};
}