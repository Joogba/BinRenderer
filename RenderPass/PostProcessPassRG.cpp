#include "PostProcessPassRG.h"
#include "../Core/Logger.h"

namespace BinRenderer
{
	PostProcessPassRG::PostProcessPassRG(RHI* rhi)
		: RGPass<PostProcessPassData>(rhi, "PostProcessPass")
	{
	}

	PostProcessPassRG::~PostProcessPassRG()
	{
		shutdown();
	}

	bool PostProcessPassRG::initialize()
	{
		printLog("[PostProcessPassRG] Initializing...");
		createPipeline();
		printLog("[PostProcessPassRG] Initialized successfully");
		return true;
	}

	void PostProcessPassRG::shutdown()
	{
		destroyPipeline();
	}

	void PostProcessPassRG::setup(PostProcessPassData& data, RenderGraphBuilder& builder)
	{
		printLog("[PostProcessPassRG] Setup - Declaring inputs and outputs");

		// 입력: HDR Image (자동 의존성!)
		data.hdrIn = builder.readTexture(hdrHandle_);

		// 출력: LDR Image (Tone Mapped)
		RGTextureDesc ldrDesc;
		ldrDesc.name = "PostProcess_LDR";
		ldrDesc.width = width_;
		ldrDesc.height = height_;
		ldrDesc.format = RHI_FORMAT_R8G8B8A8_UNORM;
		ldrDesc.usage = RHI_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | 
		    RHI_IMAGE_USAGE_SAMPLED_BIT;

		data.ldrOut = builder.writeTexture(builder.createTexture(ldrDesc));
	}

	void PostProcessPassRG::execute(const PostProcessPassData& data, RHI* rhi, uint32_t frameIndex)
	{
		// TODO: 실제 Post-Processing
		// - Tone Mapping (Reinhard, ACES, Filmic 등)
		// - FXAA
		// - Bloom
		// - Color Grading
	}

	void PostProcessPassRG::createPipeline()
	{
		// TODO: Post-Process 파이프라인 생성
		// - Fullscreen Triangle
		// - Fragment Shader에서 Tone Mapping
	}

	void PostProcessPassRG::destroyPipeline()
	{
		if (pipeline_.isValid()) {
			rhi_->destroyPipeline(pipeline_);
			pipeline_ = {};
		}

		if (vertexShader_.isValid()) {
			rhi_->destroyShader(vertexShader_);
			vertexShader_ = {};
		}

		if (fragmentShader_.isValid()) {
			rhi_->destroyShader(fragmentShader_);
			fragmentShader_ = {};
		}
	}

} // namespace BinRenderer
