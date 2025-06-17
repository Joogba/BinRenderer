#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <cassert>
#include <cstdint>

namespace BinRenderer {

    // 크기를 정해 두고 D3D11_DYNAMIC/WRITE_DISCARD 모드로 매 프레임 Map/Unmap,
    // 그리고 그 범위 안에서 바이트 단위로 Alloc 해 주는 유틸리티 클래스
    class TransientBufferAllocator
    {
    public:
        // bindFlags: D3D11_BIND_VERTEX_BUFFER 또는 D3D11_BIND_INDEX_BUFFER
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

        // 매 프레임 시작 시
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

        // 매 프레임 끝날 때
        void endFrame()
        {
            m_context->Unmap(m_buffer.Get(), 0);
            m_data = nullptr;
            m_offset = 0;
        }

        // sizeBytes 만큼 빌리고, dataPtr 에 CPU 쓰기 가능한 포인터 돌려줌.
        // 리턴값은 버퍼 내 byte-offset.
        uint32_t alloc(uint32_t sizeBytes, void*& dataPtr)
        {
            // 16바이트 정렬(버텍스) 혹은 4바이트 정렬(인덱스)
            const uint32_t align = (m_bindFlags == D3D11_BIND_VERTEX_BUFFER) ? 16u : 4u;
            uint32_t aligned = (m_offset + (align - 1)) & ~(align - 1);
            if (aligned + sizeBytes > m_size)
            {
                // 남은 공간이 부족하면, 그냥 남은 최대 크기로 잘라서 돌려주기
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
