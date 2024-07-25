#pragma once

#include <utils/Macros.h>

#include <graphics/d3d12/CommonGraphicsHeaders.h>

#include <vector>
#include <memory>
#include <optional>

namespace neural::graphics {

class DescriptorHeap
{
public:
    struct Handle
    {
        D3D12_CPU_DESCRIPTOR_HANDLE cpu = D3D12_CPU_DESCRIPTOR_HANDLE(0);
        D3D12_GPU_DESCRIPTOR_HANDLE gpu = D3D12_GPU_DESCRIPTOR_HANDLE(0);
        bool isShaderVisiable() {
            return gpu.ptr != 0;
        }
    };
    void initialize(ID3D12Device* a_device, uint32_t a_nFrames,
        D3D12_DESCRIPTOR_HEAP_TYPE a_type, uint32_t a_capacity, bool isShaderVisiable);

    Handle allocate();

    void resetDeferred(uint32_t a_newFrame);
    void deferredDeallocate(Handle& a_handle);

    // Be careful, when multiple command allocators access a handle, deallocate it immediatly can lead to errors
    void immediatelyDeallocate(Handle& a_handle);

    ID3D12DescriptorHeap* getID3D12DescriptorHeap()
    {
        return m_heap.Get();
    }
private:
    void deallocate(uint32_t a_index);
    uint32_t getIndex(const Handle& a_handle) const;
    void clearHandle(Handle& a_handle);

    ComPtr<ID3D12DescriptorHeap> m_heap;
    Handle m_startHeapAddress;
    uint32_t m_descriptorSize;

    std::unique_ptr<uint32_t[]> m_freeIndices;
    std::uint32_t m_heapSize;
    DEBUG_LINE(std::uint32_t m_heapCapacity);

    std::unique_ptr<std::vector<uint32_t>[]> m_indicesToDeferredDeallocate;
    std::optional<std::uint32_t> m_currentFrame;
    DEBUG_LINE(std::uint32_t m_nDeferredVectors);
};
}