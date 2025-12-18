#pragma once

#include "../RenderPass/RenderGraph/RGGraph.h"
#include "../RenderPass/GBufferPassRG.h"
#include "../RenderPass/ShadowPassRG.h"
#include "../RenderPass/LightingPassRG.h"
#include "../RenderPass/ForwardPassRG.h"
#include "../RenderPass/PostProcessPassRG.h"
#include "../RenderPass/GUIPassRG.h"
#include "../RHI/Core/RHI.h"
#include <memory>

namespace BinRenderer
{
	/**
	 * @brief RenderGraph 기반 Deferred Renderer (완전판)
	 * 
	 * 모든 Pass를 RenderGraph 시스템으로 통합
	 * 
	 * @features
	 * - 6가지 Pass 클래스 기반
	 * - 자동 의존성 관리
	 * - 타입 안전한 PassData
	 * 
	 * @pipeline
	 * 1. ShadowPass: 그림자 맵
	 * 2. GBufferPass: G-Buffer 생성
	 * 3. LightingPass: Deferred Lighting
	 * 4. ForwardPass: 투명 오브젝트
	 * 5. PostProcessPass: Tone Mapping, FXAA
	 * 6. GUIPass: ImGui UI
	 */
	class DeferredRendererRG
	{
	public:
		DeferredRendererRG(RHI* rhi);
		~DeferredRendererRG();

		bool initialize(uint32_t width, uint32_t height);
		void shutdown();
		void resize(uint32_t width, uint32_t height);
		void render(uint32_t frameIndex);

		RHIImage* getFinalOutput() const;
		RenderGraph* getRenderGraph() { return renderGraph_.get(); }

		void printDebugInfo() const;

	private:
		RHI* rhi_ = nullptr;
		std::unique_ptr<RenderGraph> renderGraph_;

		uint32_t width_ = 0;
		uint32_t height_ = 0;

		void setupRenderGraph();
	};

} // namespace BinRenderer
