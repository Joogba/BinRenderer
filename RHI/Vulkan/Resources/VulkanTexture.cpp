#include "VulkanTexture.h"
#include "VulkanBuffer.h"
#include "../VulkanRHI.h"
#include "../Commands/VulkanCommandPool.h"
#include "../Commands/VulkanCommandBuffer.h"
#include "Vulkan/Logger.h"

// stb_image 사용 (이미지 로딩)
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include <stb_image.h>
#include <cmath>

namespace BinRenderer::Vulkan
{
	VulkanTexture::VulkanTexture(VkDevice device, VkPhysicalDevice physicalDevice, VulkanRHI* rhi)
		: device_(device), physicalDevice_(physicalDevice), rhi_(rhi)
	{
	}

	VulkanTexture::~VulkanTexture()
	{
		destroy();
	}

	bool VulkanTexture::create2D(uint32_t width, uint32_t height, VkFormat format, uint32_t mipLevels, bool createSampler)
	{
		width_ = width;
		height_ = height;
		mipLevels_ = mipLevels;

		// 이미지 생성
		image_ = new VulkanImage(device_, physicalDevice_);
		RHIImageCreateInfo imageInfo{};
		imageInfo.width = width;
		imageInfo.height = height;
		imageInfo.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = static_cast<RHIFormat>(format);
		imageInfo.tiling = RHI_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = RHI_IMAGE_USAGE_SAMPLED_BIT | RHI_IMAGE_USAGE_TRANSFER_DST_BIT | RHI_IMAGE_USAGE_TRANSFER_SRC_BIT;
		imageInfo.samples = RHI_SAMPLE_COUNT_1_BIT;

		if (!image_->create(imageInfo))
		{
			delete image_;
			image_ = nullptr;
			return false;
		}

		// 이미지 뷰 생성
		imageView_ = new VulkanImageView(device_, image_);
		if (!imageView_->create(VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT))
		{
			delete imageView_;
			imageView_ = nullptr;
			delete image_;
			image_ = nullptr;
			return false;
		}

		// 샘플러 생성
		if (createSampler)
		{
			sampler_ = new VulkanSampler(device_);
			if (!sampler_->createLinear())
			{
				delete sampler_;
				sampler_ = nullptr;
				// 샘플러 실패해도 텍스처는 사용 가능
			}
		}

		return true;
	}

	bool VulkanTexture::loadFromFile(const char* filename, bool generateMipmaps)
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(filename, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		if (!pixels)
		{
			printLog("Failed to load texture: {}", filename);
			return false;
		}

		VkDeviceSize imageSize = texWidth * texHeight * 4;
		mipLevels_ = generateMipmaps ? static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1 : 1;

		// 텍스처 생성
		if (!createFromData(pixels, texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, mipLevels_))
		{
			stbi_image_free(pixels);
			return false;
		}

		stbi_image_free(pixels);
		return true;
	}

	bool VulkanTexture::createFromData(const void* data, uint32_t width, uint32_t height, VkFormat format, uint32_t mipLevels)
	{
		if (!create2D(width, height, format, mipLevels, true))
		{
			return false;
		}

		VkDeviceSize imageSize = width * height * 4;

		// 1. 스테이징 버퍼 생성
		RHIBufferCreateInfo stagingInfo{};
		stagingInfo.size = imageSize;
		stagingInfo.usage = RHI_BUFFER_USAGE_TRANSFER_SRC_BIT;
		stagingInfo.memoryProperties = RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		stagingInfo.initialData = data;

		auto* stagingBuffer = new VulkanBuffer(device_, physicalDevice_);
		if (!stagingBuffer->create(stagingInfo))
		{
			delete stagingBuffer;
			return false;
		}

		// 2. 이미지 레이아웃 전환: UNDEFINED -> TRANSFER_DST
		transitionImageLayout(image_->getVkImage(), format,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			mipLevels_);

		// 3. 버퍼 -> 이미지 복사
		copyBufferToImage(stagingBuffer->getVkBuffer(), image_->getVkImage(), width, height);

		// 4. Mipmap 생성 (레이아웃도 자동 전환)
		if (mipLevels_ > 1)
		{
			generateMipmaps(image_->getVkImage(), format, width, height, mipLevels_);
		}
		else
		{
			// Mipmap 없으면 수동 전환: TRANSFER_DST -> SHADER_READ_ONLY
			transitionImageLayout(image_->getVkImage(), format,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				mipLevels_);
		}

		// 스테이징 버퍼 정리
		delete stagingBuffer;

		return true;
	}

	void VulkanTexture::destroy()
	{
		if (sampler_)
		{
			delete sampler_;
			sampler_ = nullptr;
		}

		if (imageView_)
		{
			delete imageView_;
			imageView_ = nullptr;
		}

		if (image_)
		{
			delete image_;
			image_ = nullptr;
		}
	}

	void VulkanTexture::generateMipmaps(VkImage image, VkFormat format, uint32_t width, uint32_t height, uint32_t mipLevels)
	{
		// 포맷이 선형 필터링을 지원하는지 확인
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice_, format, &formatProperties);

		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		{
			printLog("Texture image format does not support linear blitting!");
			return;
		}

		// VulkanRHI의 단일 시간 커맨드 사용
		VkCommandBuffer commandBuffer = rhi_ ? rhi_->beginSingleTimeCommands() : VK_NULL_HANDLE;
		if (!commandBuffer)
		{
			printLog("Failed to begin single time commands - RHI not available!");
			return;
		}

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = width;
		int32_t mipHeight = height;

		for (uint32_t i = 1; i < mipLevels; i++)
		{
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(commandBuffer,
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (rhi_)
		{
			rhi_->endSingleTimeCommands(commandBuffer);
		}
	}

	void VulkanTexture::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
	{
		VkCommandBuffer commandBuffer = rhi_ ? rhi_->beginSingleTimeCommands() : VK_NULL_HANDLE;
		if (!commandBuffer)
		{
			printLog("Failed to begin single time commands - RHI not available!");
			return;
		}

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
		{
			exitWithMessage("Unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		if (rhi_)
		{
			rhi_->endSingleTimeCommands(commandBuffer);
		}
	}

	void VulkanTexture::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = rhi_ ? rhi_->beginSingleTimeCommands() : VK_NULL_HANDLE;
		if (!commandBuffer)
		{
			printLog("Failed to begin single time commands - RHI not available!");
			return;
		}

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		if (rhi_)
		{
			rhi_->endSingleTimeCommands(commandBuffer);
		}
	}

} // namespace BinRenderer::Vulkan
