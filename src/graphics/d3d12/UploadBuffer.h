#pragma once
#include <utils/Macros.h>

#include <d3d12.h>
#include <wrl.h>
#include <d3dx12.h>

template<typename T>
class UploadBuffer
{
public:
	UploadBuffer() {};
	void initialize(ID3D12Device* a_device, UINT a_elementCount, bool a_isConstantBuffer,
		D3D12_CPU_DESCRIPTOR_HANDLE a_view)
	{
		m_view = a_view;
		m_isConstantBuffer = a_isConstantBuffer;
		m_elementByteSize = sizeof(T);
		// Constant buffer elements need to be multiples of 256 bytes.
		// This is because the hardware can only view constant data
		// at m*256 byte offsets and of n*256 byte lengths.
		// typedef struct D3D12_CONSTANT_BUFFER_VIEW_DESC {
		// UINT64 OffsetInBytes; // multiple of 256
		// UINT SizeInBytes; // multiple of 256
		// } D3D12_CONSTANT_BUFFER_VIEW_DESC;
		if (a_isConstantBuffer) {
			m_elementByteSize = CalcConstantBufferByteSize(sizeof(T));
		}
		CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_elementByteSize * a_elementCount);
		DX_CALL(a_device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_uploadBuffer.GetAddressOf())));
		DX_CALL(m_uploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedData)));
		// We do not need to unmap until we are done with the resource.
		// However, we must not write to the resource while it is in use by
		// the GPU (so we must use synchronization techniques).

		// Address to start of the buffer (0th constant buffer).
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_uploadBuffer->GetGPUVirtualAddress();
		// Offset to the ith object constant buffer in the buffer.
		int boxCBufIndex = 0;
		cbAddress += boxCBufIndex * m_elementByteSize;

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = m_elementByteSize;
		a_device->CreateConstantBufferView(
			&cbvDesc,
			m_view);
	}
	UploadBuffer(const UploadBuffer&) = delete;
	UploadBuffer& operator=(const UploadBuffer&) = delete;
	~UploadBuffer() {
		if (m_uploadBuffer != nullptr)
			m_uploadBuffer->Unmap(0, nullptr);
		m_mappedData = nullptr;
	}
	ID3D12Resource* getResource() const {
		return m_uploadBuffer.Get();
	}
	D3D12_CPU_DESCRIPTOR_HANDLE getView() const {
		return m_view;
	}
	void setData(int a_elementIndex, const T& a_data) {
		memcpy(&m_mappedData[a_elementIndex * m_elementByteSize], &a_data, sizeof(T));
	}
private:
	uint32_t CalcConstantBufferByteSize(uint32_t a_byteSize) {
		return (a_byteSize + 255) & ~255;
	}
	D3D12_CPU_DESCRIPTOR_HANDLE m_view;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadBuffer;
	BYTE* m_mappedData = nullptr;
	UINT m_elementByteSize = 0;
	bool m_isConstantBuffer = false;
};