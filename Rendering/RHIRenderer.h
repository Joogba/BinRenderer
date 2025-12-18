#pragma once

#include "../RHI/Core/RHI.h"
#include "../RenderPass/RenderGraph/RGGraph.h"
#include "../Core/RHIScene.h"
#include "../Scene/Animation.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace BinRenderer
{
	/**
	 * @brief 플랫폼 독립적 Renderer (RHI 기반)
	 */
	class RHIRenderer
	{
	public:
		RHIRenderer(RHI* rhi);
		~RHIRenderer();

		// 초기화
		bool initialize(uint32_t width, uint32_t height);
		void shutdown();

		// 프레임 렌더링
		void beginFrame(uint32_t frameIndex);
		void render(RHIScene& scene, uint32_t frameIndex);
		void endFrame(uint32_t frameIndex);

		// 리소스 관리
		void resize(uint32_t width, uint32_t height);

		// RenderGraph 접근
		RenderGraph* getRenderGraph() { return renderGraph_.get(); }

	private:
		RHI* rhi_;
		std::unique_ptr<RenderGraph> renderGraph_;

		uint32_t width_ = 0;
		uint32_t height_ = 0;

		// 렌더 패스들
		void setupRenderPasses();
	};

} // namespace BinRenderer
