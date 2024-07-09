#pragma once

#include <utils/Macros.h>

#include "CommonGraphicsHeaders.h"

#include <vector>
#include <memory>

namespace neural::graphics {

class DescriptorHeap 
{
	struct DescriptorHandler 
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cpu;
		D3D12_GPU_DESCRIPTOR_HANDLE gpu;
	};
	DescriptorHeap();
	void initialize(ID3D12Device8* a_device, 
	                D3D12_DESCRIPTOR_HEAP_TYPE a_type, uint32_t a_capacity, bool isShaderVisiable);
	DescriptorHandler allocate();
	void deallocateDeferred(DescriptorHandler& a_viewHandler);
	void deallocate();
private:
	ComPtr<ID3D12DescriptorHeap> m_heap;
	DescriptorHandler m_startHeapAddress;
	uint32_t m_viewSize;

	//std::vector<uint32_t> m_indicesToDefferedDeallocate[k_nFrames];
	std::unique_ptr<uint32_t[]> m_freeIndices;
	std::uint32_t m_heapSize;
	DEBUG_LINE(std::uint32_t m_heapCapacity);
};
}