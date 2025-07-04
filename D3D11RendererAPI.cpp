#include "D3D11RendererAPI.h"
#include "PipelineState.h"
#include "RendererAPI.h"

#include <cassert>
#include <d3dcompiler.h>

namespace {
    HRESULT CompileShaderFromFile(
        const std::wstring& filePath,
        const char* entryPoint,
        const char* target,     // e.g. "vs_5_0" or "ps_5_0"
        ID3DBlob** codeBlob)
    {
        UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        ID3DBlob* errorBlob = nullptr;
        HRESULT hr = D3DCompileFromFile(
            filePath.c_str(),
            nullptr,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            entryPoint,
            target,
            flags,
            0,
            codeBlob,
            &errorBlob
        );
        if (FAILED(hr) && errorBlob) {
            OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
            errorBlob->Release();
        }
        return hr;
    }

    D3D11_BLEND_DESC ToD3D11BlendDesc(const BinRenderer::BlendState& bs)
    {
        D3D11_BLEND_DESC d = {};
        d.AlphaToCoverageEnable = bs.AlphaToCoverageEnable ? TRUE : FALSE;
        d.IndependentBlendEnable = bs.IndependentBlendEnable ? TRUE : FALSE;

        for (int i = 0; i < 8; ++i)
        {
            const auto& src = bs.RenderTarget[i];
            auto& dst = d.RenderTarget[i];
            dst.BlendEnable = src.BlendEnable ? TRUE : FALSE;
            dst.SrcBlend = static_cast<D3D11_BLEND>(src.SrcBlend);
            dst.DestBlend = static_cast<D3D11_BLEND>(src.DestBlend);
            dst.BlendOp = static_cast<D3D11_BLEND_OP>(src.BlendOp);
            dst.SrcBlendAlpha = static_cast<D3D11_BLEND>(src.SrcBlendAlpha);
            dst.DestBlendAlpha = static_cast<D3D11_BLEND>(src.DestBlendAlpha);
            dst.BlendOpAlpha = static_cast<D3D11_BLEND_OP>(src.BlendOpAlpha);
            dst.RenderTargetWriteMask = src.RenderTargetWriteMask;
        }
        return d;
    }

    D3D11_DEPTH_STENCIL_DESC ToD3D11DepthStencilDesc(const BinRenderer::DepthStencilState& ds)
    {
        D3D11_DEPTH_STENCIL_DESC d = {};
        d.DepthEnable = ds.DepthEnable ? TRUE : FALSE;
        d.DepthWriteMask = ds.DepthWriteMask ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
        d.DepthFunc = static_cast<D3D11_COMPARISON_FUNC>(ds.DepthFunc);

        d.StencilEnable = ds.StencilEnable ? TRUE : FALSE;
        d.StencilReadMask = ds.StencilReadMask;
        d.StencilWriteMask = ds.StencilWriteMask;

        // Front-face ops
        d.FrontFace.StencilFunc = static_cast<D3D11_COMPARISON_FUNC>(ds.FrontFace.StencilFunc);
        d.FrontFace.StencilFailOp = static_cast<D3D11_STENCIL_OP>(ds.FrontFace.StencilFailOp);
        d.FrontFace.StencilDepthFailOp = static_cast<D3D11_STENCIL_OP>(ds.FrontFace.StencilDepthFailOp);
        d.FrontFace.StencilPassOp = static_cast<D3D11_STENCIL_OP>(ds.FrontFace.StencilPassOp);

        // Back-face ops
        d.BackFace.StencilFunc = static_cast<D3D11_COMPARISON_FUNC>(ds.BackFace.StencilFunc);
        d.BackFace.StencilFailOp = static_cast<D3D11_STENCIL_OP>(ds.BackFace.StencilFailOp);
        d.BackFace.StencilDepthFailOp = static_cast<D3D11_STENCIL_OP>(ds.BackFace.StencilDepthFailOp);
        d.BackFace.StencilPassOp = static_cast<D3D11_STENCIL_OP>(ds.BackFace.StencilPassOp);

        return d;
    }

    D3D11_RASTERIZER_DESC ToD3D11RasterizerDesc(const BinRenderer::RasterizerState& rs)
    {
        D3D11_RASTERIZER_DESC d = {};
        d.FillMode = static_cast<D3D11_FILL_MODE>(rs.fillMode);
        d.CullMode = static_cast<D3D11_CULL_MODE>(rs.cullMode);
        d.FrontCounterClockwise = rs.frontCounterClockwise ? TRUE : FALSE;
        d.DepthBias = rs.depthBias;
        d.DepthBiasClamp = rs.depthBiasClamp;
        d.SlopeScaledDepthBias = rs.slopeScaledDepthBias;
        d.DepthClipEnable = rs.depthClipEnable ? TRUE : FALSE;
        d.ScissorEnable = rs.scissorEnable ? TRUE : FALSE;
        d.MultisampleEnable = rs.multisampleEnable ? TRUE : FALSE;
        return d;
    }

    std::vector<D3D11_INPUT_ELEMENT_DESC> ToD3D11InputLayout(
        const std::vector<BinRenderer::InputElementDesc>& inElems)
    {
        std::vector<D3D11_INPUT_ELEMENT_DESC> out;
        out.reserve(inElems.size());
        for (auto& e : inElems)
        {
            D3D11_INPUT_ELEMENT_DESC d = {};
            d.SemanticName = e.SemanticName;
            d.SemanticIndex = e.SemanticIndex;
            d.Format = static_cast<DXGI_FORMAT>(e.Format);
            d.InputSlot = e.InputSlot;
            d.AlignedByteOffset = e.AlignedByteOffset;
            d.InputSlotClass = (e.InputSlotClass == 0)
                ? D3D11_INPUT_PER_VERTEX_DATA
                : D3D11_INPUT_PER_INSTANCE_DATA;
            d.InstanceDataStepRate = e.InstanceDataStepRate;
            out.push_back(d);
        }
        return out;
    }
}


namespace BinRenderer {

    D3D11RendererAPI::D3D11RendererAPI() : 
        m_psoRegistry(std::make_unique<PSORegistry>())
       ,m_samplerRegistry(std::make_unique<SamplerRegistry>())
    { }

    D3D11RendererAPI::~D3D11RendererAPI() = default;

    bool D3D11RendererAPI::Init(const InitParams& p) {
        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 1;
        sd.BufferDesc.Width = p.width;
        sd.BufferDesc.Height = p.height;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = static_cast<HWND>(p.windowHandle);
        sd.SampleDesc.Count = 1;
        sd.Windowed = TRUE;
        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
            nullptr, 0, D3D11_SDK_VERSION,
            &sd, &m_swapChain,
            &m_device, nullptr, &m_context
        );
        return SUCCEEDED(hr);
    }

    void D3D11RendererAPI::Resize(uint32_t w, uint32_t h) {
        m_context->OMSetRenderTargets(0, nullptr, nullptr);
        m_swapChain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);
    }

    void D3D11RendererAPI::BeginFrame() {}
    void D3D11RendererAPI::EndFrame() {}

    void D3D11RendererAPI::Present() {
        m_swapChain->Present(1, 0);
    }

    TextureHandle D3D11RendererAPI::CreateTexture(const TextureDesc& desc) {
        D3D11_TEXTURE2D_DESC td = {};
        td.Width = desc.width;
        td.Height = desc.height;
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = ToDXGIFormat(desc.format);
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = desc.bindFlags;
        Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
        m_device->CreateTexture2D(&td, nullptr, &tex);
        TextureHandle h{ m_nextTexH++ };
        m_textures[h] = tex;
        return h;
    }

    RenderTargetViewHandle D3D11RendererAPI::CreateRTV(TextureHandle th) {
        auto tex = m_textures[th];
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
        m_device->CreateRenderTargetView(tex.Get(), nullptr, &rtv);
        RenderTargetViewHandle h{ m_nextRTVH++ };
        m_rtvs[h] = rtv;
        return h;
    }

    ShaderResourceViewHandle D3D11RendererAPI::CreateSRV(TextureHandle th) {
        auto tex = m_textures[th];
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
        m_device->CreateShaderResourceView(tex.Get(), nullptr, &srv);
        ShaderResourceViewHandle h{ m_nextSRVH++ };
        m_srvs[h] = srv;
        return h;
    }

    DepthStencilViewHandle D3D11RendererAPI::CreateDSV(TextureHandle th) {
        auto tex = m_textures[th];
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvd = {};
        dsvd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> dsv;
        m_device->CreateDepthStencilView(tex.Get(), &dsvd, &dsv);
        DepthStencilViewHandle h{ m_nextDSVH++ };
        m_dsvs[h] = dsv;
        return h;
    }

    PSOHandle D3D11RendererAPI::CreatePipelineState(const PSODesc& desc)
    {
        // 1) PipelineState 객체 생성
        auto pso = std::make_unique<PipelineState>();

        // 2) 버텍스 셰이더 컴파일
        std::wstring wvsFile(desc.vsFile.begin(), desc.vsFile.end());
        ComPtr<ID3DBlob> vsBlob;
        HRESULT hr = CompileShaderFromFile(
            wvsFile,
            desc.vsEntry.c_str(),
            "vs_5_0",
            &vsBlob
        );
        assert(SUCCEEDED(hr) && "Vertex shader compilation failed");
        m_device->CreateVertexShader(
            vsBlob->GetBufferPointer(),
            vsBlob->GetBufferSize(),
            nullptr,
            &pso->m_vertexShader
        );

        // 3) 픽셀 셰이더 컴파일
        std::wstring wpsFile(desc.psFile.begin(), desc.psFile.end());
        ComPtr<ID3DBlob> psBlob;
        hr = CompileShaderFromFile(
            wpsFile,
            desc.psEntry.c_str(),
            "ps_5_0",
            &psBlob
        );
        assert(SUCCEEDED(hr) && "Pixel shader compilation failed");
        m_device->CreatePixelShader(
            psBlob->GetBufferPointer(),
            psBlob->GetBufferSize(),
            nullptr,
            &pso->m_pixelShader
        );

        // 4) 입력 레이아웃 생성
        auto d3dLayout = ToD3D11InputLayout(desc.inputElements);
        m_device->CreateInputLayout(
            d3dLayout.data(),
            static_cast<UINT>(desc.inputElements.size()),
            vsBlob->GetBufferPointer(),
            vsBlob->GetBufferSize(),
            &pso->m_inputLayout
        );

        // 5) 렌더 스테이트 생성 (helper 함수 사용)
        auto blendDesc = ToD3D11BlendDesc(desc.blendState);
        auto dsDesc = ToD3D11DepthStencilDesc(desc.depthStencilState);
        auto rsDesc = ToD3D11RasterizerDesc(desc.rasterizerState);

        m_device->CreateBlendState(&blendDesc, &pso->m_blendState);
        m_device->CreateDepthStencilState(&dsDesc, &pso->m_depthStencilState);
        m_device->CreateRasterizerState(&rsDesc, &pso->m_rasterizerState);



        // 6) 기타 PSO 설정 복사
        pso->m_sampleMask = desc.sampleMask;
        pso->m_primitiveTopology = desc.primitiveTopology;
        pso->SetBlendFactor(desc.blendFactor);
        pso->m_stencilRef = desc.stencilRef;

        // 7) 레지스트리에 등록하고 핸들 반환
        return m_psoRegistry->Register(std::move(pso));
    }

    SamplerHandle D3D11RendererAPI::CreateSampler(const SamplerDesc& desc) {
        D3D11_SAMPLER_DESC sd = {};
        sd.Filter = static_cast<D3D11_FILTER>(desc.filter);
        sd.AddressU = static_cast<D3D11_TEXTURE_ADDRESS_MODE>(desc.addressU);
        sd.AddressV = static_cast<D3D11_TEXTURE_ADDRESS_MODE>(desc.addressV);
        sd.AddressW = static_cast<D3D11_TEXTURE_ADDRESS_MODE>(desc.addressW);
        sd.ComparisonFunc = static_cast<D3D11_COMPARISON_FUNC>(desc.comparison);
        sd.MinLOD = desc.minLOD;
        sd.MaxLOD = desc.maxLOD;
        sd.MipLODBias = desc.mipLODBias;
        sd.MaxAnisotropy = desc.maxAnisotropy;
        sd.BorderColor[0] = desc.borderColor[0];
        sd.BorderColor[1] = desc.borderColor[1];
        sd.BorderColor[2] = desc.borderColor[2];
        sd.BorderColor[3] = desc.borderColor[3];
        ComPtr<ID3D11SamplerState> ss;
        m_device->CreateSamplerState(&sd, &ss);

        // 인스턴스 레지스트리에 등록
        return m_samplerRegistry->Register(ss);
    }

    void D3D11RendererAPI::BindPipelineState(PSOHandle pso) {
        const PipelineState* p = m_psoRegistry->Get(pso);
        bindInputLayout(p->m_inputLayout.Get());
        bindPrimitiveTopology(p->m_primitiveTopology);
        bindShaders(*p);
        bindBlendState(p->m_blendState.Get(), p->m_blendFactor, p->m_sampleMask);
        bindDepthStencilState(p->m_depthStencilState.Get(), p->m_stencilRef);
        bindRasterizerState(p->m_rasterizerState.Get());
    }

    void D3D11RendererAPI::BindRenderTargets(const RenderTargetViewHandle* rtvs, size_t cnt, DepthStencilViewHandle dsv) {
        std::vector<ID3D11RenderTargetView*> views(cnt);
        for (size_t i = 0; i < cnt; ++i) views[i] = m_rtvs[rtvs[i]].Get();
        ID3D11DepthStencilView* d = dsv ? m_dsvs[dsv].Get() : nullptr;
        m_context->OMSetRenderTargets((UINT)cnt, views.data(), d);
    }

    void D3D11RendererAPI::ClearRenderTargets(uint32_t flags, uint32_t clr, float d, uint8_t s) {
        if (flags & ClearColor) {
            float c[4] = { ((clr >> 16) & 0xFF) / 255.f, ((clr >> 8) & 0xFF) / 255.f, ((clr >> 0) & 0xFF) / 255.f, ((clr >> 24) & 0xFF) / 255.f };
            ID3D11RenderTargetView* rtv = nullptr;
            m_context->OMGetRenderTargets(1, &rtv, nullptr);
            m_context->ClearRenderTargetView(rtv, c);
            if (rtv) rtv->Release();
        }
        UINT dflags = 0;
        if (flags & ClearDepth)   dflags |= D3D11_CLEAR_DEPTH;
        if (flags & ClearStencil) dflags |= D3D11_CLEAR_STENCIL;
        ID3D11DepthStencilView* dz = nullptr;
        m_context->OMGetRenderTargets(0, nullptr, &dz);
        if (dflags && dz) {
            m_context->ClearDepthStencilView(dz, dflags, d, s);
            dz->Release();
        }
    }

    void D3D11RendererAPI::BindShaderResource(uint32_t slot, ShaderResourceViewHandle srv) {
        auto view = m_srvs[srv].Get();
        m_context->PSSetShaderResources(slot, 1, &view);
    }

    void D3D11RendererAPI::BindSampler(SamplerHandle sampler, uint32_t slot) {
        auto ss = m_samplerRegistry->Get(sampler);
        m_context->PSSetSamplers(slot, 1, &ss);
    }

    void D3D11RendererAPI::EnqueueDraw(const DrawCommand& cmd)
    {
        
    }

    void D3D11RendererAPI::ExecuteDrawQueue() {
        // existing DrawQueue flush logic
    }

    void D3D11RendererAPI::BindFullScreenQuad() {
        UINT stride = sizeof(float) * 4;
        UINT off = 0;
        m_context->IASetInputLayout(m_fsIL.Get());
        m_context->IASetVertexBuffers(0, 1, m_fsVB.GetAddressOf(), &stride, &off);
        m_context->IASetIndexBuffer(m_fsIB.Get(), DXGI_FORMAT_R16_UINT, 0);
    }

    void D3D11RendererAPI::DrawFullScreenQuad() {
        m_context->DrawIndexed(6, 0, 0);
    }

    RenderTargetViewHandle D3D11RendererAPI::GetRTVByName(const char* name) const {
        return m_namedRTVs.at(name);
    }

    DepthStencilViewHandle D3D11RendererAPI::GetDSVByName(const char* name) const {
        return m_namedDSVs.at(name);
    }

    ShaderResourceViewHandle D3D11RendererAPI::GetSRVByName(const char* name) const {
        return m_namedSRVs.at(name);
    }

    DXGI_FORMAT D3D11RendererAPI::ToDXGIFormat(Format fmt) const {
        switch (fmt) {
        case Format::RGBA32_FLOAT:      return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case Format::R8G8B8A8_UNORM:    return DXGI_FORMAT_R8G8B8A8_UNORM;
        case Format::R16G16B16A16_FLOAT:return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case Format::R32_FLOAT:         return DXGI_FORMAT_R32_FLOAT;
        case Format::DEPTH24_STENCIL8:  return DXGI_FORMAT_R24G8_TYPELESS;
        }
        return DXGI_FORMAT_UNKNOWN;
    }

    // Helper implementations

    void D3D11RendererAPI::bindInputLayout(ID3D11InputLayout* layout) {
        if (layout != m_lastState.inputLayout) {
            m_context->IASetInputLayout(layout);
            m_lastState.inputLayout = layout;
        }
    }

    void D3D11RendererAPI::bindPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topo) {
        if (topo != m_lastState.topology) {
            m_context->IASetPrimitiveTopology(topo);
            m_lastState.topology = topo;
        }
    }

    void D3D11RendererAPI::bindShaders(const PipelineState& pso) {
        if (pso.m_vertexShader.Get() != m_lastState.vs) {
            m_context->VSSetShader(pso.m_vertexShader.Get(), nullptr, 0);
            m_lastState.vs = pso.m_vertexShader.Get();
        }
        if (pso.m_pixelShader.Get() != m_lastState.ps) {
            m_context->PSSetShader(pso.m_pixelShader.Get(), nullptr, 0);
            m_lastState.ps = pso.m_pixelShader.Get();
        }
        if (pso.m_geometryShader.Get() != m_lastState.gs) {
            m_context->GSSetShader(pso.m_geometryShader.Get(), nullptr, 0);
            m_lastState.gs = pso.m_geometryShader.Get();
        }
        if (pso.m_hullShader.Get() != m_lastState.hs) {
            m_context->HSSetShader(pso.m_hullShader.Get(), nullptr, 0);
            m_lastState.hs = pso.m_hullShader.Get();
        }
        if (pso.m_domainShader.Get() != m_lastState.ds) {
            m_context->DSSetShader(pso.m_domainShader.Get(), nullptr, 0);
            m_lastState.ds = pso.m_domainShader.Get();
        }
    }

    void D3D11RendererAPI::bindBlendState(ID3D11BlendState* bs, const float bf[4], UINT mask) {
        bool diffBS = bs != m_lastState.blendState;
        bool diffBF = memcmp(bf, m_lastState.blendFactor, sizeof m_lastState.blendFactor) != 0;
        if (diffBS || diffBF) {
            m_context->OMSetBlendState(bs, bf, mask);
            m_lastState.blendState = bs;
            memcpy(m_lastState.blendFactor, bf, sizeof m_lastState.blendFactor);
            m_lastState.sampleMask = mask;
        }
    }

    void D3D11RendererAPI::bindDepthStencilState(ID3D11DepthStencilState* dss, UINT stencilRef) {
        if (dss != m_lastState.depthStencilState || stencilRef != m_lastState.stencilRef) {
            m_context->OMSetDepthStencilState(dss, stencilRef);
            m_lastState.depthStencilState = dss;
            m_lastState.stencilRef = stencilRef;
        }
    }

    void D3D11RendererAPI::bindRasterizerState(ID3D11RasterizerState* rs) {
        if (rs != m_lastState.rasterizerState) {
            m_context->RSSetState(rs);
            m_lastState.rasterizerState = rs;
        }
    }

} // namespace BinRenderer
