#include "DeferredRendererRG.h"
#include "../Core/Logger.h"

namespace BinRenderer
{
	DeferredRendererRG::DeferredRendererRG(RHI* rhi)
		: rhi_(rhi)
	{
	}

	DeferredRendererRG::~DeferredRendererRG()
	{
		shutdown();
	}

	bool DeferredRendererRG::initialize(uint32_t width, uint32_t height)
	{
		width_ = width;
		height_ = height;

		printLog("=== Initializing DeferredRendererRG (Complete Pipeline) ===");
		printLog("  Resolution: {}x{}", width, height);

		// RenderGraph 생성 및 구성
		renderGraph_ = std::make_unique<RenderGraph>(rhi_);
		setupRenderGraph();

		// RenderGraph 컴파일
		printLog("  Compiling RenderGraph...");
		renderGraph_->compile();

		// 디버그 정보 출력
		printDebugInfo();

		printLog("✅ DeferredRendererRG initialized successfully");
		return true;
	}

	void DeferredRendererRG::shutdown()
	{
		renderGraph_.reset();
		printLog("DeferredRendererRG shutdown");
	}

	void DeferredRendererRG::resize(uint32_t width, uint32_t height)
	{
		if (width_ == width && height_ == height)
			return;

		printLog("Resizing DeferredRendererRG: {}x{} -> {}x{}", width_, height_, width, height);

		width_ = width;
		height_ = height;

		// RenderGraph 재구성
		renderGraph_.reset();
		renderGraph_ = std::make_unique<RenderGraph>(rhi_);
		setupRenderGraph();
		renderGraph_->compile();
	}

	void DeferredRendererRG::render(uint32_t frameIndex)
	{
		renderGraph_->execute(frameIndex);
	}

	RHIImage* DeferredRendererRG::getFinalOutput() const
	{
		return renderGraph_->getFinalOutput();
	}

	void DeferredRendererRG::printDebugInfo() const
	{
		printLog("\n=== DeferredRendererRG Debug Info ===");
		renderGraph_->printExecutionOrder();
		renderGraph_->printResourceUsage();
		printLog("=====================================\n");
	}

	void DeferredRendererRG::setupRenderGraph()
	{
		printLog("  Setting up complete rendering pipeline...");

		// ========================================
		// 1. Shadow Pass
		// ========================================
		printLog("    [1/6] Adding ShadowPassRG...");
		auto shadow = std::make_unique<ShadowPassRG>(rhi_);
		shadow->initialize();
		auto shadowMapHandle = shadow->getShadowMapHandle();
		renderGraph_->addPass(std::move(shadow));

		// ========================================
		// 2. G-Buffer Pass
		// ========================================
		printLog("    [2/6] Adding GBufferPassRG...");
		auto gbuffer = std::make_unique<GBufferPassRG>(rhi_);
		gbuffer->resize(width_, height_);
		gbuffer->initialize();
		
		auto albedoHandle = gbuffer->getAlbedoHandle();
		auto normalHandle = gbuffer->getNormalHandle();
		auto positionHandle = gbuffer->getPositionHandle();
		auto metallicRoughnessHandle = gbuffer->getMetallicRoughnessHandle();
		auto depthHandle = gbuffer->getDepthHandle();
		
		renderGraph_->addPass(std::move(gbuffer));

		// ========================================
		// 3. Lighting Pass
		// ========================================
		printLog("    [3/6] Adding LightingPassRG...");
		auto lighting = std::make_unique<LightingPassRG>(rhi_);
		lighting->resize(width_, height_);
		
		// ✅ 의존성 설정: Shadow + GBuffer → Lighting
		lighting->setShadowMapHandle(shadowMapHandle);
		lighting->setAlbedoHandle(albedoHandle);
		lighting->setNormalHandle(normalHandle);
		lighting->setPositionHandle(positionHandle);
		lighting->setMetallicRoughnessHandle(metallicRoughnessHandle);
		lighting->setDepthHandle(depthHandle);
		
		lighting->initialize();
		auto lightingHandle = lighting->getLightingHandle();
		
		renderGraph_->addPass(std::move(lighting));

		// ========================================
		// 4. Forward Pass
		// ========================================
		printLog("    [4/6] Adding ForwardPassRG...");
		auto forward = std::make_unique<ForwardPassRG>(rhi_);
		forward->resize(width_, height_);
		
		// ✅ 의존성 설정: Lighting + Depth → Forward
		forward->setLightingHandle(lightingHandle);
		forward->setDepthHandle(depthHandle);
		
		forward->initialize();
		auto forwardHandle = forward->getForwardHandle();
		
		renderGraph_->addPass(std::move(forward));

		// ========================================
		// 5. Post-Process Pass
		// ========================================
		printLog("    [5/6] Adding PostProcessPassRG...");
		auto postProcess = std::make_unique<PostProcessPassRG>(rhi_);
		postProcess->resize(width_, height_);
		
		// ✅ 의존성 설정: Forward → PostProcess
		postProcess->setHDRHandle(forwardHandle);
		
		postProcess->initialize();
		auto ldrHandle = postProcess->getLDRHandle();
		
		renderGraph_->addPass(std::move(postProcess));

		// ========================================
		// 6. GUI Pass
		// ========================================
		printLog("    [6/6] Adding GUIPassRG...");
		auto gui = std::make_unique<GUIPassRG>(rhi_);
		gui->resize(width_, height_);
		
		// ✅ 의존성 설정: PostProcess → GUI
		gui->setSceneHandle(ldrHandle);
		
		gui->initialize();
		
		renderGraph_->addPass(std::move(gui));

		printLog("  ✅ All 6 passes added to RenderGraph");
	}

} // namespace BinRenderer
