
#include "DeferredRenderer.h"
#include "D3D11RendererAPI.h"

namespace BinRenderer
{

    DeferredRenderer::DeferredRenderer() = default;
    DeferredRenderer::~DeferredRenderer() = default;

    void DeferredRenderer::Init(const InitParams& params)
    {
        // 1) RHI 생성 및 초기화
        m_core.reset(CreateD3D11Renderer());
        m_core->Init(params);

        // 2) RenderGraph 생성 (크기 정보 전달)
        m_graph = std::make_unique<RenderGraph>(m_core.get(), params.width, params.height);

        // 3) 패스 생성
        m_gbufferPass = std::make_unique<GBufferPass>();
        m_lightingPass = std::make_unique<LightingPass>();
        m_compositePass = std::make_unique<CompositePass>();

        // 4) 패스별 리소스·PSO 초기화
        m_gbufferPass->Initialize(m_core.get());
        m_lightingPass->Initialize(m_core.get());
        m_compositePass->Initialize(m_core.get());

        // 5) 그래프에 패스 등록 (Declare/Execute 순서 보장)
        m_graph->AddPass(std::move(m_gbufferPass));
        m_graph->AddPass(std::move(m_lightingPass));
        m_graph->AddPass(std::move(m_compositePass));
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

        // 1) Declare → 리소스 생성
        m_graph->Build();

        // 2) Execute 모든 패스
        m_graph->Execute();

        // 프레임 종료
        m_core->EndFrame();
        m_core->Present();
    }

} // namespace BinRenderer