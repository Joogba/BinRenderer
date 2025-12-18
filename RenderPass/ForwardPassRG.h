#pragma once

#include "RGPassBase.h"

namespace BinRenderer
{
	/**
	 * @brief Forward Pass 데이터
	 */
	struct ForwardPassData
	{
		// 입력
		RGTextureHandle lightingIn;  // HDR from LightingPass
		RGTextureHandle depthIn;  // Depth from GBufferPass

		// 출력
		RGTextureHandle forwardOut;  // HDR + Transparent Objects
	};

	/**
	 * @brief Forward Pass (투명 오브젝트 렌더링)
	 * 
	 * @features
	 * - 투명 오브젝트 렌더링
	 * - Depth Test 사용
	 * - Alpha Blending
	 * 
	 * @inputs
	 * - Lighting HDR (from LightingPass)
	 * - Depth (from GBufferPass)
	 * 
	 * @outputs
	 * - Forward Output (HDR, R16G16B16A16_SFLOAT)
	 */
	class ForwardPassRG : public RGPass<ForwardPassData>
	{
	public:
		ForwardPassRG(RHI* rhi);
		~ForwardPassRG() override;

		// RGPass 인터페이스
		void setup(ForwardPassData& data, RenderGraphBuilder& builder) override;
		void execute(const ForwardPassData& data, RHI* rhi, uint32_t frameIndex) override;

		// 기존 API 호환
		bool initialize() override;
		void shutdown() override;

		// 입력 핸들 설정 (setup 전에 호출)
		void setLightingHandle(RGTextureHandle handle) { lightingHandle_ = handle; }
		void setDepthHandle(RGTextureHandle handle) { depthHandle_ = handle; }

		// 출력 핸들
		RGTextureHandle getForwardHandle() const { return getData().forwardOut; }

	private:
		// 입력 핸들
		RGTextureHandle lightingHandle_;
		RGTextureHandle depthHandle_;

		// 파이프라인
		RHIPipeline* pipeline_ = nullptr;
		RHIShader* vertexShader_ = nullptr;
		RHIShader* fragmentShader_ = nullptr;

		void createPipeline();
		void destroyPipeline();
	};

} // namespace BinRenderer
