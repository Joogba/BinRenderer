#pragma once

#include "../../Resources/RHIImage.h"
#include "../../Structs/RHIImageCreateInfo.h"
#include <vulkan/vulkan.h>

namespace BinRenderer::Vulkan
{
	/**
	 * @brief Vulkan 이미지 구현
  */
	class VulkanImage : public RHIImage
	{
	public:
		VulkanImage(VkDevice device, VkPhysicalDevice physicalDevice);
		~VulkanImage() override;

		bool create(const RHIImageCreateInfo& createInfo);
		void destroy();

		// RHIImage 인터페이스 구현
		uint32_t getWidth() const override { return width_; }
		uint32_t getHeight() const override { return height_; }
		uint32_t getDepth() const override { return depth_; }
		uint32_t getMipLevels() const override { return mipLevels_; }
		uint32_t getArrayLayers() const override { return arrayLayers_; }
		RHIFormat getFormat() const override { return format_; }
		RHISampleCountFlagBits getSamples() const override { return samples_; }

		// Vulkan 네이티브 접근
		VkImage getVkImage() const { return image_; }
		VkDeviceMemory getVkMemory() const { return memory_; }

	private:
		VkDevice device_;
		VkPhysicalDevice physicalDevice_;
		VkImage image_ = VK_NULL_HANDLE;
		VkDeviceMemory memory_ = VK_NULL_HANDLE;

		uint32_t width_ = 0;
		uint32_t height_ = 0;
		uint32_t depth_ = 1;
		uint32_t mipLevels_ = 1;
		uint32_t arrayLayers_ = 1;
		RHIFormat format_ = RHI_FORMAT_UNDEFINED;
		RHISampleCountFlagBits samples_ = RHI_SAMPLE_COUNT_1_BIT;

		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	};

	/**
	 * @brief Vulkan 이미지 뷰 구현
	 */
	class VulkanImageView : public RHIImageView
	{
	public:
		VulkanImageView(VkDevice device, VulkanImage* image);
		~VulkanImageView() override;

		bool create(VkImageViewType viewType, VkImageAspectFlags aspectFlags);
		void destroy();

		// RHIImageView 인터페이스 구현
		RHIImage* getImage() const override { return image_; }
		RHIImageViewType getViewType() const override { return viewType_; }
		RHIFormat getFormat() const override;

		// Vulkan 네이티브 접근
		VkImageView getVkImageView() const { return imageView_; }

	private:
		VkDevice device_;
		VulkanImage* image_;
		VkImageView imageView_ = VK_NULL_HANDLE;
		RHIImageViewType viewType_;
	};

} // namespace BinRenderer::Vulkan
