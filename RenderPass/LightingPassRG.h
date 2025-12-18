#pragma once

#include "RGPassBase.h"

namespace BinRenderer
{
	/**
	 * @brief Lighting Pass 데이터
	 */
	struct LightingPassData
	{
		// 입력
		RGTextureHandle shadowMapIn;
		RGTextureHandle albedoIn;
		RGTextureHandle normalIn;
		RGTextureHandle positionIn;
		RGTextureHandle metallicRoughnessIn;
		RGTextureHandle depthIn;

		// 출력
		RGTextureHandle lightingOut;  // HDR
	};

	/**
	 * @brief Lighting Pass (Deferred Lighting)
	 * 
	 * @features
	 * - PBR (Physically Based Rendering)
	 * - Shadow Mapping
	 * - IBL (Image-Based Lighting)
	 * 
	 * @inputs
	 * - Shadow Map
	 * - G-Buffer (Albedo, Normal, Position, Metallic-Roughness, Depth)
	 * 
	 * @outputs
	 * - Lighting HDR (R16G16B16A16_SFLOAT)
	 */
	class LightingPassRG : public RGPass<LightingPassData>
	{
	public:
		LightingPassRG(RHI* rhi);
		~LightingPassRG() override;

		// RGPass 인터페이스
		void setup(LightingPassData& data, RenderGraphBuilder& builder) override;
		void execute(const LightingPassData& data, RHI* rhi, uint32_t frameIndex) override;

		// 기존 API 호환
		bool initialize() override;
		void shutdown() override;

		// 입력 핸들 설정 (setup 전에 호출)
		void setShadowMapHandle(RGTextureHandle handle) { shadowMapHandle_ = handle; }
		void setAlbedoHandle(RGTextureHandle handle) { albedoHandle_ = handle; }
		void setNormalHandle(RGTextureHandle handle) { normalHandle_ = handle; }
		void setPositionHandle(RGTextureHandle handle) { positionHandle_ = handle; }
		void setMetallicRoughnessHandle(RGTextureHandle handle) { metallicRoughnessHandle_ = handle; }
		void setDepthHandle(RGTextureHandle handle) { depthHandle_ = handle; }

		// 출력 핸들
		RGTextureHandle getLightingHandle() const { return getData().lightingOut; }

	private:
		// 입력 핸들 (다른 Pass에서 받음)
		RGTextureHandle shadowMapHandle_;
		RGTextureHandle albedoHandle_;
		RGTextureHandle normalHandle_;
		RGTextureHandle positionHandle_;
		RGTextureHandle metallicRoughnessHandle_;
		RGTextureHandle depthHandle_;

		// 파이프라인
		RHIPipeline* pipeline_ = nullptr;
		RHIShader* vertexShader_ = nullptr;
		RHIShader* fragmentShader_ = nullptr;

		void createPipeline();
		void destroyPipeline();
	};

} // namespace BinRenderer
