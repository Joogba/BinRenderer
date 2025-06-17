#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <cassert>
#include <cstdint>

namespace BinRenderer {

    // ũ�⸦ ���� �ΰ� D3D11_DYNAMIC/WRITE_DISCARD ���� �� ������ Map/Unmap,
    // �׸��� �� ���� �ȿ��� ����Ʈ ������ Alloc �� �ִ� ��ƿ��Ƽ Ŭ����
    class TransientBufferAllocator
    {
    public:
        // bindFlags: D3D11_BIND_VERTEX_BUFFER �Ǵ� D3D11_BIND_INDEX_BUFFER
        TransientBufferAllocator(
            ID3D11Device* device,
            ID3D11DeviceContext* context,
            uint32_t                totalBytes,
            uint32_t                bindFlags
        )
            : m_context(context)
            , m_size(totalBytes)
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

        // �� ������ ���� ��
        void beginFrame()
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

        // �� ������ ���� ��
        void endFrame()
        {
            m_context->Unmap(m_buffer.Get(), 0);
            m_data = nullptr;
            m_offset = 0;
        }

        // sizeBytes ��ŭ ������, dataPtr �� CPU ���� ������ ������ ������.
        // ���ϰ��� ���� �� byte-offset.
        uint32_t alloc(uint32_t sizeBytes, void*& dataPtr)
        {
            // 16����Ʈ ����(���ؽ�) Ȥ�� 4����Ʈ ����(�ε���)
            const uint32_t align = (m_bindFlags == D3D11_BIND_VERTEX_BUFFER) ? 16u : 4u;
            uint32_t aligned = (m_offset + (align - 1)) & ~(align - 1);
            if (aligned + sizeBytes > m_size)
            {
                // ���� ������ �����ϸ�, �׳� ���� �ִ� ũ��� �߶� �����ֱ�
                sizeBytes = m_size - aligned;
            }
            dataPtr = m_data + aligned;
            m_offset = aligned + sizeBytes;
            return aligned;
        }

        ID3D11Buffer* buffer() const { return m_buffer.Get(); }

    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;
        ID3D11DeviceContext* m_context;
        uint8_t* m_data = nullptr;
        uint32_t                            m_size;
        uint32_t                            m_offset = 0;
        uint32_t                            m_bindFlags;
    };

} // namespace BinRenderer
