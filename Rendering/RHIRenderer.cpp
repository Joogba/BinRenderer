#include "RHIRenderer.h"
#include "../Core/Logger.h"
#include "../RenderPass/RHIForwardPassRG.h"

namespace BinRenderer
{
	RHIRenderer::RHIRenderer(RHI* rhi)
		: rhi_(rhi)
	{
	}

	RHIRenderer::~RHIRenderer()
	{
		shutdown();
	}

	bool RHIRenderer::initialize(uint32_t width, uint32_t height)
	{
		width_ = width;
		height_ = height;

		printLog("RHIRenderer::initialize - {}x{}", width, height);

		// RenderGraph 생성
		renderGraph_ = std::make_unique<RenderGraph>(rhi_);

		// 렌더 패스 설정
		setupRenderPasses();

		// RenderGraph 컴파일
		renderGraph_->compile();

		printLog("RHIRenderer initialized successfully");
		return true;
	}

	void RHIRenderer::shutdown()
	{
		if (renderGraph_)
		{
			renderGraph_.reset();
		}

		printLog("RHIRenderer shutdown complete");
	}

	void RHIRenderer::setupRenderPasses()
	{
		// Forward Pass 추가
		auto forwardPass = std::make_unique<RHIForwardPassRG>(rhi_);
		renderGraph_->addPass(std::move(forwardPass));

		printLog("RenderGraph passes setup complete");
	}

	void RHIRenderer::beginFrame(uint32_t frameIndex)
	{
		// 프레임 시작 준비
	}

	void RHIRenderer::render(RHIScene& scene, uint32_t frameIndex)
	{
		// RenderGraph 실행
		if (renderGraph_)
		{
			renderGraph_->execute(frameIndex);
		}
	}

	void RHIRenderer::endFrame(uint32_t frameIndex)
	{
		// 프레임 종료 처리
	}

	void RHIRenderer::resize(uint32_t width, uint32_t height)
	{
		width_ = width;
		height_ = height;

		printLog("RHIRenderer::resize - {}x{}", width, height);

		// TODO: RenderGraph 리소스 재생성
	}

} // namespace BinRenderer
