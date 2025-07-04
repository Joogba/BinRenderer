
#include "GBufferPass.h"
#include "IRenderPass.h"
#include <array>


namespace BinRenderer {

    bool GBufferPass::Initialize(RendererAPI* rhi)
    {

        // TODO : (inputElements, rasterizerState, depthStencilState, blendState 설정)
        // PSO 생성
         // PSO 준비 (GeometryPass 셰이더)
        PSODesc desc = {};
        desc.name = "GBuffer";
        desc.vsFile = "shaders/GBuffer.hlsl";
        desc.vsEntry = "VSMain";
        desc.psFile = "shaders/GBuffer.hlsl";
        desc.psEntry = "PSMain";
        m_pso = rhi->CreatePipelineState(desc);

        // 샘플러 생성
        SamplerDesc sd{};
        sd.filter = FilterMode::Linear;
        sd.addressU = AddressMode::Clamp;
        sd.addressV = AddressMode::Clamp;
        sd.addressW = AddressMode::Clamp;
        sd.comparison = ComparisonFunc::Always;    // 비교 기능은 사용 안 함
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
        td.bindFlags = BindFlags::Bind_RenderTarget | BindFlags::Bind_ShaderResource;
        td.format = Format::RGBA32_FLOAT;
        td.width = builder.GetWidth();
        td.height = builder.GetHeight();

        builder.DeclareRenderTarget(kRT_Normal, td);
        builder.DeclareRenderTarget(kRT_Albedo, td);
        builder.DeclareRenderTarget(kRT_Param, td);

        td.format = Format::DEPTH24_STENCIL8;
        td.bindFlags = uint32_t(BindFlags::Bind_DepthStencil | BindFlags::Bind_ShaderResource);

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
        rhi->ClearRenderTargets(ClearFlags::ClearColor | ClearFlags::ClearDepth, 0x303030ff, 1.0f, 0);
        rhi->BindSampler(m_sampler, 0);

        // DrawQueue 실행
        rhi->ExecuteDrawQueue();
    }

} // namespace BinRenderer