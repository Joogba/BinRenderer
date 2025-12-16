#pragma once

#include "../../Resources/RHITexture.h"
#include "VulkanImage.h"
#include "VulkanSampler.h"

namespace BinRenderer::Vulkan
{
	class VulkanRHI;

	/**
	 * @brief Vulkan 텍스처 구현 (Image + ImageView + Sampler)
   */
	class VulkanTexture : public RHITexture
	{
	public:
		VulkanTexture(VkDevice device, VkPhysicalDevice physicalDevice, VulkanRHI* rhi = nullptr);
		~VulkanTexture() override;

		// 2D 텍스처 생성
		bool create2D(uint32_t width, uint32_t height, VkFormat format,
			uint32_t mipLevels = 1, bool createSampler = true);

		// 파일에서 로드
		bool loadFromFile(const char* filename, bool generateMipmaps = true);

		// 데이터로 생성
		bool createFromData(const void* data, uint32_t width, uint32_t height,
			VkFormat format, uint32_t mipLevels = 1);

		void destroy();

		// RHITexture 인터페이스 구현
		RHIImage* getImage() const override { return image_; }
		RHIImageView* getImageView() const override { return imageView_; }
		RHISampler* getSampler() const override { return sampler_; }

		uint32_t getWidth() const override { return width_; }
		uint32_t getHeight() const override { return height_; }
		uint32_t getMipLevels() const override { return mipLevels_; }

		// Vulkan 네이티브 접근
		VulkanImage* getVulkanImage() const { return image_; }
		VulkanImageView* getVulkanImageView() const { return imageView_; }
		VulkanSampler* getVulkanSampler() const { return sampler_; }

	private:
		VkDevice device_;
		VkPhysicalDevice physicalDevice_;
		VulkanRHI* rhi_;

		VulkanImage* image_ = nullptr;
		VulkanImageView* imageView_ = nullptr;
		VulkanSampler* sampler_ = nullptr;

		uint32_t width_ = 0;
		uint32_t height_ = 0;
		uint32_t mipLevels_ = 1;

		void generateMipmaps(VkImage image, VkFormat format, uint32_t width, uint32_t height, uint32_t mipLevels);
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	};

} // namespace BinRenderer::Vulkan
