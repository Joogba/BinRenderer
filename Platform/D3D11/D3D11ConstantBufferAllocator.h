#pragma once
#include "Core/ITransientBufferAllocator.h"
#include <d3d11.h>
#include <wrl/client.h>
#include <cassert>

namespace BinRenderer {

    class D3D11ConstantBufferAllocator : public ITransientBufferAllocator {
    public:
        D3D11ConstantBufferAllocator(
            ID3D11Device* device,
            ID3D11DeviceContext* context,
            uint32_t initialBytes = 4096 // 4KB 기본
        )
            : m_device(device), m_context(context), m_size(initialBytes)
        {
            createBuffer(m_size);
        }

        void beginFrame() override {
            mapBuffer();
            m_offset = 0;
        }

        void endFrame() override {
            unmapBuffer();
            m_offset = 0;
        }

        uint32_t alloc(uint32_t sizeBytes, void*& dataPtr, uint32_t align = 16) override {
            // CB는 16바이트 align 필수!
            uint32_t aligned = (m_offset + 15) & ~15;
            if (aligned + sizeBytes > m_size) {
                // 리사이즈 필요
                resize(aligned + sizeBytes);
                mapBuffer();
            }
            dataPtr = m_data + aligned;
            m_offset = aligned + sizeBytes;
            return aligned;
        }

        void* buffer() const override { return m_buffer.Get(); }

    private:
        void createBuffer(uint32_t bytes) {
            D3D11_BUFFER_DESC desc{};
            desc.ByteWidth = bytes;
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.MiscFlags = 0;
            HRESULT hr = m_device->CreateBuffer(&desc, nullptr, &m_buffer);
            assert(SUCCEEDED(hr));
            m_size = bytes;
        }

        void mapBuffer() {
            D3D11_MAPPED_SUBRESOURCE mapped;
            HRESULT hr = m_context->Map(m_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            assert(SUCCEEDED(hr));
            m_data = reinterpret_cast<uint8_t*>(mapped.pData);
        }

        void unmapBuffer() {
            m_context->Unmap(m_buffer.Get(), 0);
            m_data = nullptr;
        }

        void resize(uint32_t requiredSize) {
            unmapBuffer();
            uint32_t newSize = m_size * 2;
            while (newSize < requiredSize) newSize *= 2;
            createBuffer(newSize);
            mapBuffer();
        }

        Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;
        ID3D11Device* m_device;
        ID3D11DeviceContext* m_context;
        uint8_t* m_data = nullptr;
        uint32_t m_size;
        uint32_t m_offset = 0;
    };

}