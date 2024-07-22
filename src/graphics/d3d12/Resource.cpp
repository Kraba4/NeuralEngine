#include "Resource.h"

namespace neural::graphics {
void Resource::initializeInternal(ID3D12Device* a_device, const CreateInfo& a_info)
{
    assert(a_device);
    m_device = a_device;
    assert(!a_info.clearValue.has_value() ||
        (a_info.resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET ||
            a_info.resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL));

    auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(m_heapType);
    if (a_info.heapInfo.has_value()) {
        assert(a_info.heapInfo.value().heap != nullptr);  // failed looks like something wrong, 
        // however mb this condition need to add in above "if"
        DX_CALL(a_device->CreatePlacedResource(
            a_info.heapInfo.value().heap,
            a_info.heapInfo.value().offset,
            &a_info.resourceDesc,
            a_info.initialState,
            a_info.clearValue.has_value() ? &a_info.clearValue.value() : nullptr,
            IID_PPV_ARGS(&m_resource)));
    }
    else {
        DX_CALL(a_device->CreateCommittedResource(
            &defaultHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &a_info.resourceDesc,
            a_info.initialState,
            a_info.clearValue.has_value() ? &a_info.clearValue.value() : nullptr,
            IID_PPV_ARGS(&m_resource)));
        DX_CALL(a_device->GetDeviceRemovedReason());
    }
}

void Resource::addRenderTargetView(std::string a_viewName, std::optional<D3D12_RENDER_TARGET_VIEW_DESC> a_rtvDesc)
{
    assert(a_viewName.find(k_metaNameRTV) == -1);
    assert(m_resource.Get());
    assert(m_rtvHeap);
    a_viewName = k_metaNameRTV + std::to_string(0) + a_viewName;
    assert(!m_handles.contains(a_viewName));

    auto handle = m_rtvHeap->allocate();
    m_device->CreateRenderTargetView(m_resource.Get(),
        a_rtvDesc.has_value() ? &a_rtvDesc.value() : nullptr,
        handle.cpu);
    m_handles[a_viewName] = handle;
}

void Resource::addShaderResourceView(std::string a_viewName, std::optional<D3D12_SHADER_RESOURCE_VIEW_DESC> a_srvDesc)
{
    assert(a_viewName.find(k_metaNameSRV) == -1);
    assert(m_resource.Get());
    assert(m_srvHeap);
    a_viewName = k_metaNameSRV + a_viewName;
    assert(!m_handles.contains(a_viewName));

    auto handle = m_srvHeap->allocate();
    m_device->CreateShaderResourceView(m_resource.Get(),
        a_srvDesc.has_value() ? &a_srvDesc.value() : nullptr,
        handle.cpu);
    m_handles[a_viewName] = handle;
}

void Resource::addDepthStencilView(std::string a_viewName, std::optional<D3D12_DEPTH_STENCIL_VIEW_DESC> a_dsvDesc)
{
    assert(a_viewName.find(k_metaNameDSV) == -1);
    assert(m_resource.Get());
    assert(m_dsvHeap);
    a_viewName = k_metaNameDSV + a_viewName;
    assert(!m_handles.contains(a_viewName));

    auto handle = m_dsvHeap->allocate();
    m_device->CreateDepthStencilView(m_resource.Get(),
        a_dsvDesc.has_value() ? &a_dsvDesc.value() : nullptr,
        handle.cpu);
    m_handles[a_viewName] = handle;
}
void Resource::addConstantBufferView(std::string a_viewName, uint32_t a_elementSize)
{

    assert(a_viewName.find(k_metaNameCBV) == -1);
    assert(m_resource.Get());
    assert(m_srvHeap);
    a_viewName = k_metaNameCBV + a_viewName;
    assert(!m_handles.contains(a_viewName));


    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
    cbvDesc.BufferLocation = m_resource.Get()->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = a_elementSize;

    auto handle = m_srvHeap->allocate();
    m_device->CreateConstantBufferView(&cbvDesc, handle.cpu);

    m_handles[a_viewName] = handle;
}
}