#include "GBufferPass.h"

namespace BinRenderer
{
	GBufferPass::GBufferPass(RHI* rhi)
		: RenderPassBase(rhi, "GBufferPass")
	{
	}

	GBufferPass::~GBufferPass()
	{
		shutdown();
	}

	bool GBufferPass::initialize()
	{
		width_ = 1920;
		height_ = 1080;

		createRenderTargets();
		createRenderPass();
		createFramebuffer();
		createPipeline();

		return true;
	}

	void GBufferPass::shutdown()
	{
		if (pipeline_)
		{
			rhi_->destroyPipeline(pipeline_);
			pipeline_ = nullptr;
		}

		destroyRenderTargets();
	}

	void GBufferPass::resize(uint32_t width, uint32_t height)
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

	void GBufferPass::execute(uint32_t frameIndex)
	{
		// Clear values for G-Buffer
		RHIClearValue clearValues[5];
		clearValues[0].color[0] = 0.0f; clearValues[0].color[1] = 0.0f; clearValues[0].color[2] = 0.0f; clearValues[0].color[3] = 1.0f; // Albedo
		clearValues[1].color[0] = 0.0f; clearValues[1].color[1] = 0.0f; clearValues[1].color[2] = 0.0f; clearValues[1].color[3] = 1.0f; // Normal
		clearValues[2].color[0] = 0.0f; clearValues[2].color[1] = 0.0f; clearValues[2].color[2] = 0.0f; clearValues[2].color[3] = 1.0f; // Position
		clearValues[3].color[0] = 0.0f; clearValues[3].color[1] = 0.0f; clearValues[3].color[2] = 0.0f; clearValues[3].color[3] = 1.0f; // Metallic-Roughness
		clearValues[4].depthStencil.depth = 1.0f;
		clearValues[4].depthStencil.stencil = 0;

		beginRenderPass(frameIndex, clearValues, 5);

		// TODO: 지오메트리 렌더링
		  // rhi_->cmdBindPipeline(pipeline_);
			 // ... draw calls ...

		endRenderPass();
	}

	void GBufferPass::createRenderTargets()
	{
		// Albedo (RGBA8)
		RHIImageCreateInfo albedoInfo{};
		albedoInfo.width = width_;
		albedoInfo.height = height_;
		albedoInfo.depth = 1;
		albedoInfo.mipLevels = 1;
		albedoInfo.arrayLayers = 1;
		albedoInfo.format = RHI_FORMAT_R8G8B8A8_UNORM;
		albedoInfo.tiling = RHI_IMAGE_TILING_OPTIMAL;
		albedoInfo.usage = RHI_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | RHI_IMAGE_USAGE_SAMPLED_BIT;
		albedoInfo.samples = RHI_SAMPLE_COUNT_1_BIT;
		albedoImage_ = rhi_->createImage(albedoInfo);

		// Normal (RGBA16F)
		RHIImageCreateInfo normalInfo = albedoInfo;
		normalInfo.format = RHI_FORMAT_R16G16B16A16_SFLOAT;
		normalImage_ = rhi_->createImage(normalInfo);

		// Position (RGBA32F)
		RHIImageCreateInfo positionInfo = albedoInfo;
		positionInfo.format = RHI_FORMAT_R32G32B32A32_SFLOAT;
		positionImage_ = rhi_->createImage(positionInfo);

		// Metallic-Roughness (RGBA8)
		metallicRoughnessImage_ = rhi_->createImage(albedoInfo);

		// Depth (D32)
		RHIImageCreateInfo depthInfo{};
		depthInfo.width = width_;
		depthInfo.height = height_;
		depthInfo.depth = 1;
		depthInfo.mipLevels = 1;
		depthInfo.arrayLayers = 1;
		depthInfo.format = RHI_FORMAT_D32_SFLOAT;
		depthInfo.tiling = RHI_IMAGE_TILING_OPTIMAL;
		depthInfo.usage = RHI_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | RHI_IMAGE_USAGE_SAMPLED_BIT;
		depthInfo.samples = RHI_SAMPLE_COUNT_1_BIT;
		depthImage_ = rhi_->createImage(depthInfo);

		// TODO: Image Views 생성
	}

	void GBufferPass::destroyRenderTargets()
	{
		if (albedoImage_) rhi_->destroyImage(albedoImage_);
		if (normalImage_) rhi_->destroyImage(normalImage_);
		if (positionImage_) rhi_->destroyImage(positionImage_);
		if (metallicRoughnessImage_) rhi_->destroyImage(metallicRoughnessImage_);
		if (depthImage_) rhi_->destroyImage(depthImage_);

		albedoImage_ = normalImage_ = positionImage_ = metallicRoughnessImage_ = depthImage_ = nullptr;
	}

	void GBufferPass::createRenderPass()
	{
		// TODO: RHI에서 RenderPass 생성 지원 필요
	}

	void GBufferPass::createFramebuffer()
	{
		// TODO: RHI에서 Framebuffer 생성 지원 필요
	}

	void GBufferPass::createPipeline()
	{
		// TODO: G-Buffer용 파이프라인 생성
	}

} // namespace BinRenderer
