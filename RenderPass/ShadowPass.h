#pragma once

#include "RenderPassBase.h"
#include "../RHI/Pipeline/RHIFramebuffer.h"

namespace BinRenderer
{
	/**
  * @brief 그림자 맵 생성 패스
	 *
  * Directional Light용 Cascaded Shadow Maps 지원
	 */
	class ShadowPass : public RenderPassBase
	{
	public:
		ShadowPass(RHI* rhi);
		~ShadowPass() override;

		bool initialize() override;
		void shutdown() override;
		void resize(uint32_t width, uint32_t height) override;
		void execute(uint32_t frameIndex) override;

		// 그림자 맵 접근
		RHIImage* getShadowMap(uint32_t cascadeIndex = 0) const;

		// Cascade 설정
		void setCascadeCount(uint32_t count);
		uint32_t getCascadeCount() const { return cascadeCount_; }

		// 그림자 맵 해상도
		void setShadowMapResolution(uint32_t resolution);
		uint32_t getShadowMapResolution() const { return shadowMapResolution_; }

	private:
		uint32_t cascadeCount_ = 4;  // Cascaded Shadow Maps
		uint32_t shadowMapResolution_ = 2048;

		// 각 Cascade별 그림자 맵
		std::vector<RHIImage*> shadowMaps_;
		std::vector<RHIImageView*> shadowMapViews_;
		std::vector<RHIFramebuffer*> shadowMapFramebuffers_;

		RHIPipeline* pipeline_ = nullptr;

		void createShadowMaps();
		void destroyShadowMaps();
		void createRenderPass();
		void createPipeline();
	};

} // namespace BinRenderer
