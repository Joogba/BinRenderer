#include "D3D11RenderTargetPool.h"
namespace BinRenderer {

    D3D11RenderTargetPool::D3D11RenderTargetPool(ID3D11Device* device)
        : m_device(device)
    {
    }

    D3D11RenderTargetResource* D3D11RenderTargetPool::CreateResource(const TextureDesc& desc)
    {
        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = desc.width;
        texDesc.Height = desc.height;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // desc.format 매핑 필요!
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        ComPtr<ID3D11Texture2D> texture;
        HRESULT hr = m_device->CreateTexture2D(&texDesc, nullptr, &texture);
        if (FAILED(hr)) return nullptr;

        ComPtr<ID3D11RenderTargetView> rtv;
        hr = m_device->CreateRenderTargetView(texture.Get(), nullptr, &rtv);
        if (FAILED(hr)) return nullptr;

        auto res = std::make_unique<D3D11RenderTargetResource>();
        res->desc = desc;
        res->texture = texture;
        res->rtv = rtv;
        res->handle = TextureHandle(m_nextId++);
        res->inUse = true;

        D3D11RenderTargetResource* ptr = res.get();
        m_resources[res->handle] = std::move(res);

        return ptr;
    }

    TextureHandle D3D11RenderTargetPool::Acquire(const TextureDesc& desc)
    {
        // 1. 풀에 사용 가능한 리소스가 있으면 반환
        for (auto& pair : m_resources) {
            auto& res = pair.second;
            if (!res->inUse && res->desc == desc) {
                res->inUse = true;
                return res->handle;
            }
        }
        // 2. 없으면 새로 생성
        D3D11RenderTargetResource* newRes = CreateResource(desc);
        return newRes ? newRes->handle : TextureHandle();
    }

    void D3D11RenderTargetPool::Release(TextureHandle handle)
    {
        auto it = m_resources.find(handle);
        if (it != m_resources.end()) {
            it->second->inUse = false;
        }
    }

    void D3D11RenderTargetPool::Reset()
    {
        for (auto& pair : m_resources) {
            pair.second->inUse = false;
        }
    }

    D3D11RenderTargetResource* D3D11RenderTargetPool::GetResource(TextureHandle handle)
    {
        auto it = m_resources.find(handle);
        return it != m_resources.end() ? it->second.get() : nullptr;
    }

}