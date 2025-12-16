#pragma once

#include "../../Commands/RHICommandPool.h"
#include <vulkan/vulkan.h>
#include <vector>

namespace BinRenderer::Vulkan
{
	class VulkanCommandBuffer;

	/**
	 * @brief Vulkan 커맨드 풀 구현
   */
	class VulkanCommandPool : public RHICommandPool
	{
	public:
		VulkanCommandPool(VkDevice device);
		~VulkanCommandPool() override;

		bool create(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
		void destroy();

		// RHICommandPool 인터페이스 구현
		void reset() override;
		RHICommandBuffer* allocateCommandBuffer(RHICommandBufferLevel level = RHI_COMMAND_BUFFER_LEVEL_PRIMARY) override;

		// Vulkan 네이티브 접근
		VkCommandPool getVkCommandPool() const { return commandPool_; }

		// 여러 커맨드 버퍼 할당
		std::vector<VulkanCommandBuffer*> allocateCommandBuffers(uint32_t count, RHICommandBufferLevel level = RHI_COMMAND_BUFFER_LEVEL_PRIMARY);

	private:
		VkDevice device_;
		VkCommandPool commandPool_ = VK_NULL_HANDLE;
		std::vector<VulkanCommandBuffer*> allocatedBuffers_;
	};

} // namespace BinRenderer::Vulkan
