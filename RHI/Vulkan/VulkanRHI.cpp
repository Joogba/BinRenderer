#include "VulkanRHI.h"
#include "Vulkan/Logger.h"

#include <GLFW/glfw3.h>
#include <cassert>

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
			context_ = std::make_unique<Context>(initInfo.requiredInstanceExtensions, true);
			createSurface();

			VkExtent2D extent = { initInfo.windowWidth, initInfo.windowHeight };
			swapchain_ = std::make_unique<Swapchain>(*context_, surface_, extent, false);

			commandBuffers_ = context_->createGraphicsCommandBuffers(maxFramesInFlight_);
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
		if (context_) {
			context_->waitIdle();
		}

		VkDevice device = context_->device();
		for (size_t i = 0; i < maxFramesInFlight_; i++) {
			if (imageAvailableSemaphores_[i] != VK_NULL_HANDLE) {
				vkDestroySemaphore(device, imageAvailableSemaphores_[i], nullptr);
			}
			if (renderFinishedSemaphores_[i] != VK_NULL_HANDLE) {
				vkDestroySemaphore(device, renderFinishedSemaphores_[i], nullptr);
			}
			if (inFlightFences_[i] != VK_NULL_HANDLE) {
				vkDestroyFence(device, inFlightFences_[i], nullptr);
			}
		}

		commandBuffers_.clear();
		swapchain_.reset();

		if (surface_ != VK_NULL_HANDLE) {
			vkDestroySurfaceKHR(context_->instance(), surface_, nullptr);
			surface_ = VK_NULL_HANDLE;
		}

		context_.reset();
	}

	void VulkanRHI::waitIdle()
	{
		if (context_) {
			context_->waitIdle();
		}
	}

	bool VulkanRHI::beginFrame(uint32_t& imageIndex)
	{
		currentFrameIndex_ = (currentFrameIndex_ + 1) % maxFramesInFlight_;

		vkWaitForFences(context_->device(), 1, &inFlightFences_[currentFrameIndex_], VK_TRUE, UINT64_MAX);

		VkResult result = swapchain_->acquireNextImage(imageAvailableSemaphores_[currentFrameIndex_], imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			return false;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			exitWithMessage("Failed to acquire swap chain image!");
		}

		vkResetFences(context_->device(), 1, &inFlightFences_[currentFrameIndex_]);
		return true;
	}

	void VulkanRHI::endFrame(uint32_t imageIndex)
	{
		VkResult result = swapchain_->queuePresent(context_->graphicsQueue(), imageIndex, renderFinishedSemaphores_[currentFrameIndex_]);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			// 스왑체인 재생성 필요
		}
		else if (result != VK_SUCCESS) {
			exitWithMessage("Failed to present swap chain image!");
		}
	}

	uint32_t VulkanRHI::getCurrentFrameIndex() const
	{
		return currentFrameIndex_;
	}

	RHIBuffer* VulkanRHI::createBuffer(const RHIBufferCreateInfo& createInfo)
	{
		// TODO: 구현
		return nullptr;
	}

	RHIImage* VulkanRHI::createImage(const RHIImageCreateInfo& createInfo)
	{
		// TODO: 구현
		return nullptr;
	}

	RHIShader* VulkanRHI::createShader(const RHIShaderCreateInfo& createInfo)
	{
		// TODO: 구현
		return nullptr;
	}

	RHIPipeline* VulkanRHI::createPipeline(const RHIPipelineCreateInfo& createInfo)
	{
		// TODO: 구현
		return nullptr;
	}

	void VulkanRHI::destroyBuffer(RHIBuffer* buffer) { delete buffer; }
	void VulkanRHI::destroyImage(RHIImage* image) { delete image; }
	void VulkanRHI::destroyShader(RHIShader* shader) { delete shader; }
	void VulkanRHI::destroyPipeline(RHIPipeline* pipeline) { delete pipeline; }

	void VulkanRHI::beginCommandRecording()
	{
		VkCommandBuffer cmdBuffer = commandBuffers_[currentFrameIndex_].handle();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkBeginCommandBuffer(cmdBuffer, &beginInfo);
	}

	void VulkanRHI::endCommandRecording()
	{
		VkCommandBuffer cmdBuffer = commandBuffers_[currentFrameIndex_].handle();
		vkEndCommandBuffer(cmdBuffer);
	}

	void VulkanRHI::submitCommands()
	{
		VkCommandBuffer cmdBuffer = commandBuffers_[currentFrameIndex_].handle();

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores_[currentFrameIndex_] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores_[currentFrameIndex_] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkQueueSubmit(context_->graphicsQueue(), 1, &submitInfo, inFlightFences_[currentFrameIndex_]);
	}

	void VulkanRHI::cmdBindPipeline(RHIPipeline* pipeline) { /* TODO */ }
	void VulkanRHI::cmdBindVertexBuffer(RHIBuffer* buffer, RHIDeviceSize offset) { /* TODO */ }
	void VulkanRHI::cmdBindIndexBuffer(RHIBuffer* buffer, RHIDeviceSize offset) { /* TODO */ }
	void VulkanRHI::cmdBindDescriptorSets(RHIPipelineLayout* layout, RHIDescriptorSet** sets, uint32_t setCount) { /* TODO */ }
	void VulkanRHI::cmdSetViewport(const RHIViewport& viewport) { /* TODO */ }
	void VulkanRHI::cmdSetScissor(const RHIRect2D& scissor) { /* TODO */ }
	void VulkanRHI::cmdDraw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) { /* TODO */ }
	void VulkanRHI::cmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) { /* TODO */ }

	void VulkanRHI::createSurface()
	{
		if (!initInfo_.window) {
			exitWithMessage("Window is null!");
		}

		if (glfwCreateWindowSurface(context_->instance(), static_cast<GLFWwindow*>(initInfo_.window), nullptr, &surface_) != VK_SUCCESS) {
			exitWithMessage("Failed to create window surface!");
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

		VkDevice device = context_->device();

		for (size_t i = 0; i < maxFramesInFlight_; i++) {
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores_[i]);
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores_[i]);
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences_[i]);
		}
	}

} // namespace BinRenderer::Vulkan