#include "ConstantBuffer.h"

namespace neural::graphics {
void ConstantBuffer::initialize(ID3D12Device* a_device, DescriptorHeap* a_cbvHeap,
                                const ConstantBufferCreateInfo& a_createInfo) {
    assert(a_device);
    assert(a_cbvHeap);
    Buffer::initialize(a_device, a_cbvHeap, {
        .size = a_createInfo.size,
        .elementSize = CalcConstantBufferByteSize(a_createInfo.elementSize),
        .aligment = a_createInfo.aligment,
        .usageFlags = a_createInfo.usageFlags,
        .initialState = D3D12_RESOURCE_STATE_GENERIC_READ,
        .heapType = D3D12_HEAP_TYPE_UPLOAD,
        .heapInfo = a_createInfo.heapInfo
    });
    mapData();
}

DescriptorHeap::Handle& ConstantBuffer::getConstantBufferView() {
        Buffer::ViewParams viewParams = { Buffer::ViewType::CONSTANT_BUFFER_VIEW };
        auto it = m_views.find(viewParams);
        if (it != m_views.end()) {
            return it->second;
        }
        else {
            auto handle = m_srvUavHeap->allocate();
            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
            cbvDesc.BufferLocation = m_resource->GetGPUVirtualAddress();
            cbvDesc.SizeInBytes = m_elementSize * m_size;
            m_device->CreateConstantBufferView(&cbvDesc, handle.cpu);
            m_views[viewParams] = handle;
            return handle;
        }
    }
}