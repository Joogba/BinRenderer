
#pragma once

#include "RendererAPI.h"
#include "DrawCommand.h"
#include "RenderGraph.h"
#include "GBufferPass.h"
#include "LightingPass.h"
#include "CompositePass.h"

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

    private:
        // 실제 렌더링 구현체 (초기에는 D3D11RendererAPI)
        std::unique_ptr<BinRenderer::RendererAPI> m_core;
        std::unique_ptr<RenderGraph>                m_graph;      // RenderGraph 실행 엔진

        // Deferred 파이프라인을 구성하는 3개 패스
        std::unique_ptr<IRenderPass> m_gbufferPass;
        std::unique_ptr<IRenderPass> m_lightingPass;
        std::unique_ptr<IRenderPass> m_compositePass;
    };
}