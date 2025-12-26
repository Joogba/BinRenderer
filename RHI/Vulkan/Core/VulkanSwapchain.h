#pragma once

#include <vulkan/vulkan.h>
#include "../../Core/RHISwapchain.h"
#include <vector>
#include <memory>

namespace BinRenderer::Vulkan
{
	// Forward declaration
	class VulkanContext;
	class VulkanImageView;

	/**
	 * @brief Vulkan 스왑체인 래퍼
	 * 
	 * Vulkan/Swapchain.h/cpp의 기능을 RHI 구조에 맞게 재구현
	 */
	class VulkanSwapchain : public RHISwapchain
	{
	public:
		VulkanSwapchain(VulkanContext* context);
		~VulkanSwapchain() override;

		// 스왑체인 생성/재생성
		bool create(VkSurfaceKHR surface, uint32_t width, uint32_t height, bool vsync = false);
		void destroy();
		bool recreate(uint32_t width, uint32_t height) override;
		bool recreate(uint32_t width, uint32_t height, bool vsync);

		// 이미지 획득 및 Present (Vulkan 전용)
		VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t& imageIndex);
		VkResult present(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);

		//  RHISwapchain 인터페이스 구현
		bool acquireNextImage(uint32_t& imageIndex, RHISemaphore* semaphore = nullptr, RHIFence* fence = nullptr) override;
		bool present(uint32_t imageIndex, RHISemaphore* waitSemaphore = nullptr) override;
		uint32_t getImageCount() const override { return static_cast<uint32_t>(images_.size()); }
		RHIFormat getFormat() const override;
		uint32_t getWidth() const override { return extent_.width; }
		uint32_t getHeight() const override { return extent_.height; }
		RHIImageHandle getImage(uint32_t index) const override { return {}; } // TODO: VulkanImage 래퍼
		RHIImageViewHandle getImageView(uint32_t index) const override { return imageViewHandles_[index]; }
		void setImageViewHandle(uint32_t index, RHIImageViewHandle handle) { imageViewHandles_[index] = handle; }

		RHIPresentMode getPresentMode() const override;
		void setPresentMode(RHIPresentMode mode) override;

		// Vulkan 전용 접근자
		VkSwapchainKHR getHandle() const { return swapchain_; }
		VkFormat getColorFormat() const { return colorFormat_; }
		VkColorSpaceKHR getColorSpace() const { return colorSpace_; }
		VkExtent2D getExtent() const { return extent_; }

		const std::vector<VkImage>& getVkImages() const { return images_; }
		VkImage getVkImage(uint32_t index) const { return images_[index]; }

		const std::vector<VkImageView>& getVkImageViews() const { return imageViews_; }
		VkImageView getVkImageView(uint32_t index) const { return imageViews_[index]; }
		
		// 내부적으로 사용하는 Raw Pointer 접근 (Pool 등록용)
		RHIImageView* getImageViewRaw(uint32_t index) const { return imageViewWrappers_[index].get(); }

	private:
		VulkanContext* context_;
		VkSurfaceKHR surface_ = VK_NULL_HANDLE;
		VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;

		// 스왑체인 설정
		VkFormat colorFormat_ = VK_FORMAT_UNDEFINED;
		VkColorSpaceKHR colorSpace_ = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		VkExtent2D extent_ = {};
		VkPresentModeKHR presentMode_ = VK_PRESENT_MODE_FIFO_KHR;

		// 스왑체인 이미지
		std::vector<VkImage> images_;
		std::vector<VkImageView> imageViews_;
		
		//  RHIImageView 래퍼들
		std::vector<std::unique_ptr<RHIImageView>> imageViewWrappers_;
		std::vector<RHIImageViewHandle> imageViewHandles_;

		// 헬퍼 함수
		struct SwapchainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool vsync);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height);

		bool createSwapchain(uint32_t width, uint32_t height, bool vsync);
		bool createImageViews();
		void destroyImageViews();
		
		//  RHIImageView 래퍼 생성/삭제
		void createImageViewWrappers();
		void destroyImageViewWrappers();
	};

} // namespace BinRenderer::Vulkan
