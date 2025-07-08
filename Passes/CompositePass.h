#pragma once
#include "IRenderPass.h"

namespace BinRenderer {

    class CompositePass : public IRenderPass
    {
    public:
        bool Initialize(RendererAPI* rhi) override;
        void Declare(RenderGraphBuilder& builder) override;
        void Execute(RendererAPI* rhi, const PassResources& res) override;

    private:
        PSOHandle m_pso;
        SamplerHandle m_sampler;

        static inline const char* kSRV_Lighting = "Lighting";
        static inline const char* kSRV_Albedo = "GBuffer_Albedo";
        static inline const char* kRT_BackBuffer = "BackBuffer";
    };

} // namespace BinRenderer

