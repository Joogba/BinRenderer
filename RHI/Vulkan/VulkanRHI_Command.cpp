#include "VulkanRHI.h"
#include "Commands/VulkanCommandBuffer.h"
#include "Utilities/VulkanBarrier.h"
#include "Resources/VulkanBuffer.h"
#include "Resources/VulkanImage.h"
#include "Pipeline/VulkanPipeline.h"
#include "Pipeline/VulkanPipelineLayout.h"
#include "Pipeline/VulkanDescriptor.h"
#include "Core/Logger.h"

namespace BinRenderer::Vulkan
{
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

		std::vector<RHIDescriptorSet*> ptrSets(setCount);
		
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

		auto* vulkanPipeline = static_cast<VulkanPipeline*>(pipeline);
		VkPipelineLayout vkPipelineLayout = vulkanPipeline->getVkPipelineLayout();
		
		if (vkPipelineLayout == VK_NULL_HANDLE)
		{
			printLog("❌ ERROR: Pipeline layout is null in cmdBindDescriptorSets");
			return;
		}

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

		VkCommandBuffer vkCmdBuffer = cmdBuffer->getVkCommandBuffer();
		vkCmdBindDescriptorSets(
			vkCmdBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			vkPipelineLayout,
			firstSet,
			setCount,
			vkDescriptorSets.data(),
			0, nullptr
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
		
		auto* vulkanLayout = dynamic_cast<VulkanPipelineLayout*>(layout);
		if (vulkanLayout)
		{
			vkCmdPushConstants(vkCmdBuffer, vulkanLayout->getVkPipelineLayout(), 
				static_cast<VkShaderStageFlags>(stageFlags), offset, size, pValues);
			return;
		}
		
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

		auto* vulkanPipeline = static_cast<VulkanPipeline*>(pipeline);
		VkPipelineLayout vkPipelineLayout = vulkanPipeline->getVkPipelineLayout();
		
		if (vkPipelineLayout == VK_NULL_HANDLE)
		{
			printLog("❌ ERROR: Pipeline layout is null in cmdPushConstants");
			return;
		}

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

		if (!swapchain_)
		{
			printLog("❌ ERROR: Swapchain is null in cmdEndRendering");
			return;
		}

		VkImage swapchainImage = swapchain_->getVkImage(currentImageIndex_);
		VkFormat swapchainFormat = swapchain_->getColorFormat();

		VulkanBarrier barrier(swapchainImage, swapchainFormat, 1, 1);
		barrier.transitionColorToPresent(vkCmdBuffer);
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

		auto* vulkanImage = static_cast<VulkanImage*>(image);
		VkImage vkImage = vulkanImage->getVkImage();
		VkFormat vkFormat = static_cast<VkFormat>(image->getFormat());

		VkCommandBuffer cmdBuffer = commandBuffers_[currentFrameIndex_]->getVkCommandBuffer();

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

} // namespace BinRenderer::Vulkan
