#include "CompositePass.h"
#include "Core/FlagOps.h"
#include <array>
#include <cfloat>

namespace BinRenderer {

    bool CompositePass::Initialize(RendererAPI* rhi)
    {
        // TODO: 실제 셰이더 로딩 및 PSO 생성
        PSODesc desc{};
        // desc에는 셰이더 핸들을 직접 설정해야 함
        m_pso = rhi->CreatePipelineState(desc);

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

    void CompositePass::Declare(RenderGraphBuilder& builder)
    {
        builder.ReadTexture(kSRV_Lighting);
        builder.ReadTexture(kSRV_Albedo);
        builder.ImportBackbuffer(kRT_BackBuffer);
    }

    void CompositePass::Execute(RendererAPI* rhi, const PassResources& res)
    {
        auto srvLight = res.GetSRV(kSRV_Lighting);
        auto srvAlbedo = res.GetSRV(kSRV_Albedo);
        auto rtvBB = res.GetRTV(kRT_BackBuffer);

        rhi->BindPipelineState(m_pso);
        rhi->BindRenderTargets(&rtvBB, 1, {});
        rhi->BindSampler(m_sampler, 0);

        rhi->BindShaderResource(0, srvLight);
        rhi->BindShaderResource(1, srvAlbedo);

        rhi->BindFullScreenQuad();
        rhi->DrawFullScreenQuad();
    }

} // namespace BinRenderer