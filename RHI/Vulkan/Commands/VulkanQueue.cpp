#include "VulkanQueue.h"
#include "VulkanCommandBuffer.h"
#include "../../Synchronization/RHIFence.h"
#include "Core/Logger.h"

namespace BinRenderer::Vulkan
{
	VulkanQueue::VulkanQueue(VkQueue queue, uint32_t queueFamilyIndex)
		: queue_(queue), queueFamilyIndex_(queueFamilyIndex)
	{
	}

	VulkanQueue::~VulkanQueue()
	{
		// 큐는 디바이스가 해제되면 자동으로 해제됨
	}

	void VulkanQueue::waitIdle()
	{
		vkQueueWaitIdle(queue_);
	}

	void VulkanQueue::submit(RHICommandBuffer* commandBuffer, RHIFence* fence)
	{
		auto* vulkanCommandBuffer = static_cast<VulkanCommandBuffer*>(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		VkCommandBuffer vkCmdBuffer = vulkanCommandBuffer->getVkCommandBuffer();
		submitInfo.pCommandBuffers = &vkCmdBuffer;

		VkFence vkFence = VK_NULL_HANDLE;
		if (fence)
		{
			// TODO: VulkanFence 구현 필요
			 // vkFence = static_cast<VulkanFence*>(fence)->getVkFence();
		}

		if (vkQueueSubmit(queue_, 1, &submitInfo, vkFence) != VK_SUCCESS)
		{
			exitWithMessage("Failed to submit command buffer to queue!");
		}
	}

	void VulkanQueue::submit(uint32_t submitCount, const RHISubmitInfo* submitInfos, RHIFence* fence)
	{
		// TODO: RHISubmitInfo 구현 필요
		// 현재는 간단한 구현만 제공
	}

	VkResult VulkanQueue::submitVulkan(uint32_t submitCount, const VkSubmitInfo* submitInfos, VkFence fence)
	{
		return vkQueueSubmit(queue_, submitCount, submitInfos, fence);
	}

	VkResult VulkanQueue::present(const VkPresentInfoKHR* presentInfo)
	{
		return vkQueuePresentKHR(queue_, presentInfo);
	}

} // namespace BinRenderer::Vulkan
