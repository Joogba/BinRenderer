#pragma once
#include "IRenderPass.h"
#include "RendererAPI.h"
#include "Scene/Light/LightData.h"

#include <wrl/client.h>
#include <vector>

namespace BinRenderer
{

    class LightingPass : public IRenderPass
    {
    public:
        bool Initialize(RendererAPI* rhi) override;

        void Declare(RenderGraphBuilder& builder) override;

        void Execute(RendererAPI* rhi, const PassResources& res) override;

        void SetLights(const Light* lights, uint32_t count);

    private:
        PSOHandle     m_pso;
        SamplerHandle m_sampler;

        std::vector<Light> m_lights;

        static inline const char* kSRV_Normal = "GBuffer_Normal";
        static inline const char* kSRV_Albedo = "GBuffer_Albedo";
        static inline const char* kSRV_Param = "GBuffer_Param";
        static inline const char* kSRV_Depth = "GBuffer_Depth";
        static inline const char* kRT_Lighting = "Lighting";
    };

}