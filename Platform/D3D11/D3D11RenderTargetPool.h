#pragma once
#include "Resources/RenderTargetPool.h"

#include <d3d11.h>
#include <wrl/client.h>
#include <unordered_map>
#include <vector>
#include <memory>

namespace BinRenderer
{

    using Microsoft::WRL::ComPtr;

    struct D3D11RenderTargetResource {
        TextureDesc desc;
        ComPtr<ID3D11Texture2D> texture;
        ComPtr<ID3D11RenderTargetView> rtv;
        // 필요하다면 SRV, DSV 등 추가
        TextureHandle handle;
        bool inUse = false; // 풀 관리용
    };

    class D3D11RenderTargetPool : public RenderTargetPool {
    public:
        D3D11RenderTargetPool(ID3D11Device* device);

        TextureHandle Acquire(const TextureDesc& desc) override;
        void Release(TextureHandle handle) override;
        void Reset() override;

        // 실제 GPU 객체 조회 (패스/드로우에서 사용)
        D3D11RenderTargetResource* GetResource(TextureHandle handle);

    private:
        ID3D11Device* m_device = nullptr;

        std::unordered_map<TextureHandle, std::unique_ptr<D3D11RenderTargetResource>> m_resources;
        std::vector<D3D11RenderTargetResource*> m_freeList;
        uint32_t m_nextId = 1;

        D3D11RenderTargetResource* CreateResource(const TextureDesc& desc);
    };


}