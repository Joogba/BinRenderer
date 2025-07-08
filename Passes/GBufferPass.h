
#pragma once
#include "IRenderPass.h"
#include <wrl/client.h>
#include <d3d11.h>

namespace BinRenderer {

    class GBufferPass : public IRenderPass
    {
    public:
        bool Initialize(RendererAPI* rhi) override;

        void Declare(RenderGraphBuilder& builder) override;

        void Execute(RendererAPI* rhi, const PassResources& res) override;

    private:
        PSOHandle        m_pso;
        SamplerHandle    m_sampler;
        static inline const char* kRT_Normal = "GBuffer_Normal";
        static inline const char* kRT_Albedo = "GBuffer_Albedo";
        static inline const char* kRT_Param = "GBuffer_Param";
        static inline const char* kDS_Depth = "GBuffer_Depth";
    };

}