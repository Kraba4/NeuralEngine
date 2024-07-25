#include "DescriptorHeap.h"
#include <utils/Macros.h>

namespace neural::graphics {

void DescriptorHeap::initialize(ID3D12Device* a_device, uint32_t a_nFrames,
    D3D12_DESCRIPTOR_HEAP_TYPE a_type, uint32_t a_capacity, bool isShaderVisiable)
{
    assert(!m_heap);

    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {
    .Type = a_type,
    .NumDescriptors = a_capacity,
    .Flags = isShaderVisiable ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
                              : D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
    .NodeMask = 0,
    };
    DX_CALL(a_device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_heap)));

    m_descriptorSize = a_device->GetDescriptorHandleIncrementSize(a_type);
    m_startHeapAddress.cpu = m_heap->GetCPUDescriptorHandleForHeapStart();
    m_startHeapAddress.gpu = isShaderVisiable ? m_heap->GetGPUDescriptorHandleForHeapStart()
        : D3D12_GPU_DESCRIPTOR_HANDLE(0);

    m_freeIndices = std::move(std::make_unique<uint32_t[]>(a_capacity));
    DEBUG_LINE(m_heapCapacity = a_capacity);
    m_heapSize = 0;

    for (uint32_t i = 0; i < a_capacity; ++i)
    {
        m_freeIndices[i] = i;
    }

    m_indicesToDeferredDeallocate = std::move(std::make_unique<std::vector<uint32_t>[]>(a_nFrames));
    DEBUG_LINE(m_nDeferredVectors = a_nFrames);
}

DescriptorHeap::Handle DescriptorHeap::allocate()
{
    assert(m_heapSize < m_heapCapacity);
    uint32_t freeIndex = m_freeIndices[m_heapSize];
    ++m_heapSize;
    SIZE_T addressOffset = freeIndex * m_descriptorSize;
    return { m_startHeapAddress.cpu.ptr + addressOffset,
             m_startHeapAddress.gpu.ptr + addressOffset };
}

void DescriptorHeap::deferredDeallocate(Handle& a_handle)
{
    uint32_t index = getIndex(a_handle);

    clearHandle(a_handle);

    assert(m_currentFrame.has_value() && "if failed need set current frame for deferred deallocations, to do this call resetDeferred");
    m_indicesToDeferredDeallocate[m_currentFrame.value()].push_back(index);
}

void DescriptorHeap::immediatelyDeallocate(Handle& a_handle)
{
    uint32_t index = getIndex(a_handle);

    clearHandle(a_handle);

    deallocate(index);
}

void DescriptorHeap::resetDeferred(uint32_t a_newFrameIndex)
{
    assert(a_newFrameIndex < m_nDeferredVectors);
    m_currentFrame = a_newFrameIndex;
    std::vector<uint32_t>& currentDeferredIndices = m_indicesToDeferredDeallocate[a_newFrameIndex];

    if (!currentDeferredIndices.empty())
    {
        for (auto index : currentDeferredIndices)
        {
            deallocate(index);
        }
        currentDeferredIndices.clear();
    }
}

void DescriptorHeap::deallocate(uint32_t a_index)
{
    --m_heapSize;
    m_freeIndices[m_heapSize] = a_index;
}

uint32_t DescriptorHeap::getIndex(const Handle& a_handle) const
{
    assert(m_heapSize > 0);
    assert(a_handle.cpu.ptr != 0);
    assert(a_handle.cpu.ptr > m_startHeapAddress.cpu.ptr);
    SIZE_T addressOffset = a_handle.cpu.ptr - m_startHeapAddress.cpu.ptr;

    uint32_t index = static_cast<uint32_t>(addressOffset) / m_descriptorSize;
    assert(index < m_heapCapacity);
    return index;
}

void DescriptorHeap::clearHandle(Handle& a_handle)
{
    a_handle.cpu.ptr = 0;
    a_handle.gpu.ptr = 0;
}

}