#pragma once
#include "Handle.h"

#include <vector>
#include <wrl/client.h>
#include <d3d11.h>

namespace BinRenderer {

    class SamplerRegistry
    {
    public:
        SamplerHandle Register(Microsoft::WRL::ComPtr<ID3D11SamplerState> ss)
        {
            m_samplers.push_back(ss);
            return { static_cast<uint32_t>(m_samplers.size() - 1) };
        }
        ID3D11SamplerState* Get(SamplerHandle h) const
        {
            if (h.idx < m_samplers.size()) return m_samplers[h.idx].Get();
            return nullptr;
        }
    private:
        std::vector<Microsoft::WRL::ComPtr<ID3D11SamplerState>> m_samplers;
    };
}