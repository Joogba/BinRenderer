#include "GBufferPass.h"
#include "Core/IRenderPass.h"
#include "Core/FlagOps.h"
#include <array>
#include <cfloat>


namespace BinRenderer {

    bool GBufferPass::Initialize(RendererAPI* rhi)
    {
        // TODO: 실제 셰이더 로딩 및 PSO 생성
        // 현재는 기본 PSO만 생성
        PSODesc desc = {};
        // desc에는 셰이더 핸들을 직접 설정해야 함
        m_pso = rhi->CreatePipelineState(desc);

        // 샘플러 생성
        SamplerDesc sd{};
        sd.filter = FilterMode::Linear;
        sd.addressU = AddressMode::Clamp;
        sd.addressV = AddressMode::Clamp;
        sd.addressW = AddressMode::Clamp;
        sd.comparison = ComparisonFunc::Always;
        sd.minLOD = 0.0f;
        sd.maxLOD = FLT_MAX;
        sd.mipLODBias = 0.0f;
        sd.maxAnisotropy = 1;
        sd.borderColor[0] = sd.borderColor[1] =
            sd.borderColor[2] = sd.borderColor[3] = 0.0f;
        m_sampler = rhi->CreateSampler(sd);
        return true;
    }

    void GBufferPass::Declare(RenderGraphBuilder& builder)
    {
        // 텍스처 설명
        TextureDesc td;
        td.bindFlags = uint32_t(BindFlags::Bind_RenderTarget) | uint32_t(BindFlags::Bind_ShaderResource);
        td.format = Format::RGBA32_FLOAT;
        td.width = builder.GetWidth();
        td.height = builder.GetHeight();

        builder.DeclareRenderTarget(kRT_Normal, td);
        builder.DeclareRenderTarget(kRT_Albedo, td);
        builder.DeclareRenderTarget(kRT_Param, td);

        td.format = Format::DEPTH24_STENCIL8;
        td.bindFlags = uint32_t(BindFlags::Bind_DepthStencil) | uint32_t(BindFlags::Bind_ShaderResource);

        builder.DeclareDepthStencil(kDS_Depth, td);
    }

    void GBufferPass::Execute(RendererAPI* rhi, const PassResources& res)
    {
        // 핸들 가져오기
        auto rtvNormal = res.GetRTV(kRT_Normal);
        auto rtvAlbedo = res.GetRTV(kRT_Albedo);
        auto rtvParam = res.GetRTV(kRT_Param);
        auto dsvDepth = res.GetDSV(kDS_Depth);
        
        // MRT + DSV 바인딩
        auto mrt = std::array<RenderTargetViewHandle, 3> { rtvNormal, rtvAlbedo, rtvParam };

        // 파이프라인 및 샘플러 바인딩
        rhi->BindPipelineState(m_pso);
        rhi->BindRenderTargets(mrt.data(), mrt.size(), dsvDepth);
        rhi->ClearRenderTargets(
            uint32_t(ClearFlags::ClearColor) | uint32_t(ClearFlags::ClearDepth), 
            0x303030ff, 1.0f, 0
        );
        rhi->BindSampler(m_sampler, 0);

        // DrawQueue 실행
        rhi->ExecuteDrawQueue();
    }

} // namespace BinRenderer