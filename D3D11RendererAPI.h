#pragma once
#include "RendererAPI.h"
#include "DrawQueue.h"
#include "MeshRegistry.h"
#include "PSORegistry.h"
#include "MaterialSystem.h"

#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <DirectXMath.h>


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
        void SetViewProj(const DirectX::XMMATRIX& view,
            const DirectX::XMMATRIX& proj) override;

        // Accessors for integration and testing
        ID3D11Device* GetDevice() const;
        ID3D11DeviceContext* GetContext() const;
        MeshRegistry* GetMeshRegistry() const;
        PSORegistry* GetPSORegistry() const;
        MaterialRegistry* GetMaterialRegistry() const;

    private:
        HWND m_hwnd;
        Microsoft::WRL::ComPtr<ID3D11Device> m_device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
        Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;

        uint32_t m_width;
        uint32_t m_height;

        DirectX::XMMATRIX                           m_view;
        DirectX::XMMATRIX                           m_proj;
        DirectX::XMMATRIX                           m_viewProj;

    private:
        DrawQueue                           m_drawQueue;
        std::unique_ptr<MeshRegistry>       m_meshRegistry;
        std::unique_ptr<PSORegistry>        m_psoRegistry;
        std::unique_ptr<MaterialRegistry>   m_materialRegistry;
    };

    RendererAPI* CreateD3D11Renderer();
    void DestroyRenderer(RendererAPI* renderer);

};
