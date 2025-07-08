#include "LightingPass.h"
#include "D3D11RendererAPI.h"

namespace BinRenderer
{
    bool LightingPass::Initialize(RendererAPI* rhi)
    {
        PSODesc desc{};
        desc.name = "Lighting";
        desc.vsFile = "shaders/Lighting.hlsl";
        desc.vsEntry = "VSQuad";
        desc.psFile = "shaders/Lighting.hlsl";
        desc.psEntry = "PSMain";
        // TODO : (inputElements, rasterizerState, depthStencilState, blendState desc)
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


    void LightingPass::Declare(RenderGraphBuilder& builder)
    {
        builder.ReadTexture(kSRV_Normal);
        builder.ReadTexture(kSRV_Albedo);
        builder.ReadTexture(kSRV_Param);
        builder.ReadTexture(kSRV_Depth);

        TextureDesc td{
            uint32_t(builder.GetWidth()),
            uint32_t(builder.GetHeight()),
            Format::RGBA32_FLOAT,
            uint32_t(BindFlags::Bind_RenderTarget | BindFlags::Bind_ShaderResource)
        };
        builder.DeclareRenderTarget(kRT_Lighting, td);
    }

    void LightingPass::Execute(RendererAPI* rhi, const PassResources& res)
    {
        auto srvNormal = res.GetSRV(kSRV_Normal);
        auto srvAlbedo = res.GetSRV(kSRV_Albedo);
        auto srvParam = res.GetSRV(kSRV_Param);
        auto srvDepth = res.GetSRV(kSRV_Depth);
        auto rtvLight = res.GetRTV(kRT_Lighting);

        rhi->BindPipelineState(m_pso);
        rhi->BindRenderTargets(&rtvLight, 1, {});
        rhi->ClearRenderTargets(
            uint32_t(ClearFlags::ClearColor),
            0x000000FF, 1.0f, 0
        );
        rhi->BindSampler(m_sampler, 0);

        rhi->BindShaderResource(0, srvNormal);
        rhi->BindShaderResource(1, srvAlbedo);
        rhi->BindShaderResource(2, srvParam);
        rhi->BindShaderResource(3, srvDepth);

        rhi->BindFullScreenQuad();
        rhi->DrawFullScreenQuad();
    }

}