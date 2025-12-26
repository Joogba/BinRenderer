#pragma once

#include "RGPassBase.h"

namespace BinRenderer
{
	/**
	 * @brief Post-Process Pass 데이터
	 */
	struct PostProcessPassData
	{
		// 입력
		RGTextureHandle hdrIn;  // HDR from ForwardPass

		// 출력
		RGTextureHandle ldrOut;  // LDR (Tone Mapped)
	};

	/**
	 * @brief Post-Process Pass (Tone Mapping, FXAA 등)
	 * 
	 * @features
	 * - Tone Mapping (HDR → LDR)
	 * - FXAA (Anti-Aliasing)
	 * - Bloom
	 * - Color Grading
	 * 
	 * @inputs
	 * - HDR Image (R16G16B16A16_SFLOAT)
	 * 
	 * @outputs
	 * - LDR Image (R8G8B8A8_UNORM)
	 */
	class PostProcessPassRG : public RGPass<PostProcessPassData>
	{
	public:
		PostProcessPassRG(RHI* rhi);
		~PostProcessPassRG() override;

		// RGPass 인터페이스
		void setup(PostProcessPassData& data, RenderGraphBuilder& builder) override;
		void execute(const PostProcessPassData& data, RHI* rhi, uint32_t frameIndex) override;

		// 기존 API 호환
		bool initialize() override;
		void shutdown() override;

		// 입력 핸들 설정 (setup 전에 호출)
		void setHDRHandle(RGTextureHandle handle) { hdrHandle_ = handle; }

		// 출력 핸들
		RGTextureHandle getLDRHandle() const { return getData().ldrOut; }

	private:
		// 입력 핸들
		RGTextureHandle hdrHandle_;

		// 파이프라인
		RHIPipelineHandle pipeline_;
		RHIShaderHandle vertexShader_;
		RHIShaderHandle fragmentShader_;

		void createPipeline();
		void destroyPipeline();
	};

} // namespace BinRenderer
