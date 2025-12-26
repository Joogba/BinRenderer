#include "VulkanRHI.h"
#include "Commands/VulkanCommandPool.h"
#include "Commands/VulkanCommandBuffer.h"
#include "Resources/VulkanBuffer.h"
#include "Resources/VulkanImage.h"
#include "Resources/VulkanShader.h"
#include "Resources/VulkanSampler.h"
#include "Resources/VulkanTexture.h"
#include "Pipeline/VulkanPipeline.h"
#include "Pipeline/VulkanPipelineLayout.h"
#include "Pipeline/VulkanDescriptor.h"
#include "Utilities/VulkanBarrier.h"
#include "Core/Logger.h"
#include "../../Platform/IWindow.h"

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
			//  헤드레스 모드 여부 확인 (IWindow 우선, 레거시 window 폴백)
			bool requireSwapchain = (initInfo.windowInterface != nullptr || initInfo.window != nullptr);
			
			// VulkanContext 초기화 (스왑체인 필요 여부 전달)
			context_ = std::make_unique<VulkanContext>();
			if (!context_->initialize(initInfo.requiredInstanceExtensions, initInfo.enableValidationLayer, requireSwapchain))
			{
				printLog("Failed to initialize Vulkan context");
				return false;
			}

			//  헤드레스 모드 체크: window가 있을 때만 스왑체인 생성
			if (requireSwapchain) {
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

			printLog(" VulkanRHI initialized successfully ({})", 
			  requireSwapchain ? "Window Mode" : "Headless Mode");
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
			//  모든 semaphores 정리 (maxFramesInFlight가 아니라 실제 개수)
			for (auto semaphore : imageAvailableSemaphores_)
			{
				if (semaphore != VK_NULL_HANDLE)
				{
					vkDestroySemaphore(device, semaphore, nullptr);
				}
			}
			for (auto semaphore : renderFinishedSemaphores_)
			{
				if (semaphore != VK_NULL_HANDLE)
				{
					vkDestroySemaphore(device, semaphore, nullptr);
				}
			}
			imageAvailableSemaphores_.clear();
			renderFinishedSemaphores_.clear();

			//  Fences 정리 (maxFramesInFlight 개수)
			for (auto fence : inFlightFences_)
			{
				if (fence != VK_NULL_HANDLE)
				{
					vkDestroyFence(device, fence, nullptr);
				}
			}
			inFlightFences_.clear();

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
		
		//  현재 frame의 fence 대기
		vkWaitForFences(device, 1, &inFlightFences_[currentFrameIndex_], VK_TRUE, UINT64_MAX);

		//  먼저 fence만으로 imageIndex 획득
		VkResult result = swapchain_->acquireNextImage(VK_NULL_HANDLE, imageIndex);
		
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			return false;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			printLog("ERROR: Failed to acquire swap chain image!");
			return false;
		}

		//  이 image가 이전 frame에서 사용 중이면 대기
		if (imagesInFlight_[imageIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(device, 1, &imagesInFlight_[imageIndex], VK_TRUE, UINT64_MAX);
		}
		
		//  이 image를 현재 frame의 fence로 마크
		imagesInFlight_[imageIndex] = inFlightFences_[currentFrameIndex_];

		//  imageIndex를 저장 (submitCommands와 endFrame에서 사용)
		currentImageIndex_ = imageIndex;
		
		//  fence reset은 acquire 후, submit 전에 수행
		vkResetFences(device, 1, &inFlightFences_[currentFrameIndex_]);
		
		return true;
	}

	void VulkanRHI::endFrame(uint32_t imageIndex)
	{
		if (!swapchain_)
		{
			return;
		}

		// submitCommands()가 이미 호출되었으므로 여기서는 present만 수행
		//  currentImageIndex_로 semaphore 사용 (swapchain image별 semaphore)
		VkResult result = swapchain_->present(context_->getPresentQueue(), imageIndex, renderFinishedSemaphores_[currentImageIndex_]);

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

	uint32_t VulkanRHI::getCurrentImageIndex() const
	{
		return currentImageIndex_;
	}

	RHIImageViewHandle VulkanRHI::getSwapchainImageView(uint32_t index) const
	{
		if (!swapchain_ || index >= swapchainImageViewHandles_.size())
		{
			printLog("❌ ERROR: Swapchain is null or invalid index in getSwapchainImageView");
			return {};
		}

		return swapchainImageViewHandles_[index];
	}

RHIBufferHandle VulkanRHI::createBuffer(const RHIBufferCreateInfo& createInfo)
	{
		auto* vulkanBuffer = new VulkanBuffer(context_->getDevice(), context_->getPhysicalDevice());
		if (!vulkanBuffer->create(createInfo))
		{
			delete vulkanBuffer;
			return {};
		}
		return bufferPool.insert(vulkanBuffer);
	}

	RHIImageHandle VulkanRHI::createImage(const RHIImageCreateInfo& createInfo)
	{
		auto* vulkanImage = new VulkanImage(context_->getDevice(), context_->getPhysicalDevice());
		if (!vulkanImage->create(createInfo))
		{
			delete vulkanImage;
			return {};
		}
		return imagePool.insert(vulkanImage);
	}

	RHIShaderHandle VulkanRHI::createShader(const RHIShaderCreateInfo& createInfo)
	{
		auto* vulkanShader = new VulkanShader(context_->getDevice());
		if (!vulkanShader->create(createInfo))
		{
			delete vulkanShader;
			return {};
		}
		return shaderPool.insert(vulkanShader);
	}

	RHIPipelineHandle VulkanRHI::createPipeline(const RHIPipelineCreateInfo& createInfo)
	{
		// Descriptor Set Layout 핸들 해석
		std::vector<RHIDescriptorSetLayout*> resolvedLayouts;
		resolvedLayouts.reserve(createInfo.descriptorSetLayouts.size());
		
		for (const auto& handle : createInfo.descriptorSetLayouts)
		{
			RHIDescriptorSetLayout* layout = descriptorSetLayoutPool.get(handle);
			if (layout)
			{
				resolvedLayouts.push_back(layout);
			}
			else
			{
				printLog("❌ ERROR: Invalid descriptor set layout handle in createPipeline");
				return {};
			}
		}

		auto* vulkanPipeline = new VulkanPipeline(context_->getDevice());

		// createinfor로 ShaderPool 에서 받아온후 VulkanShader* 로 변환한후 함수호출
		std::vector<VulkanShader*> vulkanShaders;
		for (const auto& shaderHandle : createInfo.shaderStages)
		{
			RHIShader* shader = shaderPool.get(shaderHandle);
			if (!shader)
			{
				printLog("ERROR: Invalid shader handle in createPipeline");
				delete vulkanPipeline;
				return {};
			}
			vulkanShaders.push_back(static_cast<VulkanShader*>(shader));
		}


		if (!vulkanPipeline->create(createInfo, resolvedLayouts, vulkanShaders))
		{
			delete vulkanPipeline;
			return {};
		}
		return pipelinePool.insert(vulkanPipeline);
	}

	RHIImageViewHandle VulkanRHI::createImageView(RHIImageHandle imageHandle, const RHIImageViewCreateInfo& createInfo)
	{
		RHIImage* image = imagePool.get(imageHandle);
		if (!image)
		{
			return {};
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
            return {};
      }
    
 return imageViewPool.insert(imageView);
	}

	RHISamplerHandle VulkanRHI::createSampler(const RHISamplerCreateInfo& createInfo)
	{
		auto* sampler = new VulkanSampler(context_->getDevice());
		
		// TODO: createInfo 파라미터 사용하여 Sampler 생성
		// 현재는 기본 Linear 설정
		if (!sampler->createLinear())
		{
			delete sampler;
			return {};
		}
	 
		return samplerPool.insert(sampler);
	}

	void VulkanRHI::destroyBuffer(RHIBufferHandle buffer) { bufferPool.remove(buffer); }
	void VulkanRHI::destroyImage(RHIImageHandle image) { imagePool.remove(image); }
	void VulkanRHI::destroyShader(RHIShaderHandle shader) { shaderPool.remove(shader); }
	void VulkanRHI::destroyPipeline(RHIPipelineHandle pipeline) { pipelinePool.remove(pipeline); }
	void VulkanRHI::destroyImageView(RHIImageViewHandle imageView) { imageViewPool.remove(imageView); }
	void VulkanRHI::destroySampler(RHISamplerHandle sampler) { samplerPool.remove(sampler); }

	void VulkanRHI::beginCommandRecording()
	{
		// 커맨드 버퍼 유효성 검사
		if (commandBuffers_.empty() || currentFrameIndex_ >= commandBuffers_.size())
		{
			printLog("❌ ERROR: Invalid command buffer index {} (size: {})", 
				currentFrameIndex_, commandBuffers_.size());
			return;
		}

		VulkanCommandBuffer* cmdBuffer = commandBuffers_[currentFrameIndex_];
		if (!cmdBuffer)
		{
			printLog("❌ ERROR: Command buffer {} is null", currentFrameIndex_);
			return;
		}

		cmdBuffer->reset();
		cmdBuffer->begin();
	}

	void VulkanRHI::endCommandRecording()
	{
		if (commandBuffers_.empty() || currentFrameIndex_ >= commandBuffers_.size())
		{
			printLog("❌ ERROR: Invalid command buffer index in endCommandRecording");
			return;
		}

		VulkanCommandBuffer* cmdBuffer = commandBuffers_[currentFrameIndex_];
		if (!cmdBuffer)
		{
			printLog("❌ ERROR: Command buffer is null in endCommandRecording");
			return;
		}

		cmdBuffer->end();
	}

	void VulkanRHI::submitCommands()
	{
		if (commandBuffers_.empty() || currentFrameIndex_ >= commandBuffers_.size())
		{
			printLog("❌ ERROR: Invalid command buffer index in submitCommands");
			return;
		}

		VulkanCommandBuffer* cmdBuffer = commandBuffers_[currentFrameIndex_];
		if (!cmdBuffer)
		{
			printLog("❌ ERROR: Command buffer is null in submitCommands");
			return;
		}

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		
		// ❌ acquire에서 semaphore를 사용하지 않았으므로 wait 불필요
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
		submitInfo.pWaitDstStageMask = nullptr;
		
		// 커맨드 버퍼 설정
		submitInfo.commandBufferCount = 1;
		VkCommandBuffer vkCmdBuffer = cmdBuffer->getVkCommandBuffer();
		submitInfo.pCommandBuffers = &vkCmdBuffer;
		
		//  currentImageIndex_로 semaphore 선택 (present에서 사용)
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores_[currentImageIndex_] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		// Fence와 함께 제출
		VkResult result = vkQueueSubmit(context_->getGraphicsQueue(), 1, &submitInfo, inFlightFences_[currentFrameIndex_]);
		if (result != VK_SUCCESS)
		{
			printLog("❌ ERROR: Failed to submit commands! Error: {}", static_cast<int>(result));
		}
	}

	void VulkanRHI::cmdBindPipeline(RHIPipelineHandle pipelineHandle)
	{
		if (commandBuffers_.empty() || currentFrameIndex_ >= commandBuffers_.size())
		{
			return;
		}

		VulkanCommandBuffer* cmdBuffer = commandBuffers_[currentFrameIndex_];
		if (!cmdBuffer)
		{
			return;
		}

		RHIPipeline* pipeline = pipelinePool.get(pipelineHandle);
		if (pipeline)
		{
			cmdBuffer->bindPipeline(pipeline);
		}
	}

	void VulkanRHI::cmdBindVertexBuffer(RHIBufferHandle bufferHandle, RHIDeviceSize offset)
	{
		if (commandBuffers_.empty() || currentFrameIndex_ >= commandBuffers_.size())
		{
			return;
		}

		VulkanCommandBuffer* cmdBuffer = commandBuffers_[currentFrameIndex_];
		if (!cmdBuffer)
		{
			return;
		}

		RHIBuffer* buffer = bufferPool.get(bufferHandle);
		if (buffer)
		{
			cmdBuffer->bindVertexBuffer(0, buffer, offset);
		}
	}

	void VulkanRHI::cmdBindIndexBuffer(RHIBufferHandle bufferHandle, RHIDeviceSize offset)
	{
		if (commandBuffers_.empty() || currentFrameIndex_ >= commandBuffers_.size())
		{
			return;
		}

		VulkanCommandBuffer* cmdBuffer = commandBuffers_[currentFrameIndex_];
		if (!cmdBuffer)
		{
			return;
		}

		RHIBuffer* buffer = bufferPool.get(bufferHandle);
		if (buffer)
		{
			cmdBuffer->bindIndexBuffer(buffer, offset);
		}
	}

	void VulkanRHI::cmdBindDescriptorSets(RHIPipelineLayout* layout, const RHIDescriptorSetHandle* sets, uint32_t setCount)
	{
		if (commandBuffers_.empty() || currentFrameIndex_ >= commandBuffers_.size())
		{
			return;
		}

		VulkanCommandBuffer* cmdBuffer = commandBuffers_[currentFrameIndex_];
		if (!cmdBuffer)
		{
			return;
		}

		// Handle 배열을 포인터 배열로 변환 (임시)
		// 주의: VulkanCommandBuffer::bindDescriptorSets가 RHIDescriptorSet**를 받음.
		// 여기서는 임시 벡터를 만들어서 변환해야 함.
		std::vector<RHIDescriptorSet*> ptrSets(setCount);
		// DescriptorSet은 Pool이 없거나 별도 관리?
		// VulkanRHI.h에 descriptorSetLayoutPool은 있는데 descriptorSetPool은 없음?
		// 아, allocateDescriptorSet에서 vulkanPool->allocateDescriptorSet을 호출함.
		// DescriptorSet은 Pool에서 관리되지 않고 VulkanDescriptorPool이 관리함.
		// 하지만 allocateDescriptorSet은 RHIDescriptorSetHandle을 반환함.
		// 즉, DescriptorSet도 Handle로 관리되어야 함.
		// VulkanRHI.h에 descriptorSetLayoutPool만 있고 descriptorSetPool은 없음.
		// 아까 VulkanRHI.h를 수정할 때 descriptorSetLayoutPool만 있었음.
		// 하지만 allocateDescriptorSet은 RHIDescriptorSetHandle을 반환함.
		// 즉, DescriptorSet을 위한 Pool이 필요함.
		// VulkanRHI.h에 `RHIResourcePool<RHIDescriptorSet, RHIDescriptorSetHandle> descriptorSetLayoutPool;` 라고 되어 있음.
		// 이름이 잘못된 것 같음. `descriptorSetPool`이어야 할 것 같은데 타입은 `RHIDescriptorSet`임.
		// 일단 `descriptorSetLayoutPool` 변수명을 사용하겠음 (타입은 맞음).
		
		for(uint32_t i=0; i<setCount; ++i) {
			ptrSets[i] = descriptorSetPool.get(sets[i]);
		}

		cmdBuffer->bindDescriptorSets(layout, 0, setCount, ptrSets.data());
	}

	void VulkanRHI::cmdBindDescriptorSets(RHIPipelineHandle pipelineHandle, uint32_t firstSet, const RHIDescriptorSetHandle* sets, uint32_t setCount)
	{
		if (commandBuffers_.empty() || currentFrameIndex_ >= commandBuffers_.size())
		{
			printLog("❌ ERROR: Invalid command buffer index in cmdBindDescriptorSets");
			return;
		}

		VulkanCommandBuffer* cmdBuffer = commandBuffers_[currentFrameIndex_];
		if (!cmdBuffer)
		{
			printLog("❌ ERROR: Command buffer is null in cmdBindDescriptorSets");
			return;
		}

		RHIPipeline* pipeline = pipelinePool.get(pipelineHandle);
		if (!pipeline)
		{
			printLog("❌ ERROR: Pipeline is null in cmdBindDescriptorSets");
			return;
		}

		// Pipeline에서 layout 가져오기
		auto* vulkanPipeline = static_cast<VulkanPipeline*>(pipeline);
		VkPipelineLayout vkPipelineLayout = vulkanPipeline->getVkPipelineLayout();
		
		if (vkPipelineLayout == VK_NULL_HANDLE)
		{
			printLog("❌ ERROR: Pipeline layout is null in cmdBindDescriptorSets");
			return;
		}

		// Vulkan descriptor sets 변환
		std::vector<VkDescriptorSet> vkDescriptorSets(setCount);
		for (uint32_t i = 0; i < setCount; i++)
		{
			RHIDescriptorSet* set = descriptorSetPool.get(sets[i]);
			if (set)
			{
				auto* vulkanSet = static_cast<VulkanDescriptorSet*>(set);
				vkDescriptorSets[i] = vulkanSet->getVkDescriptorSet();
			}
			else
			{
				vkDescriptorSets[i] = VK_NULL_HANDLE;
			}
		}

		// Vulkan 커맨드 버퍼에 바인딩
		VkCommandBuffer vkCmdBuffer = cmdBuffer->getVkCommandBuffer();
		vkCmdBindDescriptorSets(
			vkCmdBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			vkPipelineLayout,
			firstSet,
			setCount,
			vkDescriptorSets.data(),
			0, nullptr // dynamic offsets
		);
	}

	void VulkanRHI::cmdPushConstants(RHIPipelineLayout* layout, RHIShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues)
	{
		if (commandBuffers_.empty() || currentFrameIndex_ >= commandBuffers_.size())
		{
			return;
		}

		VulkanCommandBuffer* cmdBuffer = commandBuffers_[currentFrameIndex_];
		if (!cmdBuffer)
		{
			return;
		}

		VkCommandBuffer vkCmdBuffer = cmdBuffer->getVkCommandBuffer();
		
		// layout이 VulkanPipelineLayout인 경우
		auto* vulkanLayout = dynamic_cast<VulkanPipelineLayout*>(layout);
		if (vulkanLayout)
		{
			vkCmdPushConstants(vkCmdBuffer, vulkanLayout->getVkPipelineLayout(), 
				static_cast<VkShaderStageFlags>(stageFlags), offset, size, pValues);
			return;
		}
		
		// layout이 nullptr이거나 다른 타입인 경우 - 에러 처리
		printLog("❌ ERROR: Invalid pipeline layout in cmdPushConstants");
	}

	void VulkanRHI::cmdPushConstants(RHIPipelineHandle pipelineHandle, RHIShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues)
	{
		if (commandBuffers_.empty() || currentFrameIndex_ >= commandBuffers_.size())
		{
			return;
		}

		VulkanCommandBuffer* cmdBuffer = commandBuffers_[currentFrameIndex_];
		if (!cmdBuffer)
		{
			return;
		}

		RHIPipeline* pipeline = pipelinePool.get(pipelineHandle);
		if (!pipeline)
		{
			printLog("❌ ERROR: Pipeline is null in cmdPushConstants");
			return;
		}

		// Pipeline에서 layout 가져오기
		auto* vulkanPipeline = static_cast<VulkanPipeline*>(pipeline);
		VkPipelineLayout vkPipelineLayout = vulkanPipeline->getVkPipelineLayout();
		
		if (vkPipelineLayout == VK_NULL_HANDLE)
		{
			printLog("❌ ERROR: Pipeline layout is null in cmdPushConstants");
			return;
		}

		// Push constants 전송
		VkCommandBuffer vkCmdBuffer = cmdBuffer->getVkCommandBuffer();
		vkCmdPushConstants(
			vkCmdBuffer,
			vkPipelineLayout,
			static_cast<VkShaderStageFlags>(stageFlags),
			offset,
			size,
			pValues
		);
	}

	void VulkanRHI::cmdSetViewport(const RHIViewport& viewport)
	{
		if (commandBuffers_.empty() || currentFrameIndex_ >= commandBuffers_.size())
		{
			return;
		}

		VulkanCommandBuffer* cmdBuffer = commandBuffers_[currentFrameIndex_];
		if (!cmdBuffer)
		{
			return;
		}

		VkCommandBuffer vkCmdBuffer = cmdBuffer->getVkCommandBuffer();
		VkViewport vkViewport{};
		vkViewport.x = viewport.x;
		vkViewport.y = viewport.y;
		vkViewport.width = viewport.width;
		vkViewport.height = viewport.height;
		vkViewport.minDepth = viewport.minDepth;
		vkViewport.maxDepth = viewport.maxDepth;
		vkCmdSetViewport(vkCmdBuffer, 0, 1, &vkViewport);
	}

	void VulkanRHI::cmdSetScissor(const RHIRect2D& scissor)
	{
		if (commandBuffers_.empty() || currentFrameIndex_ >= commandBuffers_.size())
		{
			return;
		}

		VulkanCommandBuffer* cmdBuffer = commandBuffers_[currentFrameIndex_];
		if (!cmdBuffer)
		{
			return;
		}

		VkCommandBuffer vkCmdBuffer = cmdBuffer->getVkCommandBuffer();
		VkRect2D vkScissor{};
		vkScissor.offset = { scissor.offset.x, scissor.offset.y };
		vkScissor.extent = { scissor.extent.width, scissor.extent.height };
		vkCmdSetScissor(vkCmdBuffer, 0, 1, &vkScissor);
	}

	void VulkanRHI::cmdDraw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
	{
		if (commandBuffers_.empty() || currentFrameIndex_ >= commandBuffers_.size())
		{
			return;
		}

		VulkanCommandBuffer* cmdBuffer = commandBuffers_[currentFrameIndex_];
		if (!cmdBuffer)
		{
			return;
		}

		cmdBuffer->draw(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void VulkanRHI::cmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
	{
		if (commandBuffers_.empty() || currentFrameIndex_ >= commandBuffers_.size())
		{
			return;
		}

		VulkanCommandBuffer* cmdBuffer = commandBuffers_[currentFrameIndex_];
		if (!cmdBuffer)
		{
			return;
		}

		cmdBuffer->drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}
    void* VulkanRHI::mapBuffer(RHIBufferHandle bufferHandle)
	{
        RHIBuffer* buffer = bufferPool.get(bufferHandle);
        if (!buffer)
        {
            return nullptr;
        }

        auto* vulkanBuffer = static_cast<VulkanBuffer*>(buffer);
        return vulkanBuffer->map();
    }

    void VulkanRHI::unmapBuffer(RHIBufferHandle bufferHandle)
    {
        RHIBuffer* buffer = bufferPool.get(bufferHandle);
        if (!buffer)
        {
            return;
        }

        auto* vulkanBuffer = static_cast<VulkanBuffer*>(buffer);
        vulkanBuffer->unmap();
    }

    void VulkanRHI::flushBuffer(RHIBufferHandle bufferHandle, RHIDeviceSize offset, RHIDeviceSize size)
    {
        RHIBuffer* buffer = bufferPool.get(bufferHandle);
        if (!buffer)
        {
            return;
        }

        auto* vulkanBuffer = static_cast<VulkanBuffer*>(buffer);
        vulkanBuffer->flush(offset, size);
    }

    void VulkanRHI::createSurface()
	{
		//  IWindow 인터페이스 사용
		if (initInfo_.windowInterface)
		{
			int result = initInfo_.windowInterface->createVulkanSurface(
				context_->getInstance(),
				reinterpret_cast<void**>(&surface_)
			);
			
			if (result != 0)  // VK_SUCCESS = 0
			{
				printLog("❌ ERROR: Failed to create Vulkan surface via IWindow: {}", static_cast<int>(result));
				exitWithMessage("Failed to create window surface!");
			}
			printLog(" Vulkan surface created via IWindow");
			return;
		}

		// ❌ 레거시: window (void*) 사용 (GLFW 전용, 제거 예정)
		if (!initInfo_.window)
		{
			exitWithMessage("Window is null!");
		}

		if (glfwCreateWindowSurface(context_->getInstance(), static_cast<GLFWwindow*>(initInfo_.window), nullptr, &surface_) != VK_SUCCESS)
		{
			exitWithMessage("Failed to create window surface!");
		}
		printLog("⚠️  Vulkan surface created via legacy GLFW (deprecated)");
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

		//  스왑체인 이미지 뷰를 풀에 등록
		uint32_t imageCount = swapchain_->getImageCount();
		swapchainImageViewHandles_.resize(imageCount);
		for(uint32_t i=0; i<imageCount; ++i) {
			// 주의: VulkanSwapchain이 소유한 포인터를 풀에 등록함.
			// Double Free 위험이 있으므로 추후 VulkanSwapchain 리팩토링 필요.
			swapchainImageViewHandles_[i] = imageViewPool.insert(swapchain_->getImageViewRaw(i));
			
			//  핸들을 Swapchain에 다시 설정 (RHISwapchain 인터페이스용)
			swapchain_->setImageViewHandle(i, swapchainImageViewHandles_[i]);
		}
	}

	void VulkanRHI::destroySwapchain()
	{
		if (swapchain_)
		{
			// 핸들 정보만 지움 (Pool에서 remove하면 delete되므로 안됨)
			// 하지만 Pool Destructor에서 delete될 것임.
			swapchainImageViewHandles_.clear();

			swapchain_->destroy();
			swapchain_.reset();
		}
	}

	void VulkanRHI::createSyncObjects()
	{
		//  Swapchain image 개수만큼 semaphores 생성 (Vulkan Best Practice)
		uint32_t swapchainImageCount = swapchain_ ? swapchain_->getImageCount() : 3;
		
		imageAvailableSemaphores_.resize(swapchainImageCount);
		renderFinishedSemaphores_.resize(swapchainImageCount);
		inFlightFences_.resize(maxFramesInFlight_);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VkDevice device = context_->getDevice();

		//  Swapchain image 개수만큼 semaphores 생성
		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores_[i]);
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores_[i]);
		}
		
		//  Frame-in-flight만큼 fences 생성
		for (size_t i = 0; i < maxFramesInFlight_; i++)
		{
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences_[i]);
		}

		//  Per-image fence tracking
		imagesInFlight_.resize(swapchainImageCount, VK_NULL_HANDLE);

		printLog(" Sync objects created: {} semaphores (per image), {} fences (per frame)", swapchainImageCount, maxFramesInFlight_);

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

	void VulkanRHI::cmdBeginRendering(uint32_t width, uint32_t height, RHIImageViewHandle colorAttachmentHandle, RHIImageViewHandle depthAttachmentHandle)
	{
		if (commandBuffers_.empty() || currentFrameIndex_ >= commandBuffers_.size())
		{
			printLog("❌ ERROR: Invalid command buffer in cmdBeginRendering");
			return;
		}

		VulkanCommandBuffer* cmdBuffer = commandBuffers_[currentFrameIndex_];
		if (!cmdBuffer)
		{
			printLog("❌ ERROR: Command buffer is null in cmdBeginRendering");
			return;
		}

		//  Color attachment 검증 및 올바른 캐스팅
		RHIImageView* colorAttachment = imageViewPool.get(colorAttachmentHandle);
		if (!colorAttachment)
		{
			printLog("❌ ERROR: Color attachment is null in cmdBeginRendering");
			return;
		}

		auto* vulkanColorImageView = static_cast<VulkanImageView*>(colorAttachment);
		VkImageView vkColorImageView = vulkanColorImageView->getVkImageView();
		
		if (vkColorImageView == VK_NULL_HANDLE)
		{
			printLog("❌ ERROR: VkImageView is null");
			return;
		}

		VkCommandBuffer vkCmdBuffer = cmdBuffer->getVkCommandBuffer();

		//  Swapchain null check 추가
		if (!swapchain_)
		{
			printLog("❌ ERROR: Swapchain is null in cmdBeginRendering");
			return;
		}

		//  Swapchain image 가져오기
		VkImage swapchainImage = swapchain_->getVkImage(currentImageIndex_);
		VkFormat swapchainFormat = swapchain_->getColorFormat();

		//  VulkanBarrier를 사용한 레이아웃 전환
		VulkanBarrier barrier(swapchainImage, swapchainFormat, 1, 1);
		barrier.transitionToColorAttachment(vkCmdBuffer);

		// Color attachment 설정
		VkRenderingAttachmentInfo colorAttachmentInfo{};
		colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		colorAttachmentInfo.imageView = vkColorImageView;
		colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentInfo.clearValue.color = {{0.1f, 0.1f, 0.3f, 1.0f}};

		// Depth attachment 설정 (옵션)
		VkRenderingAttachmentInfo depthAttachmentInfo{};
		VkImageView vkDepthImageView = VK_NULL_HANDLE;
		
		if (depthAttachmentHandle.isValid())
		{
			RHIImageView* depthAttachment = imageViewPool.get(depthAttachmentHandle);
			if (depthAttachment)
			{
				auto* vulkanDepthImageView = static_cast<VulkanImageView*>(depthAttachment);
				vkDepthImageView = vulkanDepthImageView->getVkImageView();
				
				if (vkDepthImageView != VK_NULL_HANDLE)
				{
					depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
					depthAttachmentInfo.imageView = vkDepthImageView;
					depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
					depthAttachmentInfo.clearValue.depthStencil = {1.0f, 0};
				}
			}
		}

		// Rendering info 설정
		VkRenderingInfo renderingInfo{};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderingInfo.renderArea.offset = {0, 0};
		renderingInfo.renderArea.extent = {width, height};
		renderingInfo.layerCount = 1;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &colorAttachmentInfo;
		renderingInfo.pDepthAttachment = (vkDepthImageView != VK_NULL_HANDLE) ? &depthAttachmentInfo : nullptr;
		renderingInfo.pStencilAttachment = nullptr;

		// Dynamic rendering 시작
		vkCmdBeginRendering(vkCmdBuffer, &renderingInfo);
		
		printLog("[VulkanRHI]  cmdBeginRendering called successfully");
	}

	void VulkanRHI::cmdEndRendering()
	{
		if (commandBuffers_.empty() || currentFrameIndex_ >= commandBuffers_.size())
		{
			return;
		}

		VulkanCommandBuffer* cmdBuffer = commandBuffers_[currentFrameIndex_];
		if (!cmdBuffer)
		{
			return;
		}

		VkCommandBuffer vkCmdBuffer = cmdBuffer->getVkCommandBuffer();

		// Dynamic rendering 종료
		vkCmdEndRendering(vkCmdBuffer);

		//  Swapchain null check 추가
		if (!swapchain_)
		{
			printLog("❌ ERROR: Swapchain is null in cmdEndRendering");
			return;
		}

		//  VulkanBarrier를 사용한 레이아웃 전환 (PRESENT_SRC로)
		VkImage swapchainImage = swapchain_->getVkImage(currentImageIndex_);
		VkFormat swapchainFormat = swapchain_->getColorFormat();

		VulkanBarrier barrier(swapchainImage, swapchainFormat, 1, 1);
		barrier.transitionColorToPresent(vkCmdBuffer);
	}

	// ========================================
	//  Descriptor Set API 구현
	// ========================================

	RHIDescriptorSetLayoutHandle VulkanRHI::createDescriptorSetLayout(const RHIDescriptorSetLayoutCreateInfo& createInfo)
	{
		auto* layout = new VulkanDescriptorSetLayout(context_->getDevice());
		
		// RHI 바인딩을 Vulkan 바인딩으로 변환
		std::vector<VkDescriptorSetLayoutBinding> vkBindings;
		vkBindings.reserve(createInfo.bindings.size());
		
		for (const auto& binding : createInfo.bindings)
		{
			VkDescriptorSetLayoutBinding vkBinding{};
			vkBinding.binding = binding.binding;
			vkBinding.descriptorType = static_cast<VkDescriptorType>(binding.descriptorType);
			vkBinding.descriptorCount = binding.descriptorCount;
			vkBinding.stageFlags = static_cast<VkShaderStageFlags>(binding.stageFlags);
			vkBinding.pImmutableSamplers = nullptr;
			
			vkBindings.push_back(vkBinding);
		}
		
		if (!layout->create(vkBindings))
		{
			delete layout;
			return {};
		}
		
		return descriptorSetLayoutPool.insert(layout);
	}

	RHIDescriptorPoolHandle VulkanRHI::createDescriptorPool(const RHIDescriptorPoolCreateInfo& createInfo)
	{
		auto* pool = new VulkanDescriptorPool(context_->getDevice());
		
		// RHI pool size를 Vulkan pool size로 변환
		std::vector<VkDescriptorPoolSize> vkPoolSizes;
		vkPoolSizes.reserve(createInfo.poolSizes.size());
		
		for (const auto& poolSize : createInfo.poolSizes)
		{
			VkDescriptorPoolSize vkPoolSize{};
			vkPoolSize.type = static_cast<VkDescriptorType>(poolSize.type);
			vkPoolSize.descriptorCount = poolSize.descriptorCount;
			
			vkPoolSizes.push_back(vkPoolSize);
		}
		
		if (!pool->create(createInfo.maxSets, vkPoolSizes))
		{
			delete pool;
			return {};
		}
		
		return descriptorPoolPool.insert(pool);
	}

	RHIDescriptorSetHandle VulkanRHI::allocateDescriptorSet(RHIDescriptorPoolHandle poolHandle, RHIDescriptorSetLayoutHandle layoutHandle)
	{
		RHIDescriptorPool* pool = descriptorPoolPool.get(poolHandle);
		RHIDescriptorSetLayout* layout = descriptorSetLayoutPool.get(layoutHandle);

		if (!pool || !layout)
		{
			printLog("❌ ERROR: Invalid pool or layout in allocateDescriptorSet");
			return {};
		}
		
		// RHI → Vulkan 캐스팅
		auto* vulkanPool = static_cast<VulkanDescriptorPool*>(pool);
		
		// VulkanDescriptorPool의 allocateDescriptorSet 사용
		RHIDescriptorSet* set = vulkanPool->allocateDescriptorSet(layout);
		if (!set) return {};

		return descriptorSetPool.insert(set);
	}

	void VulkanRHI::updateDescriptorSet(RHIDescriptorSetHandle setHandle, uint32_t binding, RHIBufferHandle bufferHandle, size_t offset, size_t range)
	{
		RHIDescriptorSet* set = descriptorSetPool.get(setHandle);
		RHIBuffer* buffer = bufferPool.get(bufferHandle);

		if (set && buffer)
		{
			set->updateBuffer(binding, buffer, offset, range);
		}
		else
		{
			printLog("❌ ERROR: Invalid set or buffer handle in updateDescriptorSet (Buffer)");
		}
	}

	void VulkanRHI::updateDescriptorSet(RHIDescriptorSetHandle setHandle, uint32_t binding, RHIImageViewHandle imageViewHandle, RHISamplerHandle samplerHandle)
	{
		RHIDescriptorSet* set = descriptorSetPool.get(setHandle);
		RHIImageView* imageView = imageViewPool.get(imageViewHandle);
		RHISampler* sampler = samplerPool.get(samplerHandle);

		if (set && imageView)
		{
			set->updateImage(binding, imageView, sampler);
		}
		else
		{
			printLog("❌ ERROR: Invalid set or imageView handle in updateDescriptorSet (Image)");
		}
	}

	void VulkanRHI::destroyDescriptorSetLayout(RHIDescriptorSetLayoutHandle layoutHandle)
	{
		RHIDescriptorSetLayout* layout = descriptorSetLayoutPool.get(layoutHandle);
		if (layout)
		{
			auto* vulkanLayout = static_cast<VulkanDescriptorSetLayout*>(layout);
			vulkanLayout->destroy();
			delete vulkanLayout;
			descriptorSetLayoutPool.remove(layoutHandle);
		}
	}

	void VulkanRHI::destroyDescriptorPool(RHIDescriptorPoolHandle poolHandle)
	{
		RHIDescriptorPool* pool = descriptorPoolPool.get(poolHandle);
		if (pool)
		{
			auto* vulkanPool = static_cast<VulkanDescriptorPool*>(pool);
			vulkanPool->destroy();
			delete vulkanPool;
			descriptorPoolPool.remove(poolHandle);
		}
	}

	void VulkanRHI::cmdTransitionImageLayout(
		RHIImageHandle imageHandle,
		RHIImageLayout oldLayout,
		RHIImageLayout newLayout,
		RHIImageAspectFlagBits aspectMask,
		uint32_t baseMipLevel,
		uint32_t levelCount,
		uint32_t baseArrayLayer,
		uint32_t layerCount
	)
	{
		RHIImage* image = imagePool.get(imageHandle);
		if (!image || commandBuffers_.empty())
		{
			printLog("❌ cmdTransitionImageLayout: Invalid image or no active command buffer");
			return;
		}

		// RHIImage → VulkanImage 캐스팅
		auto* vulkanImage = static_cast<VulkanImage*>(image);
		VkImage vkImage = vulkanImage->getVkImage();
		VkFormat vkFormat = static_cast<VkFormat>(image->getFormat());

		// 현재 커맨드 버퍼
		VkCommandBuffer cmdBuffer = commandBuffers_[currentFrameIndex_]->getVkCommandBuffer();

		// VulkanBarrier 헬퍼 사용
		BarrierHelpers::transitionImageLayout(
			cmdBuffer,
			vkImage,
			vkFormat,
			static_cast<VkImageLayout>(oldLayout),
			static_cast<VkImageLayout>(newLayout),
			levelCount,
			layerCount
		);

		printLog(" Image layout transitioned: {} -> {}",
			static_cast<int>(oldLayout), static_cast<int>(newLayout));
	}

	void VulkanRHI::cmdCopyBufferToImage(
		RHIBufferHandle srcBufferHandle,
		RHIImageHandle dstImageHandle,
		RHIImageLayout dstImageLayout,
		uint32_t regionCount,
		const RHIBufferImageCopy* pRegions
	)
	{
		if (commandBuffers_.empty() || currentFrameIndex_ >= commandBuffers_.size())
		{
			printLog("❌ ERROR: Invalid command buffer in cmdCopyBufferToImage");
			return;
		}

		VulkanCommandBuffer* cmdBuffer = commandBuffers_[currentFrameIndex_];
		if (!cmdBuffer)
		{
			printLog("❌ ERROR: Command buffer is null in cmdCopyBufferToImage");
			return;
		}

		RHIBuffer* srcBuffer = bufferPool.get(srcBufferHandle);
		RHIImage* dstImage = imagePool.get(dstImageHandle);

		if (!srcBuffer || !dstImage)
		{
			printLog("❌ ERROR: Invalid buffer or image in cmdCopyBufferToImage");
			return;
		}

		auto* vulkanBuffer = static_cast<VulkanBuffer*>(srcBuffer);
		auto* vulkanImage = static_cast<VulkanImage*>(dstImage);

		VkCommandBuffer vkCmdBuffer = cmdBuffer->getVkCommandBuffer();

		std::vector<VkBufferImageCopy> vkRegions(regionCount);
		for (uint32_t i = 0; i < regionCount; ++i)
		{
			vkRegions[i].bufferOffset = pRegions[i].bufferOffset;
			vkRegions[i].bufferRowLength = pRegions[i].bufferRowLength;
			vkRegions[i].bufferImageHeight = pRegions[i].bufferImageHeight;
			
			vkRegions[i].imageSubresource.aspectMask = static_cast<VkImageAspectFlags>(pRegions[i].imageSubresource.aspectMask);
			vkRegions[i].imageSubresource.mipLevel = pRegions[i].imageSubresource.mipLevel;
			vkRegions[i].imageSubresource.baseArrayLayer = pRegions[i].imageSubresource.baseArrayLayer;
			vkRegions[i].imageSubresource.layerCount = pRegions[i].imageSubresource.layerCount;
			
			vkRegions[i].imageOffset = { pRegions[i].imageOffset.x, pRegions[i].imageOffset.y, pRegions[i].imageOffset.z };
			vkRegions[i].imageExtent = { pRegions[i].imageExtent.width, pRegions[i].imageExtent.height, pRegions[i].imageExtent.depth };
		}

		vkCmdCopyBufferToImage(
			vkCmdBuffer,
			vulkanBuffer->getVkBuffer(),
			vulkanImage->getVkImage(),
			static_cast<VkImageLayout>(dstImageLayout),
			regionCount,
			vkRegions.data()
		);
	}

	RHITextureHandle VulkanRHI::createTexture(RHIImageHandle imageHandle, RHIImageViewHandle viewHandle, RHISamplerHandle samplerHandle)
	{
		RHIImage* image = imagePool.get(imageHandle);
		if (!image) return {};

		// 이미지 정보 가져오기
		uint32_t width = image->getWidth();
		uint32_t height = image->getHeight();
		uint32_t mipLevels = image->getMipLevels();

		auto* texture = new VulkanTexture(imageHandle, viewHandle, samplerHandle, width, height, mipLevels);
		return texturePool.insert(texture);
	}

	void VulkanRHI::destroyTexture(RHITextureHandle texture)
	{
		texturePool.remove(texture);
	}

} // namespace BinRenderer::Vulkan
