
#pragma once

#include "Core/RendererAPI.h"
#include "Core/DrawCommand.h"
#include "Core/RenderGraph.h"
#include "Passes/GBufferPass.h"
#include "Passes/LightingPass.h"
#include "Passes/CompositePass.h"
#include "Core/RenderStates.h"

#include <memory>

namespace BinRenderer
{
    class DeferredRenderer
    {
    public:
        DeferredRenderer();
        ~DeferredRenderer();

        /// 렌더러(D3D11) 초기화 + 패스 준비
        void Init(const InitParams& params);

        /// 각 DrawCommand를 내부 core 에 등록
        void Submit(const DrawCommand& cmd);

        /// 매 프레임 G-Buffer→Lighting→Composite 실행
        void RenderFrame();

        /// 윈도우 크기 바뀔때 호출
        void Resize(uint32_t newWidth, uint32_t newHeight);

        // renderer api getter
        RendererAPI* GetCore() const { return m_core.get(); }

        // light pass 에 라이트 정보 전달
        void SetLight(const std::vector<Light>& lights);

    private:
        // 실제 렌더링 구현체 (초기에는 D3D11RendererAPI)
        std::unique_ptr<BinRenderer::RendererAPI> m_core;
        std::unique_ptr<RenderGraph>                m_graph;      // RenderGraph 실행 엔진

        // Deferred 파이프라인을 구성하는 3개 패스
        std::unique_ptr<IRenderPass> m_gbufferPass;
        std::unique_ptr<IRenderPass> m_lightingPass;
        std::unique_ptr<IRenderPass> m_compositePass;

        uint32_t m_width = 0;
        uint32_t m_height = 0;

        void setupPasses();
    };
}