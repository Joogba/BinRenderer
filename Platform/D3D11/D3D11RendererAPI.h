#pragma once

#include "RendererAPI.h"
#include "Handle.h"
#include "RenderStates.h"
#include "PSORegistry.h"
#include "SamplerRegistry.h"
#include "MaterialSystem.h"
#include "MeshRegistry.h"
#include "TextureRegistry.h"
#include "DrawQueue.h"
#include "DrawCommand.h"
#include "TransientBufferAllocator.h"
#include "View.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <unordered_map>
#include <string>



namespace BinRenderer {


    class D3D11RendererAPI : public RendererAPI {
    public:
        D3D11RendererAPI();
        ~D3D11RendererAPI() noexcept override = default;

        // RendererAPI interface
        bool Init(const InitParams& params) override;
        void Resize(uint32_t width, uint32_t height) override;
        void BeginFrame() override;
        void EndFrame() override;
        void Present() override;

        TextureHandle            CreateTexture(const TextureDesc& desc) override;
        RenderTargetViewHandle   CreateRTV(TextureHandle tex) override;
        ShaderResourceViewHandle CreateSRV(TextureHandle tex) override;
        DepthStencilViewHandle   CreateDSV(TextureHandle tex) override;
        PSOHandle                CreatePipelineState(const PSODesc& desc) override;
        SamplerHandle            CreateSampler(const SamplerDesc& desc) override;

        void BindPipelineState(PSOHandle pso) override;
        void BindRenderTargets(const RenderTargetViewHandle* rtvs, size_t count, DepthStencilViewHandle dsv) override;
        void ClearRenderTargets(uint32_t flags, uint32_t clearColor, float clearDepth, uint8_t clearStencil) override;

        void BindShaderResource(uint32_t slot, ShaderResourceViewHandle srv) override;
        void BindSampler(SamplerHandle sampler, uint32_t slot) override;

        void EnqueueDraw(const DrawCommand& cmd) override;
        void ExecuteDrawQueue() override;
        void BindFullScreenQuad() override;
        void DrawFullScreenQuad() override;

        RenderTargetViewHandle   GetRTVByName(const char* name) const override;
        DepthStencilViewHandle   GetDSVByName(const char* name) const override;
        ShaderResourceViewHandle GetSRVByName(const char* name) const override;

    private:
        // D3D11 device/context
        Microsoft::WRL::ComPtr<ID3D11Device>           m_device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext>    m_context;
        Microsoft::WRL::ComPtr<IDXGISwapChain>         m_swapChain;

        // Fullscreen quad resources
        Microsoft::WRL::ComPtr<ID3D11Buffer>           m_fsVB;
        Microsoft::WRL::ComPtr<ID3D11Buffer>           m_fsIB;
        Microsoft::WRL::ComPtr<ID3D11InputLayout>      m_fsIL;

        // 뷰 카메라
        DirectX::XMMATRIX   m_view = DirectX::XMMatrixIdentity();
        DirectX::XMMATRIX   m_proj = DirectX::XMMatrixIdentity();
        DirectX::XMMATRIX   m_viewProj = DirectX::XMMatrixIdentity();

        // 뷰 rtv / dsv / viewport 관리
        std::unordered_map<uint8_t, View>   m_views;

        // 드로우 큐
        DrawQueue   m_drawQueue;

        // Resource registries
        std::unordered_map<TextureHandle, Microsoft::WRL::ComPtr<ID3D11Texture2D>>                      m_textures;
        std::unordered_map<RenderTargetViewHandle, Microsoft::WRL::ComPtr<ID3D11RenderTargetView>>      m_rtvs;
        std::unordered_map<ShaderResourceViewHandle, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>  m_srvs;
        std::unordered_map<DepthStencilViewHandle, Microsoft::WRL::ComPtr<ID3D11DepthStencilView>>      m_dsvs;

        std::unique_ptr<PSORegistry>                m_psoRegistry;
        std::unique_ptr<SamplerRegistry>            m_samplerRegistry;
        std::unique_ptr<MeshRegistry>               m_meshRegistry;
        std::unique_ptr<MaterialRegistry>           m_materialRegistry;
        std::unique_ptr<TextureRegistry>            m_textureRegistry;
        std::unique_ptr<SamplerRegistry>            m_samplerRegistry;
        std::unique_ptr<TransientBufferAllocator>   m_transientVB;
        std::unique_ptr<TransientBufferAllocator>   m_transientIB;

        // Named lookups for render graph
        std::unordered_map<std::string, RenderTargetViewHandle>   m_namedRTVs;
        std::unordered_map<std::string, DepthStencilViewHandle>   m_namedDSVs;
        std::unordered_map<std::string, ShaderResourceViewHandle> m_namedSRVs;

        // Handle counters
        uint32_t m_nextTexH = 1;
        uint32_t m_nextRTVH = 1;
        uint32_t m_nextSRVH = 1;
        uint32_t m_nextDSVH = 1;

        Microsoft::WRL::ComPtr<ID3D11DepthStencilState>   m_depthStencilState;

        // State caching
        struct BoundState {
            ID3D11InputLayout* inputLayout = nullptr;
            D3D11_PRIMITIVE_TOPOLOGY     topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
            ID3D11VertexShader* vs = nullptr;
            ID3D11PixelShader* ps = nullptr;
            ID3D11GeometryShader* gs = nullptr;
            ID3D11HullShader* hs = nullptr;
            ID3D11DomainShader* ds = nullptr;
            ID3D11BlendState* blendState = nullptr;
            float                         blendFactor[4] = {};
            UINT                          sampleMask = 0xFFFFFFFF;
            ID3D11DepthStencilState* depthStencilState = nullptr;
            UINT                          stencilRef = 0;
            ID3D11RasterizerState* rasterizerState = nullptr;
        } m_lastState;

        // Helper: state bind with caching
        void bindInputLayout(ID3D11InputLayout* layout);
        void bindPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topo);
        void bindShaders(const PipelineState& pso);
        void bindBlendState(ID3D11BlendState* bs, const float bf[4], UINT mask = 0xFFFFFFFF);
        void bindDepthStencilState(ID3D11DepthStencilState* dss, UINT stencilRef);
        void bindRasterizerState(ID3D11RasterizerState* rs);

        // Convert abstract format to DXGI
        DXGI_FORMAT ToDXGIFormat(Format fmt) const;
    };
}