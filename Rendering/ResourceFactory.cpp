#include "ResourceFactory.h"
#include "../Core/Logger.h"

namespace BinRenderer
{
	ResourceFactory::ResourceFactory(RHI* rhi)
		: rhi_(rhi)
	{
	}

	ResourceFactory::~ResourceFactory()
	{
		cleanup();
	}

	RHIImageView* ResourceFactory::createDefaultTexture2D(uint32_t size)
	{
		RHIImageCreateInfo imageInfo{};
		imageInfo.width = size;
		imageInfo.height = size;
		imageInfo.depth = 1;
		imageInfo.format = RHI_FORMAT_R8G8B8A8_UNORM;
		imageInfo.usage = RHI_IMAGE_USAGE_SAMPLED_BIT | RHI_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.samples = RHI_SAMPLE_COUNT_1_BIT;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;

		RHIImage* image = rhi_->createImage(imageInfo);
		if (!image)
		{
			printLog("[ResourceFactory] ❌ Failed to create default 2D texture");
			return nullptr;
		}

		RHIImageViewCreateInfo viewInfo{};
		viewInfo.viewType = RHI_IMAGE_VIEW_TYPE_2D;
		viewInfo.aspectMask = RHI_IMAGE_ASPECT_COLOR_BIT;

		RHIImageView* imageView = rhi_->createImageView(image, viewInfo);
		if (!imageView)
		{
			printLog("[ResourceFactory] ❌ Failed to create image view for 2D texture");
			rhi_->destroyImage(image);
			return nullptr;
		}

		// 추적 목록에 추가 (자동 정리)
		images_.push_back(image);
		imageViews_.push_back(imageView);

		printLog("[ResourceFactory] ✅ Default 2D texture created ({}x{})", size, size);
		return imageView;
	}

	RHIImageView* ResourceFactory::createDefaultCubemap(uint32_t size)
	{
		RHIImageCreateInfo imageInfo{};
		imageInfo.width = size;
		imageInfo.height = size;
		imageInfo.depth = 1;
		imageInfo.arrayLayers = 6; // Cubemap faces
		imageInfo.format = RHI_FORMAT_R8G8B8A8_UNORM;
		imageInfo.usage = RHI_IMAGE_USAGE_SAMPLED_BIT | RHI_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.samples = RHI_SAMPLE_COUNT_1_BIT;
		imageInfo.mipLevels = 1;
		imageInfo.flags = RHI_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		RHIImage* image = rhi_->createImage(imageInfo);
		if (!image)
		{
			printLog("[ResourceFactory] ❌ Failed to create default cubemap");
			return nullptr;
		}

		RHIImageViewCreateInfo viewInfo{};
		viewInfo.viewType = RHI_IMAGE_VIEW_TYPE_CUBE;
		viewInfo.aspectMask = RHI_IMAGE_ASPECT_COLOR_BIT;

		RHIImageView* imageView = rhi_->createImageView(image, viewInfo);
		if (!imageView)
		{
			printLog("[ResourceFactory] ❌ Failed to create image view for cubemap");
			rhi_->destroyImage(image);
			return nullptr;
		}

		images_.push_back(image);
		imageViews_.push_back(imageView);

		printLog("[ResourceFactory] ✅ Default cubemap created ({}x{} 6-faces)", size, size);
		return imageView;
	}

	RHIImageView* ResourceFactory::createDefaultDepthTexture(uint32_t size)
	{
		RHIImageCreateInfo imageInfo{};
		imageInfo.width = size;
		imageInfo.height = size;
		imageInfo.depth = 1;
		imageInfo.format = RHI_FORMAT_D32_SFLOAT;
		imageInfo.usage = RHI_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | RHI_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.samples = RHI_SAMPLE_COUNT_1_BIT;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;

		RHIImage* image = rhi_->createImage(imageInfo);
		if (!image)
		{
			printLog("[ResourceFactory] ❌ Failed to create default depth texture");
			return nullptr;
		}

		RHIImageViewCreateInfo viewInfo{};
		viewInfo.viewType = RHI_IMAGE_VIEW_TYPE_2D;
		viewInfo.aspectMask = RHI_IMAGE_ASPECT_DEPTH_BIT;

		RHIImageView* imageView = rhi_->createImageView(image, viewInfo);
		if (!imageView)
		{
			printLog("[ResourceFactory] ❌ Failed to create image view for depth texture");
			rhi_->destroyImage(image);
			return nullptr;
		}

		images_.push_back(image);
		imageViews_.push_back(imageView);

		printLog("[ResourceFactory] ✅ Default depth texture created ({}x{})", size, size);
		return imageView;
	}

	RHISampler* ResourceFactory::createDefaultSampler()
	{
		RHISamplerCreateInfo samplerInfo{};
		// 기본값 사용

		RHISampler* sampler = rhi_->createSampler(samplerInfo);
		if (!sampler)
		{
			printLog("[ResourceFactory] ❌ Failed to create default sampler");
			return nullptr;
		}

		samplers_.push_back(sampler);

		printLog("[ResourceFactory] ✅ Default sampler created");
		return sampler;
	}

	void ResourceFactory::cleanup()
	{
		printLog("[ResourceFactory] Cleaning up {} image views, {} images, {} samplers",
			imageViews_.size(), images_.size(), samplers_.size());

		// ImageView 먼저 정리
		for (auto* view : imageViews_)
		{
			if (view) rhi_->destroyImageView(view);
		}
		imageViews_.clear();

		// Image 정리
		for (auto* image : images_)
		{
			if (image) rhi_->destroyImage(image);
		}
		images_.clear();

		// Sampler 정리
		for (auto* sampler : samplers_)
		{
			if (sampler) rhi_->destroySampler(sampler);
		}
		samplers_.clear();

		printLog("[ResourceFactory] ✅ Cleanup complete");
	}

} // namespace BinRenderer
