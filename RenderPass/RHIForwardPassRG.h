#pragma once

#include "RGPassBase.h"
#include "../Core/RHIScene.h"
#include <memory>

namespace BinRenderer
{
	/**
	 * @brief RHI 기반 Forward Pass 데이터
	 */
	struct RHIForwardPassData
	{
		RGTextureHandle colorOutput;
		RGTextureHandle depthOutput;
	};

	/**
	 * @brief RHI 기반 Forward Rendering Pass
	 */
	class RHIForwardPassRG : public RGPass<RHIForwardPassData>
	{
	public:
		RHIForwardPassRG(RHI* rhi);
		~RHIForwardPassRG() override;

		// RGPass 인터페이스 (PassData 버전)
		void setup(RHIForwardPassData& data, RenderGraphBuilder& builder) override;
		void execute(const RHIForwardPassData& data, RHI* rhi, uint32_t frameIndex) override;

		// 기존 API 호환
		bool initialize() override;
		void shutdown() override;

		// Scene 설정
		void setScene(RHIScene* scene) { scene_ = scene; }
		void setViewProjection(const glm::mat4& vp) { viewProjection_ = vp; }

		// 출력 핸들
		RGTextureHandle getColorHandle() const { return getData().colorOutput; }
		RGTextureHandle getDepthHandle() const { return getData().depthOutput; }

	private:
		RHIScene* scene_ = nullptr;
		glm::mat4 viewProjection_ = glm::mat4(1.0f);

		// 파이프라인
		RHIPipeline* pipeline_ = nullptr;
		RHIShader* vertexShader_ = nullptr;
		RHIShader* fragmentShader_ = nullptr;

		void createPipeline();
		void destroyPipeline();
	};

} // namespace BinRenderer
