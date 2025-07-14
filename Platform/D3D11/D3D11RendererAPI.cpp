#include "D3D11RendererAPI.h"
#include "PipelineState.h"
#include "RendererAPI.h"

#include <cassert>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <algorithm>

using namespace DirectX;



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

        // 2) 기본 깊이·스텐실 상태 생성 (Depth 테스트 ON, 쓰기 ON, Func=LESS, 스텐실 OFF)
        D3D11_DEPTH_STENCIL_DESC dsDesc = {};
        dsDesc.DepthEnable = TRUE;
        dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
        dsDesc.StencilEnable = FALSE;
        // (dsDesc.FrontFace, dsDesc.BackFace 등도 설정)
        HRESULT hr = m_device->CreateDepthStencilState(&dsDesc, &m_depthStencilState);
        assert(SUCCEEDED(hr) && "CreateDepthStencilState failed");


        return SUCCEEDED(hr);
    }

    void D3D11RendererAPI::Resize(uint32_t w, uint32_t h) {
        m_context->OMSetRenderTargets(0, nullptr, nullptr);
        m_swapChain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);
    }

	void D3D11RendererAPI::BeginFrame()
	{
		// 1) 각 View 별로 뷰포트/RTV+DSV 바인딩 및 클리어
		for (auto& [viewId, view] : m_views)
		{
			if (!view.rtv || !view.dsv)
				continue;

			// 1.1) 뷰포트 설정
			m_context->RSSetViewports(1, &view.vp);

			// 1.2) RTV + DSV 바인딩
			m_context->OMSetRenderTargets(
				1,
				view.rtv.GetAddressOf(),
				view.dsv.Get()
				 );

			// 컬러 클리어
			if (view.clearFlags & ClearFlags::ClearColor)
			{
				float c[4] = {
				((view.clearColor >> 16) & 0xFF) / 255.0f,
				((view.clearColor >> 8) & 0xFF) / 255.0f,
				((view.clearColor >> 0) & 0xFF) / 255.0f,
				((view.clearColor >> 24) & 0xFF) / 255.0f,
				};
				m_context->ClearRenderTargetView(view.rtv.Get(), c);
			}

			//깊이·스텐실 클리어
			UINT dflags = 0;
			if (view.clearFlags & ClearFlags::ClearDepth)   dflags |= D3D11_CLEAR_DEPTH;
			if (view.clearFlags & ClearFlags::ClearStencil) dflags |= D3D11_CLEAR_STENCIL;
			if (dflags)
				m_context->ClearDepthStencilView(
					view.dsv.Get(),
					dflags,
					view.clearDepth,
					view.clearStencil
				);

			// DepthStencilState 적용
			m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
		}

		// 2) 트랜지언트 버퍼 프레임 시작
		m_transientVB->beginFrame();
		m_transientIB->beginFrame();
	}
    void D3D11RendererAPI::EndFrame() 
    {
        m_transientVB->endFrame();
        m_transientIB->endFrame();
    }

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

    void D3D11RendererAPI::EnqueueDraw(const DrawCommand& incmd)
    {
        DrawCommand cmd = incmd;

        // UniformSet 초기값 업데이트
        auto* material = m_materialRegistry->Get(cmd.materialHandle);
        if (material)
        {
            material->GetUniformSet().Set("modelMatrix", &cmd.transform, sizeof(cmd.transform));
            material->GetUniformSet().Set("viewProj", &m_viewProj, sizeof(m_viewProj));
        }

        // sortkey
        // 상위 8비트: viewId
        cmd.sortKey = (uint64_t(cmd.viewId) & 0xFFull) << 56;
        // 다음 16비트: PSO handle
        cmd.sortKey |= (uint64_t(cmd.psoHandle.idx) & 0xFFFFull) << 40;
        // 다음 16비트: Material handle
        cmd.sortKey |= (uint64_t(cmd.materialHandle.idx) & 0xFFFFull) << 24;
        // 하위 24비트: 뷰 공간 Z 정규화값
        {
            XMVECTOR worldPos = cmd.transform.r[3];
            XMVECTOR viewPos = XMVector3TransformCoord(worldPos, m_view);
            float depth = XMVectorGetZ(viewPos);
            float ndc = (depth - 0.1f) / (100.0f - 0.1f);
            ndc = std::clamp(ndc, 0.0f, 1.0f);
            uint32_t zbits = uint32_t(ndc * 0xFFFFFF);
            cmd.sortKey |= uint64_t(zbits) & 0xFFFFFFull;
        }

        m_drawQueue.Submit(cmd);

    }

    void D3D11RendererAPI::DrawSingle(const DrawCommand& cmd) {
        // D3D11 파이프라인 세팅, 머티리얼, 메시, 트랜스폼 등 적용
        // DrawIndexed, Draw 등 D3D11 명령 실행
    }
    void D3D11RendererAPI::DrawInstanced(const DrawCommand& cmd, const std::vector<glm::mat4>& transforms, int count) {
        // 인스턴스 버퍼(TransientBufferAllocator 등) 할당/업로드
        // IASetVertexBuffers(slot1, ...) 등
        // DrawIndexedInstanced 등 명령 실행
    }

    void D3D11RendererAPI::ExecuteDrawQueue()
    {
        m_drawQueue.flush([this](const DrawCommand& cmd)
            {
                // 2.1) 뷰포트 & 렌더타겟 바인딩
                auto& view = m_views[cmd.viewId];
                m_context->RSSetViewports(1, &view.vp);
                m_context->OMSetRenderTargets(1, view.rtv.GetAddressOf(), view.dsv.Get());

                // 2.2) 파이프라인 상태 & UniformSet 적용
                auto* material = m_materialRegistry->Get(cmd.materialHandle);
                const PipelineState* pso = m_psoRegistry->Get(material->GetPSO());

                // 미리 채워둔 modelMatrix, viewProj 를 재적용
                auto& us = material->GetUniformSet();
                XMMATRIX mvp = cmd.transform * m_viewProj;
                us.ApplyPredefined(PredefinedUniformType::ModelViewProj, &mvp, sizeof(mvp));

                bindInputLayout(pso->m_inputLayout.Get());
                bindPrimitiveTopology(pso->m_primitiveTopology);
                bindShaders(*pso);
                bindBlendState(pso->m_blendState.Get(), pso->m_blendFactor, pso->m_sampleMask);
                bindDepthStencilState(pso->m_depthStencilState.Get(), pso->m_stencilRef);
                bindRasterizerState(pso->m_rasterizerState.Get());

                // 2.3) 상수버퍼 생성 및 바인딩
                D3D11_BUFFER_DESC bd = {};
                bd.ByteWidth = us.GetSize();
                bd.Usage = D3D11_USAGE_DEFAULT;
                bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
                D3D11_SUBRESOURCE_DATA sd{ us.GetRawData(), 0, 0 };
                Microsoft::WRL::ComPtr<ID3D11Buffer> cb;
                if (SUCCEEDED(m_device->CreateBuffer(&bd, &sd, &cb)))
                {
                    m_context->VSSetConstantBuffers(0, 1, cb.GetAddressOf());
                    m_context->PSSetConstantBuffers(0, 1, cb.GetAddressOf());
                }

                // 2.4) 텍스처·샘플러 바인딩
                for (auto& tb : material->GetTextureBindings())
                {
                    auto srv = m_textureRegistry->Get(tb.handle);
                    m_context->PSSetShaderResources(tb.slot, 1, &srv);
                }
                for (auto& sb : material->GetSamplerBindings())
                {
                    auto ss = m_samplerRegistry->Get(sb.handle);
                    m_context->PSSetSamplers(sb.slot, 1, &ss);
                }

                // 2.5) 드로우 호출 (인스턴싱 포함)
                auto* mesh = m_meshRegistry->Get(cmd.meshHandle);
                if (cmd.instanceCount > 1)
                {
                    void* dataPtr = nullptr;
                    uint32_t vbOff = m_transientVB->alloc(
                        sizeof(XMMATRIX) * cmd.instanceCount,
                        dataPtr
                    );
                    memcpy(dataPtr, cmd.transforms.data(), sizeof(XMMATRIX) * cmd.instanceCount);

                    ID3D11Buffer* vbs[2] = {
                        mesh->vertexBuffer.Get(),
                        m_transientVB->buffer()
                    };
                    UINT strides[2] = { mesh->vertexStride, sizeof(XMMATRIX) };
                    UINT offsets[2] = { mesh->vertexOffset, vbOff };
                    m_context->IASetVertexBuffers(0, 2, vbs, strides, offsets);
                    m_context->IASetIndexBuffer(mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
                    m_context->DrawIndexedInstanced(mesh->indexCount, cmd.instanceCount, 0, 0, 0);
                }
                else
                {
                    m_context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(), &mesh->vertexStride, &mesh->vertexOffset);
                    m_context->IASetIndexBuffer(mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
                    m_context->DrawIndexed(mesh->indexCount, 0, 0);
                }
            });
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
