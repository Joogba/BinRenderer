#include "VulkanSwapchain.h"
#include "VulkanContext.h"
#include "../Resources/VulkanImage.h"  //  VulkanImageView 포함
#include "../../../Core/Logger.h"
#include <algorithm>
#include <limits>

namespace BinRenderer::Vulkan
{
	VulkanSwapchain::VulkanSwapchain(VulkanContext* context)
		: context_(context)
	{
	}

	VulkanSwapchain::~VulkanSwapchain()
	{
		destroy();
	}

	bool VulkanSwapchain::create(VkSurfaceKHR surface, uint32_t width, uint32_t height, bool vsync)
	{
		surface_ = surface;
		return createSwapchain(width, height, vsync);
	}

	void VulkanSwapchain::destroy()
	{
		destroyImageViews();

		if (swapchain_ != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(context_->getDevice(), swapchain_, nullptr);
			swapchain_ = VK_NULL_HANDLE;
		}
	}

	bool VulkanSwapchain::recreate(uint32_t width, uint32_t height, bool vsync)
	{
		// 기존 스왑체인 파괴
		destroy();

		// 새로운 스왑체인 생성
		return createSwapchain(width, height, vsync);
	}

	bool VulkanSwapchain::recreate(uint32_t width, uint32_t height)
	{
		// 기본값으로 vsync를 false로 설정하거나, 현재 presentMode_ 유지
		bool vsync = (presentMode_ == VK_PRESENT_MODE_FIFO_KHR);
		return recreate(width, height, vsync);
	}

	VkResult VulkanSwapchain::acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t& imageIndex)
	{
		//  Fence 사용 (semaphore가 NULL일 경우)
		VkFence fence = VK_NULL_HANDLE;
		if (presentCompleteSemaphore == VK_NULL_HANDLE)
		{
			// Fence 생성 (임시)
			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			vkCreateFence(context_->getDevice(), &fenceInfo, nullptr, &fence);
		}

		VkResult result = vkAcquireNextImageKHR(
			context_->getDevice(),
			swapchain_,
			UINT64_MAX,
			presentCompleteSemaphore,
			fence,
			&imageIndex
		);

		// Fence 대기 및 정리
		if (fence != VK_NULL_HANDLE)
		{
			vkWaitForFences(context_->getDevice(), 1, &fence, VK_TRUE, UINT64_MAX);
			vkDestroyFence(context_->getDevice(), fence, nullptr);
		}

		return result;
	}

	VkResult VulkanSwapchain::present(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore)
	{
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = waitSemaphore != VK_NULL_HANDLE ? 1 : 0;
		presentInfo.pWaitSemaphores = &waitSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain_;
		presentInfo.pImageIndices = &imageIndex;

		return vkQueuePresentKHR(queue, &presentInfo);
	}

	VulkanSwapchain::SwapchainSupportDetails VulkanSwapchain::querySwapchainSupport(
		VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		SwapchainSupportDetails details;

		// Capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		// Formats
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		// Present modes
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	VkSurfaceFormatKHR VulkanSwapchain::chooseSwapSurfaceFormat(
		const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		// 선호하는 포맷 순서
		std::vector<VkFormat> preferredFormats = {
			VK_FORMAT_B8G8R8A8_SRGB,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_FORMAT_B8G8R8A8_UNORM,
			VK_FORMAT_R8G8B8A8_UNORM
		};

		for (const auto& preferredFormat : preferredFormats)
		{
			for (const auto& availableFormat : availableFormats)
			{
				if (availableFormat.format == preferredFormat &&
					availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				{
					return availableFormat;
				}
			}
		}

		// 기본값
		return availableFormats[0];
	}

	VkPresentModeKHR VulkanSwapchain::chooseSwapPresentMode(
		const std::vector<VkPresentModeKHR>& availablePresentModes, bool vsync)
	{
		if (vsync)
		{
			// VSync ON: FIFO 또는 MAILBOX
			for (const auto& availableMode : availablePresentModes)
			{
				if (availableMode == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					return availableMode; // Triple buffering
				}
			}
			return VK_PRESENT_MODE_FIFO_KHR; // VSync 보장
		}
		else
		{
			// VSync OFF: IMMEDIATE 우선
			for (const auto& availableMode : availablePresentModes)
			{
				if (availableMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
				{
					return availableMode;
				}
			}
			// MAILBOX도 괜찮음
			for (const auto& availableMode : availablePresentModes)
			{
				if (availableMode == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					return availableMode;
				}
			}
			return VK_PRESENT_MODE_FIFO_KHR; // Fallback
		}
	}

	VkExtent2D VulkanSwapchain::chooseSwapExtent(
		const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}
		else
		{
			VkExtent2D actualExtent = { width, height };
			actualExtent.width = std::clamp(actualExtent.width,
				capabilities.minImageExtent.width,
				capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height,
				capabilities.minImageExtent.height,
				capabilities.maxImageExtent.height);
			return actualExtent;
		}
	}

	bool VulkanSwapchain::createSwapchain(uint32_t width, uint32_t height, bool vsync)
	{
		SwapchainSupportDetails swapchainSupport = querySwapchainSupport(
			context_->getPhysicalDevice(), surface_);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupport.presentModes, vsync);
		VkExtent2D extent = chooseSwapExtent(swapchainSupport.capabilities, width, height);

		// 이미지 개수 결정
		uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
		if (swapchainSupport.capabilities.maxImageCount > 0 &&
			imageCount > swapchainSupport.capabilities.maxImageCount)
		{
			imageCount = swapchainSupport.capabilities.maxImageCount;
		}

		// 스왑체인 생성 정보
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface_;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		// 큐 패밀리 인덱스
		uint32_t queueFamilyIndices[] = {
			context_->getGraphicsQueueFamily(),
			context_->getPresentQueueFamily()
		};

		if (context_->getGraphicsQueueFamily() != context_->getPresentQueueFamily())
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(context_->getDevice(), &createInfo, nullptr, &swapchain_) != VK_SUCCESS)
		{
			printLog("ERROR: Failed to create swapchain");
			return false;
		}

		// 스왑체인 이미지 가져오기
		vkGetSwapchainImagesKHR(context_->getDevice(), swapchain_, &imageCount, nullptr);
		images_.resize(imageCount);
		vkGetSwapchainImagesKHR(context_->getDevice(), swapchain_, &imageCount, images_.data());

		// 설정 저장
		colorFormat_ = surfaceFormat.format;
		colorSpace_ = surfaceFormat.colorSpace;
		extent_ = extent;
		presentMode_ = presentMode;

		// 이미지 뷰 생성
		if (!createImageViews())
		{
			printLog("ERROR: Failed to create swapchain image views");
			return false;
		}

		printLog("Swapchain created: {}x{}, Format: {}, Images: {}",
			extent.width, extent.height, static_cast<int>(colorFormat_), imageCount);

		return true;
	}

	bool VulkanSwapchain::createImageViews()
	{
		imageViews_.resize(images_.size());

		for (size_t i = 0; i < images_.size(); i++)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = images_[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = colorFormat_;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(context_->getDevice(), &createInfo, nullptr, &imageViews_[i]) != VK_SUCCESS)
			{
				printLog("ERROR: Failed to create image view {}", i);
				return false;
			}
		}

		//  RHIImageView 래퍼 생성
		createImageViewWrappers();

		return true;
	}

	void VulkanSwapchain::destroyImageViews()
	{
		//  래퍼 먼저 삭제
		destroyImageViewWrappers();

		for (auto imageView : imageViews_)
		{
			vkDestroyImageView(context_->getDevice(), imageView, nullptr);
		}
		imageViews_.clear();
	}

	void VulkanSwapchain::createImageViewWrappers()
	{
		imageViewWrappers_.clear();
		imageViewWrappers_.reserve(imageViews_.size());
		imageViewHandles_.resize(imageViews_.size()); //  핸들 벡터 리사이즈

		for (size_t i = 0; i < imageViews_.size(); i++)
		{
			//  VulkanImageView 래퍼 제대로 생성
			auto wrapper = std::make_unique<VulkanImageView>(context_->getDevice(), nullptr);
			
			//  기존 VkImageView 설정
			wrapper->setVkImageView(imageViews_[i]);
			
			//  소유권 없음 (VulkanSwapchain이 destroyImageViews()에서 파괴)
			wrapper->setOwnsImageView(false);
			
			//  Swapchain format 설정
			wrapper->setFormat(static_cast<RHIFormat>(colorFormat_));
			
			imageViewWrappers_.push_back(std::move(wrapper));
		}

		printLog(" Created {} RHIImageView wrappers", imageViewWrappers_.size());
	}

	void VulkanSwapchain::destroyImageViewWrappers()
	{
		//  unique_ptr이 자동으로 VulkanImageView 삭제
		// VulkanImageView::destroy()는 ownsImageView_가 false이므로 VkImageView를 파괴하지 않음
		imageViewWrappers_.clear();
		imageViewHandles_.clear(); //  핸들 벡터 클리어
	}

	// ========================================
	// RHISwapchain 인터페이스 구현
	// ========================================

	bool VulkanSwapchain::acquireNextImage(uint32_t& imageIndex, RHISemaphore* semaphore, RHIFence* fence)
	{
		// TODO: RHISemaphore, RHIFence 래퍼 구현 필요
		// 현재는 내부 세마포어 사용
		VkResult result = vkAcquireNextImageKHR(
			context_->getDevice(),
			swapchain_,
			UINT64_MAX,
			VK_NULL_HANDLE,  // TODO: semaphore
			VK_NULL_HANDLE,  // TODO: fence
			&imageIndex
		);

		return (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);
	}

	bool VulkanSwapchain::present(uint32_t imageIndex, RHISemaphore* waitSemaphore)
	{
		// TODO: RHISemaphore 래퍼 구현 필요
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 0;  // TODO: waitSemaphore
		presentInfo.pWaitSemaphores = nullptr;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain_;
		presentInfo.pImageIndices = &imageIndex;

		VkResult result = vkQueuePresentKHR(context_->getPresentQueue(), &presentInfo);
		return (result == VK_SUCCESS);
	}

	RHIFormat VulkanSwapchain::getFormat() const
	{
		// Vulkan VkFormat을 RHIFormat으로 변환
		switch (colorFormat_)
		{
		case VK_FORMAT_B8G8R8A8_UNORM:
			return RHI_FORMAT_B8G8R8A8_UNORM;
		case VK_FORMAT_R8G8B8A8_UNORM:
			return RHI_FORMAT_R8G8B8A8_UNORM;
		case VK_FORMAT_B8G8R8A8_SRGB:
			return RHI_FORMAT_B8G8R8A8_SRGB;
		case VK_FORMAT_R8G8B8A8_SRGB:
			return RHI_FORMAT_R8G8B8A8_SRGB;
		default:
			return RHI_FORMAT_UNDEFINED;
		}
	}

	RHIPresentMode VulkanSwapchain::getPresentMode() const
	{
		// Vulkan VkPresentModeKHR을 RHIPresentMode로 변환
		switch (presentMode_)
		{
		case VK_PRESENT_MODE_IMMEDIATE_KHR:
			return RHI_PRESENT_MODE_IMMEDIATE_KHR;
		case VK_PRESENT_MODE_MAILBOX_KHR:
			return RHI_PRESENT_MODE_MAILBOX_KHR;
		case VK_PRESENT_MODE_FIFO_KHR:
			return RHI_PRESENT_MODE_FIFO_KHR;
		case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
			return RHI_PRESENT_MODE_FIFO_RELAXED_KHR;
		default:
			return RHI_PRESENT_MODE_FIFO_KHR;
		}
	}

	void VulkanSwapchain::setPresentMode(RHIPresentMode mode)
	{
		// RHIPresentMode를 Vulkan VkPresentModeKHR로 변환
		switch (mode)
		{
		case RHI_PRESENT_MODE_IMMEDIATE_KHR:
			presentMode_ = VK_PRESENT_MODE_IMMEDIATE_KHR;
			break;
		case RHI_PRESENT_MODE_MAILBOX_KHR:
			presentMode_ = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		case RHI_PRESENT_MODE_FIFO_KHR:
			presentMode_ = VK_PRESENT_MODE_FIFO_KHR;
			break;
		case RHI_PRESENT_MODE_FIFO_RELAXED_KHR:
			presentMode_ = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
			break;
		default:
			presentMode_ = VK_PRESENT_MODE_FIFO_KHR;
			break;
		}

		// TODO: 스왑체인 재생성 필요
	}

} // namespace BinRenderer::Vulkan
