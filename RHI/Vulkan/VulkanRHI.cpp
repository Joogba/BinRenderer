#include "VulkanRHI.h"
#include "Resources/VulkanBuffer.h"
#include "Resources/VulkanImage.h"
#include "Resources/VulkanShader.h"
#include "Pipeline/VulkanPipeline.h"
#include "Vulkan/Logger.h"

#include <GLFW/glfw3.h>
#include <cassert>
#include <algorithm>

namespace BinRenderer::Vulkan
{
	VulkanRHI::~VulkanRHI()
	{
		shutdown();
	}

	bool VulkanRHI::initialize(const RHIInitInfo& initInfo)
	{
		initInfo_ = initInfo;
		maxFramesInFlight_ = initInfo.maxFramesInFlight;

		try {
			// VulkanContext 초기화
			context_ = std::make_unique<VulkanContext>();
			if (!context_->initialize(initInfo.requiredInstanceExtensions, initInfo.enableValidationLayer))
			{
				printLog("Failed to initialize Vulkan context");
				return false;
			}

			createSurface();
			createSwapchain();
			createSwapchainImageViews();

			// 커맨드 풀 생성
			commandPool_ = std::make_unique<VulkanCommandPool>(context_->getDevice());
			if (!commandPool_->create(context_->getGraphicsQueueFamily(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT))
			{
				printLog("Failed to create command pool");
				return false;
			}

			// 커맨드 버퍼 할당
			commandBuffers_ = commandPool_->allocateCommandBuffers(maxFramesInFlight_);

			createSyncObjects();

			printLog("VulkanRHI initialized successfully");
			return true;
		}
		catch (const std::exception& e) {
			printLog("VulkanRHI initialization failed: {}", e.what());
			return false;
		}
	}

	void VulkanRHI::shutdown()
	{
		if (context_)
		{
			context_->waitIdle();
		}

		// 동기화 객체 정리
		VkDevice device = context_ ? context_->getDevice() : VK_NULL_HANDLE;
		if (device != VK_NULL_HANDLE)
		{
			for (size_t i = 0; i < maxFramesInFlight_; i++)
			{
				if (imageAvailableSemaphores_[i] != VK_NULL_HANDLE)
				{
					vkDestroySemaphore(device, imageAvailableSemaphores_[i], nullptr);
				}
				if (renderFinishedSemaphores_[i] != VK_NULL_HANDLE)
				{
					vkDestroySemaphore(device, renderFinishedSemaphores_[i], nullptr);
				}
				if (inFlightFences_[i] != VK_NULL_HANDLE)
				{
					vkDestroyFence(device, inFlightFences_[i], nullptr);
				}
			}

			// 전송 커맨드 풀 정리
			if (transferCommandPool_ != VK_NULL_HANDLE)
			{
				vkDestroyCommandPool(device, transferCommandPool_, nullptr);
				transferCommandPool_ = VK_NULL_HANDLE;
			}
		}

		// 커맨드 버퍼 및 풀 정리
		commandBuffers_.clear();
		commandPool_.reset();

		// 스왑체인 정리
		destroySwapchain();

		// Surface 정리
		if (surface_ != VK_NULL_HANDLE && context_)
		{
			vkDestroySurfaceKHR(context_->getInstance(), surface_, nullptr);
			surface_ = VK_NULL_HANDLE;
		}

		// Context 정리
		context_.reset();
	}

	void VulkanRHI::waitIdle()
	{
		if (context_)
		{
			context_->waitIdle();
		}
	}

	bool VulkanRHI::beginFrame(uint32_t& imageIndex)
	{
		currentFrameIndex_ = (currentFrameIndex_ + 1) % maxFramesInFlight_;

		VkDevice device = context_->getDevice();
		vkWaitForFences(device, 1, &inFlightFences_[currentFrameIndex_], VK_TRUE, UINT64_MAX);

		VkResult result = vkAcquireNextImageKHR(device, swapchain_, UINT64_MAX,
			imageAvailableSemaphores_[currentFrameIndex_], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			// 스왑체인 재생성 필요
			return false;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			exitWithMessage("Failed to acquire swap chain image!");
		}

		currentImageIndex_ = imageIndex;
		vkResetFences(device, 1, &inFlightFences_[currentFrameIndex_]);
		return true;
	}

	void VulkanRHI::endFrame(uint32_t imageIndex)
	{
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderFinishedSemaphores_[currentFrameIndex_];
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain_;
		presentInfo.pImageIndices = &imageIndex;

		VkResult result = vkQueuePresentKHR(context_->getPresentQueue(), &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			// 스왑체인 재생성 필요
		}
		else if (result != VK_SUCCESS)
		{
			exitWithMessage("Failed to present swap chain image!");
		}
	}

	uint32_t VulkanRHI::getCurrentFrameIndex() const
	{
		return currentFrameIndex_;
	}

	RHIBuffer* VulkanRHI::createBuffer(const RHIBufferCreateInfo& createInfo)
	{
		auto* vulkanBuffer = new VulkanBuffer(context_->getDevice(), context_->getPhysicalDevice());
		if (!vulkanBuffer->create(createInfo))
		{
			delete vulkanBuffer;
			return nullptr;
		}
		return vulkanBuffer;
	}

	RHIImage* VulkanRHI::createImage(const RHIImageCreateInfo& createInfo)
	{
		auto* vulkanImage = new VulkanImage(context_->getDevice(), context_->getPhysicalDevice());
		if (!vulkanImage->create(createInfo))
		{
			delete vulkanImage;
			return nullptr;
		}
		return vulkanImage;
	}

	RHIShader* VulkanRHI::createShader(const RHIShaderCreateInfo& createInfo)
	{
		auto* vulkanShader = new VulkanShader(context_->getDevice());
		if (!vulkanShader->create(createInfo))
		{
			delete vulkanShader;
			return nullptr;
		}
		return vulkanShader;
	}

	RHIPipeline* VulkanRHI::createPipeline(const RHIPipelineCreateInfo& createInfo)
	{
		auto* vulkanPipeline = new VulkanPipeline(context_->getDevice());
		if (!vulkanPipeline->create(createInfo))
		{
			delete vulkanPipeline;
			return nullptr;
		}
		return vulkanPipeline;
	}

	void VulkanRHI::destroyBuffer(RHIBuffer* buffer) { delete buffer; }
	void VulkanRHI::destroyImage(RHIImage* image) { delete image; }
	void VulkanRHI::destroyShader(RHIShader* shader) { delete shader; }
	void VulkanRHI::destroyPipeline(RHIPipeline* pipeline) { delete pipeline; }

	void VulkanRHI::beginCommandRecording()
	{
		VulkanCommandBuffer* cmdBuffer = commandBuffers_[currentFrameIndex_];
		cmdBuffer->reset();
		cmdBuffer->begin();
	}

	void VulkanRHI::endCommandRecording()
	{
		VulkanCommandBuffer* cmdBuffer = commandBuffers_[currentFrameIndex_];
		cmdBuffer->end();
	}

	void VulkanRHI::submitCommands()
	{
		VulkanCommandBuffer* cmdBuffer = commandBuffers_[currentFrameIndex_];

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores_[currentFrameIndex_] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		VkCommandBuffer vkCmdBuffer = cmdBuffer->getVkCommandBuffer();
		submitInfo.pCommandBuffers = &vkCmdBuffer;
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores_[currentFrameIndex_] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkQueueSubmit(context_->getGraphicsQueue(), 1, &submitInfo, inFlightFences_[currentFrameIndex_]);
	}

	void VulkanRHI::cmdBindPipeline(RHIPipeline* pipeline)
	{
		VulkanCommandBuffer* cmdBuffer = commandBuffers_[currentFrameIndex_];
		cmdBuffer->bindPipeline(pipeline);
	}

	void VulkanRHI::cmdBindVertexBuffer(RHIBuffer* buffer, RHIDeviceSize offset)
	{
		VulkanCommandBuffer* cmdBuffer = commandBuffers_[currentFrameIndex_];
		cmdBuffer->bindVertexBuffer(0, buffer, offset);
	}

	void VulkanRHI::cmdBindIndexBuffer(RHIBuffer* buffer, RHIDeviceSize offset)
	{
		VulkanCommandBuffer* cmdBuffer = commandBuffers_[currentFrameIndex_];
		cmdBuffer->bindIndexBuffer(buffer, offset);
	}

	void VulkanRHI::cmdBindDescriptorSets(RHIPipelineLayout* layout, RHIDescriptorSet** sets, uint32_t setCount)
	{
		VulkanCommandBuffer* cmdBuffer = commandBuffers_[currentFrameIndex_];
		cmdBuffer->bindDescriptorSets(layout, 0, setCount, sets);
	}

	void VulkanRHI::cmdSetViewport(const RHIViewport& viewport)
	{
		VkCommandBuffer cmdBuffer = commandBuffers_[currentFrameIndex_]->getVkCommandBuffer();
		VkViewport vkViewport{};
		vkViewport.x = viewport.x;
		vkViewport.y = viewport.y;
		vkViewport.width = viewport.width;
		vkViewport.height = viewport.height;
		vkViewport.minDepth = viewport.minDepth;
		vkViewport.maxDepth = viewport.maxDepth;
		vkCmdSetViewport(cmdBuffer, 0, 1, &vkViewport);
	}

	void VulkanRHI::cmdSetScissor(const RHIRect2D& scissor)
	{
		VkCommandBuffer cmdBuffer = commandBuffers_[currentFrameIndex_]->getVkCommandBuffer();
		VkRect2D vkScissor{};
		vkScissor.offset = { scissor.offset.x, scissor.offset.y };
		vkScissor.extent = { scissor.extent.width, scissor.extent.height };
		vkCmdSetScissor(cmdBuffer, 0, 1, &vkScissor);
	}

	void VulkanRHI::cmdDraw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
	{
		VulkanCommandBuffer* cmdBuffer = commandBuffers_[currentFrameIndex_];
		cmdBuffer->draw(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void VulkanRHI::cmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
	{
		VulkanCommandBuffer* cmdBuffer = commandBuffers_[currentFrameIndex_];
		cmdBuffer->drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void VulkanRHI::createSurface()
	{
		if (!initInfo_.window)
		{
			exitWithMessage("Window is null!");
		}

		if (glfwCreateWindowSurface(context_->getInstance(), static_cast<GLFWwindow*>(initInfo_.window), nullptr, &surface_) != VK_SUCCESS)
		{
			exitWithMessage("Failed to create window surface!");
		}
	}

	void VulkanRHI::createSwapchain()
	{
		VkDevice device = context_->getDevice();
		VkPhysicalDevice physicalDevice = context_->getPhysicalDevice();

		// Surface capabilities
		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface_, &capabilities);

		// Surface format
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface_, &formatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> formats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface_, &formatCount, formats.data());

		VkSurfaceFormatKHR surfaceFormat = formats[0];
		for (const auto& format : formats)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				surfaceFormat = format;
				break;
			}
		}

		// Present mode
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface_, &presentModeCount, nullptr);
		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface_, &presentModeCount, presentModes.data());

		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
		for (const auto& mode : presentModes)
		{
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				presentMode = mode;
				break;
			}
		}

		// Extent
		swapchainExtent_ = capabilities.currentExtent;
		if (swapchainExtent_.width == UINT32_MAX)
		{
			swapchainExtent_.width = std::clamp(initInfo_.windowWidth, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			swapchainExtent_.height = std::clamp(initInfo_.windowHeight, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		}

		// Image count
		uint32_t imageCount = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
		{
			imageCount = capabilities.maxImageCount;
		}

		// Create swapchain
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface_;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = swapchainExtent_;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.preTransform = capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain_) != VK_SUCCESS)
		{
			exitWithMessage("Failed to create swap chain!");
		}

		swapchainImageFormat_ = surfaceFormat.format;

		// Get swapchain images
		vkGetSwapchainImagesKHR(device, swapchain_, &imageCount, nullptr);
		swapchainImages_.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapchain_, &imageCount, swapchainImages_.data());
	}

	void VulkanRHI::createSwapchainImageViews()
	{
		VkDevice device = context_->getDevice();
		swapchainImageViews_.resize(swapchainImages_.size());

		for (size_t i = 0; i < swapchainImages_.size(); i++)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapchainImages_[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapchainImageFormat_;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device, &createInfo, nullptr, &swapchainImageViews_[i]) != VK_SUCCESS)
			{
				exitWithMessage("Failed to create image views!");
			}
		}
	}

	void VulkanRHI::destroySwapchain()
	{
		VkDevice device = context_ ? context_->getDevice() : VK_NULL_HANDLE;
		if (device == VK_NULL_HANDLE)
		{
			return;
		}

		for (auto imageView : swapchainImageViews_)
		{
			if (imageView != VK_NULL_HANDLE)
			{
				vkDestroyImageView(device, imageView, nullptr);
			}
		}
		swapchainImageViews_.clear();

		if (swapchain_ != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(device, swapchain_, nullptr);
			swapchain_ = VK_NULL_HANDLE;
		}
	}

	void VulkanRHI::createSyncObjects()
	{
		imageAvailableSemaphores_.resize(maxFramesInFlight_);
		renderFinishedSemaphores_.resize(maxFramesInFlight_);
		inFlightFences_.resize(maxFramesInFlight_);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VkDevice device = context_->getDevice();

		for (size_t i = 0; i < maxFramesInFlight_; i++)
		{
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores_[i]);
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores_[i]);
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences_[i]);
		}

		// 전송 커맨드 풀 생성
		createTransferCommandPool();
	}

	void VulkanRHI::createTransferCommandPool()
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = context_->getGraphicsQueueFamily();
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

		if (vkCreateCommandPool(context_->getDevice(), &poolInfo, nullptr, &transferCommandPool_) != VK_SUCCESS)
		{
			exitWithMessage("Failed to create transfer command pool!");
		}
	}

	VkCommandBuffer VulkanRHI::beginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = transferCommandPool_;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(context_->getDevice(), &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void VulkanRHI::endSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(context_->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(context_->getGraphicsQueue());

		vkFreeCommandBuffers(context_->getDevice(), transferCommandPool_, 1, &commandBuffer);
	}

} // namespace BinRenderer::Vulkan
