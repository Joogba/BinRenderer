#pragma once
#include "Handle.h"

#include <vector>
#include <wrl/client.h>
#include <d3d11.h>

namespace BinRenderer {

    class TextureRegistry
    {
    public:
        TextureHandle Register(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
        {
            m_srvs.push_back(srv);
            return { static_cast<uint32_t>(m_srvs.size() - 1) };
        }
        ID3D11ShaderResourceView* Get(TextureHandle h) const
        {
            if (h.idx < m_srvs.size()) return m_srvs[h.idx].Get();
            return nullptr;
        }
    private:
        std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_srvs;
    };
}