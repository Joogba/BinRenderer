#include "ShadowPassRG.h"
#include "../Core/Logger.h"

namespace BinRenderer
{
	ShadowPassRG::ShadowPassRG(RHI* rhi)
		: RGPass<ShadowPassData>(rhi, "ShadowPass")
	{
	}

	ShadowPassRG::~ShadowPassRG()
	{
		shutdown();
	}

	bool ShadowPassRG::initialize()
	{
		printLog("[ShadowPassRG] Initializing...");
		createPipeline();
		printLog("[ShadowPassRG] Initialized successfully");
		return true;
	}

	void ShadowPassRG::shutdown()
	{
		destroyPipeline();
	}

	void ShadowPassRG::setup(ShadowPassData& data, RenderGraphBuilder& builder)
	{
		printLog("[ShadowPassRG] Setup - Creating shadow map");

		// Shadow Map (2048x2048, Depth)
		RGTextureDesc shadowDesc;
		shadowDesc.name = "ShadowMap";
		shadowDesc.width = 2048;
		shadowDesc.height = 2048;
		shadowDesc.format = RHI_FORMAT_D32_SFLOAT;
		shadowDesc.usage = RHI_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | 
		         RHI_IMAGE_USAGE_SAMPLED_BIT;

		data.shadowMap = builder.writeTexture(builder.createTexture(shadowDesc));
	}

	void ShadowPassRG::execute(const ShadowPassData& data, RHI* rhi, uint32_t frameIndex)
	{
		// TODO: 실제 Shadow Map 렌더링
		// - Light Space 변환
		// - Depth-only 렌더링
	}

	void ShadowPassRG::createPipeline()
	{
		// TODO: Shadow Map 파이프라인 생성
	}

	void ShadowPassRG::destroyPipeline()
	{
		if (pipeline_) {
			rhi_->destroyPipeline(pipeline_);
			pipeline_ = nullptr;
		}

		if (vertexShader_) {
			rhi_->destroyShader(vertexShader_);
			vertexShader_ = nullptr;
		}
	}

} // namespace BinRenderer
