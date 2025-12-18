#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace BinRenderer::Vulkan
{
	// Forward declaration
	class VulkanContext;

	/**
	 * @brief Vulkan 스왑체인 래퍼
	 * 
	 * Vulkan/Swapchain.h/cpp의 기능을 RHI 구조에 맞게 재구현
	 */
	class VulkanSwapchain
	{
	public:
		VulkanSwapchain(VulkanContext* context);
		~VulkanSwapchain();

		// 스왑체인 생성/재생성
		bool create(VkSurfaceKHR surface, uint32_t width, uint32_t height, bool vsync = false);
		void destroy();
		bool recreate(uint32_t width, uint32_t height, bool vsync = false);

		// 이미지 획득 및 Present
		VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t& imageIndex);
		VkResult present(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);

		// 접근자
		VkSwapchainKHR getHandle() const { return swapchain_; }
		VkFormat getColorFormat() const { return colorFormat_; }
		VkColorSpaceKHR getColorSpace() const { return colorSpace_; }
		VkExtent2D getExtent() const { return extent_; }
		uint32_t getImageCount() const { return static_cast<uint32_t>(images_.size()); }

		const std::vector<VkImage>& getImages() const { return images_; }
		VkImage getImage(uint32_t index) const { return images_[index]; }

		const std::vector<VkImageView>& getImageViews() const { return imageViews_; }
		VkImageView getImageView(uint32_t index) const { return imageViews_[index]; }

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
	};

} // namespace BinRenderer::Vulkan
