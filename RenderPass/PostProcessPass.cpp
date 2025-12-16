#include "PostProcessPass.h"

namespace BinRenderer
{
	PostProcessPass::PostProcessPass(RHI* rhi)
		: RenderPassBase(rhi, "PostProcessPass")
	{
	}

	PostProcessPass::~PostProcessPass()
	{
		shutdown();
	}

	bool PostProcessPass::initialize()
	{
		createRenderTargets();
		createRenderPass();
		createFramebuffer();
		createPipelines();

		return true;
	}

	void PostProcessPass::shutdown()
	{
		if (toneMappingPipeline_)
		{
			rhi_->destroyPipeline(toneMappingPipeline_);
			toneMappingPipeline_ = nullptr;
		}

		if (fxaaPipeline_)
		{
			rhi_->destroyPipeline(fxaaPipeline_);
			fxaaPipeline_ = nullptr;
		}

		destroyRenderTargets();
	}

	void PostProcessPass::resize(uint32_t width, uint32_t height)
	{
		if (width_ == width && height_ == height)
		{
			return;
		}

		width_ = width;
		height_ = height;

		destroyRenderTargets();
		createRenderTargets();
		createFramebuffer();
	}

	void PostProcessPass::execute(uint32_t frameIndex)
	{
		if (!inputImage_)
		{
			return;
		}

		RHIClearValue clearValue{};
		clearValue.color[0] = 0.0f;
		clearValue.color[1] = 0.0f;
		clearValue.color[2] = 0.0f;
		clearValue.color[3] = 1.0f;

		beginRenderPass(frameIndex, &clearValue, 1);

		// 1. Tone Mapping
		if (options_.enableToneMapping && toneMappingPipeline_)
		{
			// rhi_->cmdBindPipeline(toneMappingPipeline_);
	  // rhi_->cmdPushConstants(..., &options_.exposure);
			 // rhi_->cmdDraw(3, 1, 0, 0); // Fullscreen Triangle
		}

		// 2. FXAA (Optional)
		if (options_.enableFXAA && fxaaPipeline_)
		{
			// rhi_->cmdBindPipeline(fxaaPipeline_);
		 // rhi_->cmdDraw(3, 1, 0, 0);
		}

		endRenderPass();
	}

	void PostProcessPass::setInputTexture(RHIImage* inputTexture)
	{
		inputImage_ = inputTexture;
	}

	void PostProcessPass::createRenderTargets()
	{
		// Output (RGBA8 - LDR)
		RHIImageCreateInfo outputInfo{};
		outputInfo.width = width_;
		outputInfo.height = height_;
		outputInfo.depth = 1;
		outputInfo.mipLevels = 1;
		outputInfo.arrayLayers = 1;
		outputInfo.format = RHI_FORMAT_R8G8B8A8_UNORM;
		outputInfo.tiling = RHI_IMAGE_TILING_OPTIMAL;
		outputInfo.usage = RHI_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | RHI_IMAGE_USAGE_SAMPLED_BIT;
		outputInfo.samples = RHI_SAMPLE_COUNT_1_BIT;
		outputImage_ = rhi_->createImage(outputInfo);

		// TODO: Image View, Sampler 생성
	}

	void PostProcessPass::destroyRenderTargets()
	{
		if (outputImage_)
		{
			rhi_->destroyImage(outputImage_);
			outputImage_ = nullptr;
		}
	}

	void PostProcessPass::createRenderPass()
	{
		// TODO: RenderPass 생성
	}

	void PostProcessPass::createFramebuffer()
	{
		// TODO: Framebuffer 생성
	}

	void PostProcessPass::createPipelines()
	{
		// TODO: Tone Mapping, FXAA 파이프라인 생성
	}

} // namespace BinRenderer
