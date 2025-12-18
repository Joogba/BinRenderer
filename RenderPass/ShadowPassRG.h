#pragma once

#include "RGPassBase.h"

namespace BinRenderer
{
	/**
	 * @brief Shadow Pass 데이터
	 */
	struct ShadowPassData
	{
		RGTextureHandle shadowMap;  // 2048x2048 Depth Map
	};

	/**
	 * @brief Shadow Pass (그림자 맵 생성)
	 * 
	 * @features
	 * - Directional Light용 그림자 맵
	 * - Depth-only 렌더링
	 * 
	 * @outputs
	 * - ShadowMap (2048x2048, D32_SFLOAT)
	 */
	class ShadowPassRG : public RGPass<ShadowPassData>
	{
	public:
		ShadowPassRG(RHI* rhi);
		~ShadowPassRG() override;

		// RGPass 인터페이스
		void setup(ShadowPassData& data, RenderGraphBuilder& builder) override;
		void execute(const ShadowPassData& data, RHI* rhi, uint32_t frameIndex) override;

		// 기존 API 호환
		bool initialize() override;
		void shutdown() override;

		// Shadow Map 핸들
		RGTextureHandle getShadowMapHandle() const { return getData().shadowMap; }

	private:
		RHIPipeline* pipeline_ = nullptr;
		RHIShader* vertexShader_ = nullptr;

		void createPipeline();
		void destroyPipeline();
	};

} // namespace BinRenderer
