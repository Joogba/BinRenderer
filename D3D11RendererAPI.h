#pragma once
#include "RendererAPI.h"
#include "DrawQueue.h"
#include "MeshRegistry.h"
#include "PSORegistry.h"
#include "MaterialSystem.h"

#include <d3d11.h>
#include <wrl/client.h>
#include <memory>


namespace BinRenderer
{

    class D3D11RendererAPI :
        public RendererAPI
    {
    public:
        D3D11RendererAPI();
        ~D3D11RendererAPI() override;

        bool Init(const InitParams& params) override;
        void BeginFrame() override;
        void Submit() override;    
        void Submit(const DrawCommand& cmd) override;
        void EndFrame() override;
        void Present() override;

        

    private:
        HWND m_hwnd;
        Microsoft::WRL::ComPtr<ID3D11Device> m_device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
        Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;

    private:
        DrawQueue m_drawQueue;
        std::unique_ptr<MeshRegistry> m_meshRegistry;
        std::unique_ptr<PSORegistry> m_psoRegistry;
        std::unique_ptr<MaterialRegistry> m_materialRegistry;
    };

    RendererAPI* CreateD3D11Renderer();
    void DestroyRenderer(RendererAPI* renderer);

};
