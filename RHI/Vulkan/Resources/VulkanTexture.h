#pragma once

#include "../../Resources/RHITexture.h"
#include "../../Resources/RHITextureLoader.h"
#include "VulkanImage.h"
#include "VulkanSampler.h"

namespace BinRenderer::Vulkan
{
	class VulkanRHI;

	/**
	 * @brief Vulkan 텍스처 구현 (Image + ImageView + Sampler)
	 * 
	 *  KTX2 큐브맵 지원
	 *  PNG/JPEG 2D 텍스처 지원
	 */
	class VulkanTexture : public RHITexture
	{
	public:
		// 기존 생성자 (호환성 유지용)
		VulkanTexture(VkDevice device, VkPhysicalDevice physicalDevice, VulkanRHI* rhi = nullptr);
		
		//  Handle 기반 생성자
		VulkanTexture(RHIImageHandle image, RHIImageViewHandle view, RHISamplerHandle sampler, uint32_t width, uint32_t height, uint32_t mipLevels);

		~VulkanTexture() override;

		// RHITexture 인터페이스 구현
		RHIImageHandle getImage() const override { return imageHandle_; }
		RHIImageViewHandle getImageView() const override { return viewHandle_; }
		RHISamplerHandle getSampler() const override { return samplerHandle_; }

		uint32_t getWidth() const override { return width_; }
		uint32_t getHeight() const override { return height_; }
		uint32_t getMipLevels() const override { return mipLevels_; }

		// ========================================
		//  새로운 로딩 메서드 (RHITextureLoader 사용)
		// ========================================
		
		/**
		 * @brief KTX2 파일에서 텍스처 로드 (큐브맵 지원)
		 */
		bool loadFromKTX2(const std::string& filename);

		/**
		 * @brief PNG/JPEG 파일에서 2D 텍스처 로드
		 */
		bool loadFromImage(const std::string& filename, bool sRGB = false);

		/**
		 * @brief 로드된 데이터로 텍스처 생성 (내부 사용)
		 */
		bool createFromLoadedData(const RHITextureLoader::LoadedTextureData& loadedData);

		// ========================================
		// 기존 메서드 (호환성 유지)
		// ========================================
		
		// 2D 텍스처 생성
		bool create2D(uint32_t width, uint32_t height, VkFormat format,
			uint32_t mipLevels = 1, bool createSampler = true);

		// 파일에서 로드 (레거시)
		bool loadFromFile(const char* filename, bool generateMipmaps = true);

		// 데이터로 생성 (레거시)
		bool createFromData(const void* data, uint32_t width, uint32_t height,
			VkFormat format, uint32_t mipLevels = 1);

		void destroy();

		// RHITexture 인터페이스 구현 (위에서 이미 구현함, 여기서는 제거)
		// RHIImage* getImage() const override { return image_; }
		// RHIImageView* getImageView() const override { return imageView_; }
		// RHISampler* getSampler() const override { return sampler_; }

		// uint32_t getWidth() const override { return width_; }
		// uint32_t getHeight() const override { return height_; }
		// uint32_t getMipLevels() const override { return mipLevels_; }

		// Vulkan 네이티브 접근
		VulkanImage* getVulkanImage() const { return image_; }
		VulkanImageView* getVulkanImageView() const { return imageView_; }
		VulkanSampler* getVulkanSampler() const { return sampler_; }

	private:
		VkDevice device_;
		VkPhysicalDevice physicalDevice_;
		VulkanRHI* rhi_;

		// Handle 모드
		bool useHandles_ = false;
		RHIImageHandle imageHandle_;
		RHIImageViewHandle viewHandle_;
		RHISamplerHandle samplerHandle_;

		VulkanImage* image_ = nullptr;
		VulkanImageView* imageView_ = nullptr;
		VulkanSampler* sampler_ = nullptr;

		uint32_t width_ = 0;
		uint32_t height_ = 0;
		uint32_t mipLevels_ = 1;

		//  텍스처 데이터 업로드 (큐브맵 지원)
		void uploadTextureData(const RHITextureLoader::LoadedTextureData& loadedData);

		// 레거시 메서드
		void generateMipmaps(VkImage image, VkFormat format, uint32_t width, uint32_t height, uint32_t mipLevels);
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	};

} // namespace BinRenderer::Vulkan
