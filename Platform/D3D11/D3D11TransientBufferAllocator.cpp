#include "D3D11TransientBufferAllocator.h"

namespace BinRenderer {

    D3D11TransientBufferAllocator::D3D11TransientBufferAllocator(
        ID3D11Device* device, ID3D11DeviceContext* context,
        uint32_t totalBytes, uint32_t bindFlags)
        : m_context(context), m_size(totalBytes), m_bindFlags(bindFlags)
    {
        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = totalBytes;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = bindFlags;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        HRESULT hr = device->CreateBuffer(&desc, nullptr, &m_buffer);
        assert(SUCCEEDED(hr) && "Failed to create transient buffer");
    }

    void D3D11TransientBufferAllocator::beginFrame()
    {
        D3D11_MAPPED_SUBRESOURCE mapped;
        HRESULT hr = m_context->Map(
            m_buffer.Get(),
            0,
            D3D11_MAP_WRITE_DISCARD,
            0,
            &mapped
        );
        assert(SUCCEEDED(hr));
        m_data = reinterpret_cast<uint8_t*>(mapped.pData);
        m_offset = 0;
    }

    void D3D11TransientBufferAllocator::endFrame()
    {
        m_context->Unmap(m_buffer.Get(), 0);
        m_data = nullptr;
        m_offset = 0;
    }

    uint32_t D3D11TransientBufferAllocator::alloc(uint32_t sizeBytes, void*& dataPtr, uint32_t align)
    {
        uint32_t aligned = (m_offset + (align - 1)) & ~(align - 1);
        if (aligned + sizeBytes > m_size) {
            sizeBytes = m_size - aligned;
        }
        dataPtr = m_data + aligned;
        m_offset = aligned + sizeBytes;
        return aligned;
    }

}