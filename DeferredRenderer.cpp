
#include "DeferredRenderer.h"

namespace BinRenderer
{

    DeferredRenderer::DeferredRenderer() = default;
    DeferredRenderer::~DeferredRenderer() = default;

    void DeferredRenderer::Init(const InitParams& params)
    {
        //RHI 생성 및 초기화
        m_core.reset(CreateD3D11Renderer());
        m_core->Init(params);

        m_width = params.width;
        m_height = params.height;

        //RenderGraph 생성 (크기 정보 전달)
        m_graph = std::make_unique<RenderGraph>(m_core.get(), params.width, params.height);

        setupPasses();
    }

    void DeferredRenderer::Submit(const DrawCommand& cmd)
    {
        // DrawCommand 큐에 등록
        m_core->EnqueueDraw(cmd);
    }

    void DeferredRenderer::RenderFrame()
    {
        // 프레임 시작
        m_core->BeginFrame();

        // Declare → 리소스 생성
        m_graph->Build();

        // Execute 모든 패스
        m_graph->Execute();

        // 프레임 종료
        m_core->EndFrame();
        m_core->Present();
    }

    void DeferredRenderer::Resize(uint32_t newWidth, uint32_t newHeight)
    {
        if (newWidth == m_width && newHeight == m_height)
            return;

        m_width = newWidth;
        m_height = newHeight;

        // RHI 자체도 크기 변경
        m_core->Resize(m_width, m_height);

        // RenderGraph에 알려주고, 의존 리소스 (G-Buffer용 텍스처 등)를 초기화
        m_graph->Resize(m_width, m_height);

        // Pass 자체(PSO, 샘플러)는 재생성할 필요 없음
    }

    void DeferredRenderer::SetLight(const std::vector<Light>& lights)
    {
        auto* lp = static_cast<LightingPass>(m_lightingPass.get());
        lp->SetLights(lights.data(), (uint32_t)lights.size());
    }

    void DeferredRenderer::setupPasses()
    {
        m_gbufferPass = std::make_unique<GBufferPass>();
        m_lightingPass = std::make_unique<LightingPass>();
        m_compositePass = std::make_unique<CompositePass>();

        m_gbufferPass->Initialize(m_core.get());
        m_lightingPass->Initialize(m_core.get());
        m_compositePass->Initialize(m_core.get());

        m_graph->AddPass(std::move(m_gbufferPass));
        m_graph->AddPass(std::move(m_lightingPass));
        m_graph->AddPass(std::move(m_compositePass));
    }
} // namespace BinRenderer