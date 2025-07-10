#pragma once
#include "Core/ITransientBufferAllocator.h"
#include <d3d11.h>
#include <wrl/client.h>
#include <cassert>

namespace BinRenderer {

    class D3D11TransientBufferAllocator : public ITransientBufferAllocator {
    public:
        D3D11TransientBufferAllocator(
            ID3D11Device* device,
            ID3D11DeviceContext* context,
            uint32_t totalBytes,
            uint32_t bindFlags
        );
        void beginFrame() override;
        void endFrame() override;
        uint32_t alloc(uint32_t sizeBytes, void*& dataPtr, uint32_t align = 16) override;
        void* buffer() const override { return m_buffer.Get(); }

    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;
        ID3D11DeviceContext* m_context;
        uint8_t* m_data = nullptr;
        uint32_t m_size;
        uint32_t m_offset = 0;
        uint32_t m_bindFlags;
    };

}