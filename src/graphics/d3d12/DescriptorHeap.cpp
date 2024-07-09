#include "DescriptorHeap.h"
#include <utils/Macros.h>

namespace neural::graphics {

void DescriptorHeap::initialize(ID3D12Device8* a_device,
	D3D12_DESCRIPTOR_HEAP_TYPE a_type, uint32_t a_capacity, bool isShaderVisiable)
{
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {
	.Type = a_type,
	.NumDescriptors = a_capacity,
	.Flags = isShaderVisiable ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
							  : D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
	.NodeMask = 0,
	};
	DX_CALL(a_device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_heap)));

	m_viewSize = a_device->GetDescriptorHandleIncrementSize(a_type);
	m_startHeapAddress.cpu = m_heap->GetCPUDescriptorHandleForHeapStart();
	m_startHeapAddress.gpu = isShaderVisiable ? m_heap->GetGPUDescriptorHandleForHeapStart()
		: D3D12_GPU_DESCRIPTOR_HANDLE(0);

	m_freeIndices = std::move(std::make_unique<uint32_t[]>(a_capacity));
	DEBUG_LINE(m_heapCapacity = a_capacity);
	m_heapSize = 0;

	for (int i = 0; i < a_capacity; ++i) {
		m_freeIndices[i] = i;
	}
}

DescriptorHeap::DescriptorHandler DescriptorHeap::allocate()
{
	assert(m_heapSize < m_heapCapacity);
	uint32_t freeIndex = m_freeIndices[m_heapSize];
	++m_heapSize;
	SIZE_T addressOffset = freeIndex * m_viewSize;
	return { m_startHeapAddress.cpu.ptr + addressOffset,
			 m_startHeapAddress.gpu.ptr + addressOffset };
}

void DescriptorHeap::deallocateDeferred(DescriptorHandler& a_viewHandler)
{
	assert(m_heapSize > 0);
	assert(a_viewHandler.cpu.ptr != 0);
	assert(a_viewHandler.cpu.ptr > m_startHeapAddress.cpu.ptr);
	SIZE_T addressOffset = a_viewHandler.cpu.ptr - m_startHeapAddress.cpu.ptr;

	uint32_t index = addressOffset / m_viewSize;
	assert(index < m_heapCapacity);

	//--m_heapSize;
	//m_freeIndices[m_heapSize] = index;

	a_viewHandler.cpu.ptr = 0;
	a_viewHandler.gpu.ptr = 0;

	//m_indicesToDefferedDeallocate.push_back(index);
}

void DescriptorHeap::deallocate()
{
	//--m_heapSize;
	//m_freeIndices[m_heapSize] = index;
}

}