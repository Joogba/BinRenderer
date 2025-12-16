#include "VulkanCommandBuffer.h"
#include "VulkanCommandPool.h"
#include "../Resources/VulkanBuffer.h"
#include "../Pipeline/VulkanPipeline.h"
#include "../Pipeline/VulkanDescriptor.h"
#include "Vulkan/Logger.h"

namespace BinRenderer::Vulkan
{
	VulkanCommandBuffer::VulkanCommandBuffer(VkDevice device, VkCommandBuffer commandBuffer, VulkanCommandPool* pool)
		: device_(device), commandBuffer_(commandBuffer), pool_(pool)
	{
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		// 커맨드 버퍼는 풀이 해제되면 자동으로 해제됨
	}

	void VulkanCommandBuffer::begin()
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(commandBuffer_, &beginInfo) != VK_SUCCESS)
		{
			exitWithMessage("Failed to begin recording command buffer!");
		}

		isRecording_ = true;
	}

	void VulkanCommandBuffer::end()
	{
		if (!isRecording_)
		{
			return;
		}

		if (vkEndCommandBuffer(commandBuffer_) != VK_SUCCESS)
		{
			exitWithMessage("Failed to record command buffer!");
		}

		isRecording_ = false;
	}

	void VulkanCommandBuffer::reset()
	{
		vkResetCommandBuffer(commandBuffer_, 0);
		isRecording_ = false;
	}

	void VulkanCommandBuffer::bindPipeline(RHIPipeline* pipeline)
	{
		auto* vulkanPipeline = static_cast<VulkanPipeline*>(pipeline);
		VkPipelineBindPoint bindPoint = static_cast<VkPipelineBindPoint>(vulkanPipeline->getBindPoint());
		vkCmdBindPipeline(commandBuffer_, bindPoint, vulkanPipeline->getVkPipeline());
	}

	void VulkanCommandBuffer::bindVertexBuffer(uint32_t binding, RHIBuffer* buffer, RHIDeviceSize offset)
	{
		auto* vulkanBuffer = static_cast<VulkanBuffer*>(buffer);
		VkBuffer vkBuffer = vulkanBuffer->getVkBuffer();
		VkDeviceSize vkOffset = offset;
		vkCmdBindVertexBuffers(commandBuffer_, binding, 1, &vkBuffer, &vkOffset);
	}

	void VulkanCommandBuffer::bindIndexBuffer(RHIBuffer* buffer, RHIDeviceSize offset)
	{
		auto* vulkanBuffer = static_cast<VulkanBuffer*>(buffer);
		vkCmdBindIndexBuffer(commandBuffer_, vulkanBuffer->getVkBuffer(), offset, VK_INDEX_TYPE_UINT32);
	}

	void VulkanCommandBuffer::bindDescriptorSets(RHIPipelineLayout* layout, uint32_t firstSet, uint32_t setCount, RHIDescriptorSet** sets)
	{
		// TODO: 파이프라인 레이아웃 구현 필요
		std::vector<VkDescriptorSet> vkSets;
		for (uint32_t i = 0; i < setCount; ++i)
		{
			auto* vulkanSet = static_cast<VulkanDescriptorSet*>(sets[i]);
			vkSets.push_back(vulkanSet->getVkDescriptorSet());
		}

		// 임시: 그래픽스 파이프라인 가정
	 // vkCmdBindDescriptorSets(commandBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, 
		  //  pipelineLayout, firstSet, setCount, vkSets.data(), 0, nullptr);
	}

	void VulkanCommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
	{
		vkCmdDraw(commandBuffer_, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void VulkanCommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
	{
		vkCmdDrawIndexed(commandBuffer_, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void VulkanCommandBuffer::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
	{
		vkCmdDispatch(commandBuffer_, groupCountX, groupCountY, groupCountZ);
	}

} // namespace BinRenderer::Vulkan
