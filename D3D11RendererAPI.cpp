#include "D3D11RendererAPI.h"
#include "DrawCommand.h"
#include "MeshRegistry.h"
#include "PSORegistry.h"
#include "MaterialSystem.h"
#include "RendererAPI.h"
#include "View.h"

#include <d3d11.h>
#include <memory>
#include <DirectXMath.h>

namespace BinRenderer
{
    D3D11RendererAPI::D3D11RendererAPI()
        : m_hwnd(nullptr)
        , m_view(DirectX::XMMatrixIdentity())
        , m_proj(DirectX::XMMatrixIdentity())
        , m_viewProj(DirectX::XMMatrixIdentity())
        , m_width(0)
        , m_height(0)
    {
    }

    D3D11RendererAPI::~D3D11RendererAPI() {}

    bool D3D11RendererAPI::Init(const InitParams& params) {
        m_hwnd = static_cast<HWND>(params.windowHandle);
        m_width = params.width;
        m_height = params.height;

        DXGI_SWAP_CHAIN_DESC scDesc = {};
        scDesc.BufferCount = 1;
        scDesc.BufferDesc.Width = params.width;
        scDesc.BufferDesc.Height = params.height;
        scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scDesc.OutputWindow = m_hwnd;
        scDesc.SampleDesc.Count = 1;
        scDesc.Windowed = TRUE;

        D3D_FEATURE_LEVEL featureLevel;
        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
            nullptr, 0,
            D3D11_SDK_VERSION,
            &scDesc,
            &m_swapChain,
            &m_device,
            &featureLevel,
            &m_context
        );

        if (FAILED(hr)) return false;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
        m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));

        m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView);
        // --- 1) Depth-Stencil Buffer 생성 ---
        D3D11_TEXTURE2D_DESC descDepth = {};
        descDepth.Width = params.width;
        descDepth.Height = params.height;
        descDepth.MipLevels = 1;
        descDepth.ArraySize = 1;
        descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        descDepth.SampleDesc.Count = 1;
        descDepth.Usage = D3D11_USAGE_DEFAULT;
        descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        m_device->CreateTexture2D(&descDepth, nullptr, &m_depthStencilBuffer);
        HRESULT hrDSV = m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), nullptr, &m_depthStencilView);
        assert(SUCCEEDED(hrDSV) && "CreateDepthStencilView failed!");
        OutputDebugStringA(">>> DSV created, ptr=");
        {
            char buf[64];
            sprintf_s(buf, "%p\n", m_depthStencilView.Get());
            OutputDebugStringA(buf);
        }

        // --- 2) 기본 Depth-Stencil State 생성 (깊이 테스트 켜기) ---
        D3D11_DEPTH_STENCIL_DESC dsDesc = {};
        dsDesc.DepthEnable = TRUE;
        dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
        dsDesc.StencilEnable = FALSE;
        m_device->CreateDepthStencilState(&dsDesc, &m_depthStencilState);

        // 1) 기본 뷰 생성
        CreateView(0);

        // 2) 렌더 타깃과 깊이 뷰 연결
        SetViewRTV(0, m_renderTargetView.Get());
        SetViewDSV(0, m_depthStencilView.Get());

        // 3) 화면 전체로 뷰포트 & 기본 클리어 설정
        SetViewRect(0, 0, 0, params.width, params.height);
        SetViewClear(0,
            static_cast<uint32_t>(ClearFlags::ClearColor | ClearFlags::ClearDepth),
            0x303030ff, 1.0f, 0
        );


        m_meshRegistry = std::make_unique<MeshRegistry>();
        m_psoRegistry = std::make_unique<PSORegistry>();
        m_materialRegistry = std::make_unique<MaterialRegistry>();

        return true;
    }

    void D3D11RendererAPI::SetViewProj(const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj) {
        m_view = view;
        m_proj = proj;
        m_viewProj = XMMatrixMultiply(view, proj);
    }

    void D3D11RendererAPI::BeginFrame() {
        // 1) 각 뷰(View) 별로 클리어 및 뷰포트 설정
        char buf[128];
        sprintf_s(buf,
            ">>> BeginFrame, view count=%zu\n",
            m_views.size()
        );
        OutputDebugStringA(buf);

        for (auto& [viewId, view] : m_views)
        {
            if (!view.rtv || !view.dsv)
                continue;   // 뷰가 완전히 설정되지 않았다면 스킵!

            // 1.1) 뷰포트 설정
            m_context->RSSetViewports(1, &view.vp);

            // 1.2) 렌더 타겟 + 깊이·스텐실 뷰 바인딩
            m_context->OMSetRenderTargets(1, view.rtv.GetAddressOf(), view.dsv.Get());

            // 1.3) 컬러 클리어
            if (view.clearFlags & ClearFlags::ClearColor)
            {
                // view.clearColor: 0xAARRGGBB
                float c[4] = {
                    ((view.clearColor >> 16) & 0xFF) / 255.0f,
                    ((view.clearColor >> 8) & 0xFF) / 255.0f,
                    ((view.clearColor >> 0) & 0xFF) / 255.0f,
                    ((view.clearColor >> 24) & 0xFF) / 255.0f,
                };
                m_context->ClearRenderTargetView(view.rtv.Get(), c);
            }

            // 1.4) 깊이·스텐실 클리어
            UINT dsvFlags = 0;
            if (view.clearFlags & ClearFlags::ClearDepth)   dsvFlags |= D3D11_CLEAR_DEPTH;
            if (view.clearFlags & ClearFlags::ClearStencil) dsvFlags |= D3D11_CLEAR_STENCIL;
            if (dsvFlags != 0)
            {
                m_context->ClearDepthStencilView(view.dsv.Get(), dsvFlags, view.clearDepth, view.clearStencil);
            }

            // 1.5) 깊이-스텐실 상태 적용 (모든 드로우에서 공통 사용)
            m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
        }
    }

    void D3D11RendererAPI::Submit(const DrawCommand& cmd) {
        // DrawCommands 처리 예정
        m_drawQueue.Submit(cmd);
    }

    void D3D11RendererAPI::Submit()
    {
        for (const auto& cmd : m_drawQueue.GetCommands()) {
            // 1) ViewId에 맞춰 RTV/DSV 재바인딩
            auto& view = m_views[cmd.viewId];
            m_context->OMSetRenderTargets(1, view.rtv.GetAddressOf(), view.dsv.Get());
            m_context->RSSetViewports(1, &view.vp);


            const Mesh* mesh = m_meshRegistry->Get(cmd.meshHandle);
            Material* material = m_materialRegistry->Get(cmd.materialHandle);
            material->GetUniformSet().Set("modelMatrix", &cmd.transform, sizeof(cmd.transform));
            material->GetUniformSet().Set("viewProj", &m_viewProj, sizeof(m_viewProj));
            if (!mesh || !material) continue;

            const PipelineState* pso = m_psoRegistry->Get(material->GetPSO());
            if (!pso) continue;


            m_context->IASetVertexBuffers(0,1, &mesh->vertexBuffer, &mesh->vertexStride, &mesh->vertexOffset);
            m_context->IASetIndexBuffer(mesh->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
            m_context->IASetInputLayout(pso->m_inputLayout.Get());
            m_context->IASetPrimitiveTopology(pso->m_primitiveTopology);

            m_context->VSSetShader(pso->m_vertexShader.Get(), nullptr, 0);
            m_context->PSSetShader(pso->m_pixelShader.Get(), nullptr, 0);
            if (pso->m_geometryShader) m_context->GSSetShader(pso->m_geometryShader.Get(), nullptr, 0);
            if (pso->m_hullShader)     m_context->HSSetShader(pso->m_hullShader.Get(), nullptr, 0);
            if (pso->m_domainShader)   m_context->DSSetShader(pso->m_domainShader.Get(), nullptr, 0);

            m_context->OMSetBlendState(pso->m_blendState.Get(), pso->m_blendFactor, 0xffffffff);
            m_context->OMSetDepthStencilState(pso->m_depthStencilState.Get(), pso->m_stencilRef);
            m_context->RSSetState(pso->m_rasterizerState.Get());


            // UniformSet으로 상수버퍼 업데이트
            const UniformSet& uniforms = material->GetUniformSet();
            D3D11_BUFFER_DESC cbDesc = {};
            cbDesc.ByteWidth = uniforms.GetSize();
            cbDesc.Usage = D3D11_USAGE_DEFAULT;
            cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = uniforms.GetRawData();

            Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;
            HRESULT hr = m_device->CreateBuffer(&cbDesc, &initData, &constantBuffer);
            if (SUCCEEDED(hr)) {
                m_context->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
                m_context->PSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
            }

            m_context->DrawIndexed(mesh->indexCount, 0, 0);

        }
        m_drawQueue.Clear();
    }


    void D3D11RendererAPI::EndFrame() {
        // Post-processing 등 예정
    }

    void D3D11RendererAPI::Present() {
        m_swapChain->Present(1, 0);
    }
    // Accessors
    ID3D11Device* D3D11RendererAPI::GetDevice() const
    {
        return m_device.Get();
    }

    ID3D11DeviceContext* D3D11RendererAPI::GetContext() const
    {
        return m_context.Get();
    }

    MeshRegistry* D3D11RendererAPI::GetMeshRegistry() const
    {
        return m_meshRegistry.get();
    }

    PSORegistry* D3D11RendererAPI::GetPSORegistry() const
    {
        return m_psoRegistry.get();
    }

    MaterialRegistry* D3D11RendererAPI::GetMaterialRegistry() const
    {
        return m_materialRegistry.get();
    }

    void D3D11RendererAPI::CreateView(uint8_t id)
    {
        OutputDebugStringA("CreateView called\n");
        m_views[id] = View{};
    }

    void D3D11RendererAPI::SetViewRTV(uint8_t id, ID3D11RenderTargetView* rtv)
    {
        m_views[id].rtv = rtv;
    }

    void D3D11RendererAPI::SetViewDSV(uint8_t id, ID3D11DepthStencilView* dsv)
    {
        char buf[128];
        sprintf_s(buf,
            ">>> SetViewDSV called id=%u, dsv_ptr=%p\n",
            id, dsv
        );
        OutputDebugStringA(buf);
        m_views[id].dsv = dsv;
    }

    void D3D11RendererAPI::SetViewClear(uint8_t id,
        uint32_t flags,
        uint32_t color,
        float depth,
        uint8_t stencil)
    {
        auto& v = m_views[id];
        v.clearFlags = flags;
        v.clearColor = color;
        v.clearDepth = depth;
        v.clearStencil = stencil;
    }

    void D3D11RendererAPI::SetViewRect(uint8_t id,
        float x, float y,
        float w, float h)
    {
        auto& vp = m_views[id].vp;
        vp.TopLeftX = x; vp.TopLeftY = y;
        vp.Width = w; vp.Height = h;
        vp.MinDepth = 0.0f; vp.MaxDepth = 1.0f;
    }

    RendererAPI* CreateD3D11Renderer()
    {
        return new D3D11RendererAPI();
    }

    void DestroyRenderer(RendererAPI* renderer) {
        delete renderer;
    }

}