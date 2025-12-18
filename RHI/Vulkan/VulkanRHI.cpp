#include "VulkanRHI.h"
#include "Resources/VulkanBuffer.h"
#include "Resources/VulkanImage.h"
#include "Resources/VulkanShader.h"
#include "Resources/VulkanSampler.h"
#include "Pipeline/VulkanPipeline.h"
#include "Pipeline/VulkanPipelineLayout.h"
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
			// ✅ 헤드리스 모드 여부 확인
			bool requireSwapchain = (initInfo.window != nullptr);
			
			// VulkanContext 초기화 (스왑체인 필요 여부 전달)
			context_ = std::make_unique<VulkanContext>();
			if (!context_->initialize(initInfo.requiredInstanceExtensions, initInfo.enableValidationLayer, requireSwapchain))
			{
				printLog("Failed to initialize Vulkan context");
				return false;
			}

			// ✅ 헤드리스 모드 체크: window가 있을 때만 스왑체인 생성
			if (initInfo.window != nullptr) {
				printLog("Creating swapchain for window mode...");
				createSurface();
				createSwapchain();
			} else {
				printLog("⚠️  Headless mode: Skipping swapchain creation");
			}

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

			printLog("✅ VulkanRHI initialized successfully ({})", 
			  initInfo.window ? "Window Mode" : "Headless Mode");
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
		if (!swapchain_)
		{
			return false;
		}

		currentFrameIndex_ = (currentFrameIndex_ + 1) % maxFramesInFlight_;

		VkDevice device = context_->getDevice();
		vkWaitForFences(device, 1, &inFlightFences_[currentFrameIndex_], VK_TRUE, UINT64_MAX);

		VkResult result = swapchain_->acquireNextImage(imageAvailableSemaphores_[currentFrameIndex_], imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			// 스왑체인 재생성 필요
			return false;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			printLog("ERROR: Failed to acquire swap chain image!");
			return false;
		}

		currentImageIndex_ = imageIndex;
		vkResetFences(device, 1, &inFlightFences_[currentFrameIndex_]);
		return true;
	}

	void VulkanRHI::endFrame(uint32_t imageIndex)
	{
		if (!swapchain_)
		{
			return;
		}

		VkResult result = swapchain_->present(context_->getPresentQueue(), imageIndex, renderFinishedSemaphores_[currentFrameIndex_]);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			// 스왑체인 재생성 필요
			printLog("Swapchain needs recreation");
		}
		else if (result != VK_SUCCESS)
		{
			printLog("ERROR: Failed to present swap chain image!");
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

	RHIImageView* VulkanRHI::createImageView(RHIImage* image, const RHIImageViewCreateInfo& createInfo)
	{
		if (!image)
		{
			return nullptr;
		}

		auto* vulkanImage = static_cast<VulkanImage*>(image);
		auto* imageView = new VulkanImageView(context_->getDevice(), vulkanImage);
  
    // createInfo를 Vulkan 타입으로 변환
   VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
        if (createInfo.viewType == RHI_IMAGE_VIEW_TYPE_CUBE)
    viewType = VK_IMAGE_VIEW_TYPE_CUBE;
  else if (createInfo.viewType == RHI_IMAGE_VIEW_TYPE_3D)
      viewType = VK_IMAGE_VIEW_TYPE_3D;

        VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    if (createInfo.aspectMask == RHI_IMAGE_ASPECT_DEPTH_BIT)
   aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
     else if (createInfo.aspectMask == RHI_IMAGE_ASPECT_STENCIL_BIT)
  aspectFlags = VK_IMAGE_ASPECT_STENCIL_BIT;

if (!imageView->create(viewType, aspectFlags))
 {
      delete imageView;
            return nullptr;
      }
    
 return imageView;
	}

	RHISampler* VulkanRHI::createSampler(const RHISamplerCreateInfo& createInfo)
	{
		auto* sampler = new VulkanSampler(context_->getDevice());
		
		// TODO: createInfo 파라미터 사용하여 Sampler 생성
		// 현재는 기본 Linear 설정
		if (!sampler->createLinear())
		{
			delete sampler;
			return nullptr;
		}
	 
		return sampler;
	}

	void VulkanRHI::destroyBuffer(RHIBuffer* buffer) { delete buffer; }
	void VulkanRHI::destroyImage(RHIImage* image) { delete image; }
	void VulkanRHI::destroyShader(RHIShader* shader) { delete shader; }
	void VulkanRHI::destroyPipeline(RHIPipeline* pipeline) { delete pipeline; }
	void VulkanRHI::destroyImageView(RHIImageView* imageView) { delete imageView; }
	void VulkanRHI::destroySampler(RHISampler* sampler) { delete sampler; }

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

	void VulkanRHI::cmdPushConstants(RHIPipelineLayout* layout, RHIShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues)
	{
		VkCommandBuffer cmdBuffer = commandBuffers_[currentFrameIndex_]->getVkCommandBuffer();
		
		// layout이 VulkanPipelineLayout인 경우
		auto* vulkanLayout = dynamic_cast<VulkanPipelineLayout*>(layout);
		if (vulkanLayout)
		{
			vkCmdPushConstants(cmdBuffer, vulkanLayout->getVkPipelineLayout(), 
				static_cast<VkShaderStageFlags>(stageFlags), offset, size, pValues);
			return;
		}
		
		// layout이 nullptr이거나 다른 타입인 경우 - 에러 처리
		// TODO: 에러 로깅
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

	void* VulkanRHI::mapBuffer(RHIBuffer* buffer)
	{
      if (!buffer)
 {
     return nullptr;
        }

auto* vulkanBuffer = static_cast<VulkanBuffer*>(buffer);
        return vulkanBuffer->map();
 }

    void VulkanRHI::unmapBuffer(RHIBuffer* buffer)
    {
   if (!buffer)
  {
     return;
        }

        auto* vulkanBuffer = static_cast<VulkanBuffer*>(buffer);
   vulkanBuffer->unmap();
    }

    void VulkanRHI::flushBuffer(RHIBuffer* buffer, RHIDeviceSize offset, RHIDeviceSize size)
    {
        if (!buffer)
   {
      return;
  }

        auto* vulkanBuffer = static_cast<VulkanBuffer*>(buffer);
   vulkanBuffer->flush(offset, size);
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
		// VulkanSwapchain 사용
		swapchain_ = std::make_unique<VulkanSwapchain>(context_.get());
		
		if (!swapchain_->create(surface_, initInfo_.windowWidth, initInfo_.windowHeight, false))
		{
			printLog("Failed to create VulkanSwapchain");
			return;
		}
		
		printLog("VulkanSwapchain created successfully");
	}

	void VulkanRHI::destroySwapchain()
	{
		if (swapchain_)
		{
			swapchain_->destroy();
			swapchain_.reset();
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
