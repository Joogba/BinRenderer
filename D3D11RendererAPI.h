#pragma once
#include "RendererAPI.h"
#include "DrawQueue.h"
#include "MeshRegistry.h"
#include "PSORegistry.h"
#include "MaterialSystem.h"
#include "View.h"

#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <DirectXMath.h>
#include <unordered_map>
#include <cstdint> 

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

        // Accessors for integration and testing
        ID3D11Device* GetDevice() const;
        ID3D11DeviceContext* GetContext() const;
        MeshRegistry* GetMeshRegistry() const;
        PSORegistry* GetPSORegistry() const;
        MaterialRegistry* GetMaterialRegistry() const;

        //View
        void CreateView(uint8_t viewId)override;
        void SetViewRTV(uint8_t viewId, ID3D11RenderTargetView* rtv)override;
        void SetViewDSV(uint8_t viewId, ID3D11DepthStencilView* dsv)override;
        void SetViewClear(uint8_t viewId, uint32_t flags, uint32_t clearColor, float depth = 1.0f, uint8_t stencil = 0)override;
        void SetViewRect(uint8_t viewId, float x, float y, float width, float height)override;
        void SetViewProj(const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj)override;

        void Resize(uint32_t width, uint32_t height) override;

    private:
        
        Microsoft::WRL::ComPtr<ID3D11Device> m_device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
        Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;

        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
        Microsoft::WRL::ComPtr<ID3D11Texture2D>           m_depthStencilBuffer;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView>    m_depthStencilView;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState>   m_depthStencilState;

        HWND                m_hwnd;
        uint32_t            m_width;
        uint32_t            m_height;
        DirectX::XMMATRIX   m_view;
        DirectX::XMMATRIX   m_proj;
        DirectX::XMMATRIX   m_viewProj;

        DrawQueue                           m_drawQueue;
        std::unique_ptr<MeshRegistry>       m_meshRegistry;
        std::unique_ptr<PSORegistry>        m_psoRegistry;
        std::unique_ptr<MaterialRegistry>   m_materialRegistry;

        std::unordered_map<uint8_t, View> m_views;
    };

    RendererAPI* CreateD3D11Renderer();
    void DestroyRenderer(RendererAPI* renderer);

};
