#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <unordered_map>
#include <memory>
#include <string>

#include "Core/Handle.h"

namespace BinRenderer {

    class D3D11ResourceRegistry
    {
    public:
        // Texture
        void RegisterTexture(TextureHandle h, Microsoft::WRL::ComPtr<ID3D11Texture2D> tex) {
            m_textures[h] = tex;
        }
        Microsoft::WRL::ComPtr<ID3D11Texture2D> GetTexture(TextureHandle h) const {
            auto it = m_textures.find(h);
            return it != m_textures.end() ? it->second : nullptr;
        }
        void UnregisterTexture(TextureHandle h) { m_textures.erase(h); }

        // RenderTargetView
        void RegisterRTV(RenderTargetViewHandle h, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv) {
            m_rtvs[h] = rtv;
        }
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> GetRTV(RenderTargetViewHandle h) const {
            auto it = m_rtvs.find(h);
            return it != m_rtvs.end() ? it->second : nullptr;
        }
        void UnregisterRTV(RenderTargetViewHandle h) { m_rtvs.erase(h); }

        // ShaderResourceView
        void RegisterSRV(ShaderResourceViewHandle h, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv) {
            m_srvs[h] = srv;
        }
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSRV(ShaderResourceViewHandle h) const {
            auto it = m_srvs.find(h);
            return it != m_srvs.end() ? it->second : nullptr;
        }
        void UnregisterSRV(ShaderResourceViewHandle h) { m_srvs.erase(h); }

        // DepthStencilView
        void RegisterDSV(DepthStencilViewHandle h, Microsoft::WRL::ComPtr<ID3D11DepthStencilView> dsv) {
            m_dsvs[h] = dsv;
        }
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> GetDSV(DepthStencilViewHandle h) const {
            auto it = m_dsvs.find(h);
            return it != m_dsvs.end() ? it->second : nullptr;
        }
        void UnregisterDSV(DepthStencilViewHandle h) { m_dsvs.erase(h); }

        // Buffer (Vertex/Index/Instance 등)
        void RegisterBuffer(MeshHandle h, Microsoft::WRL::ComPtr<ID3D11Buffer> vb, Microsoft::WRL::ComPtr<ID3D11Buffer> ib) {
            m_vertexBuffers[h] = vb;
            m_indexBuffers[h] = ib;
        }
        Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer(MeshHandle h) const {
            auto it = m_vertexBuffers.find(h);
            return it != m_vertexBuffers.end() ? it->second : nullptr;
        }
        Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer(MeshHandle h) const {
            auto it = m_indexBuffers.find(h);
            return it != m_indexBuffers.end() ? it->second : nullptr;
        }
        void UnregisterMesh(MeshHandle h) {
            m_vertexBuffers.erase(h);
            m_indexBuffers.erase(h);
        }

        // 필요에 따라 더 추가 가능 (Sampler, PipelineState, 등)

    private:
        std::unordered_map<TextureHandle, Microsoft::WRL::ComPtr<ID3D11Texture2D>> m_textures;
        std::unordered_map<RenderTargetViewHandle, Microsoft::WRL::ComPtr<ID3D11RenderTargetView>> m_rtvs;
        std::unordered_map<ShaderResourceViewHandle, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_srvs;
        std::unordered_map<DepthStencilViewHandle, Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> m_dsvs;
        std::unordered_map<MeshHandle, Microsoft::WRL::ComPtr<ID3D11Buffer>> m_vertexBuffers;
        std::unordered_map<MeshHandle, Microsoft::WRL::ComPtr<ID3D11Buffer>> m_indexBuffers;
    };

} // namespace BinRenderer