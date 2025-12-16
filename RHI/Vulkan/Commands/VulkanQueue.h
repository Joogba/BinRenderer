#pragma once

#include "../../Commands/RHICommandQueue.h"
#include <vulkan/vulkan.h>

namespace BinRenderer::Vulkan
{
	class VulkanCommandBuffer;

	/**
	   * @brief Vulkan 큐 구현
	*/
	class VulkanQueue : public RHICommandQueue
	{
	public:
		VulkanQueue(VkQueue queue, uint32_t queueFamilyIndex);
		~VulkanQueue() override;

		// RHICommandQueue 인터페이스 구현
		void waitIdle() override;
		void submit(RHICommandBuffer* commandBuffer, RHIFence* fence = nullptr) override;
		void submit(uint32_t submitCount, const struct RHISubmitInfo* submitInfos, RHIFence* fence = nullptr) override;

		// Vulkan 네이티브 접근
		VkQueue getVkQueue() const { return queue_; }
		uint32_t getQueueFamilyIndex() const { return queueFamilyIndex_; }

		// Vulkan 특화 메서드
		VkResult submitVulkan(uint32_t submitCount, const VkSubmitInfo* submitInfos, VkFence fence = VK_NULL_HANDLE);
		VkResult present(const VkPresentInfoKHR* presentInfo);

	private:
		VkQueue queue_;
		uint32_t queueFamilyIndex_;
	};

} // namespace BinRenderer::Vulkan
