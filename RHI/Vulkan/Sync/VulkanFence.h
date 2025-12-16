#pragma once

#include "../../Synchronization/RHIFence.h"
#include <vulkan/vulkan.h>

namespace BinRenderer::Vulkan
{
	/**
	 * @brief Vulkan Fence 구현
  */
	class VulkanFence : public RHIFence
	{
	public:
		VulkanFence(VkDevice device);
		~VulkanFence() override;

		bool create(bool signaled = false);
		void destroy();

		// RHIFence 인터페이스 구현
		void wait(uint64_t timeout = UINT64_MAX) override;
		void reset() override;
		bool isSignaled() override;

		// Vulkan 네이티브 접근
		VkFence getVkFence() const { return fence_; }

	private:
		VkDevice device_;
		VkFence fence_ = VK_NULL_HANDLE;
	};

} // namespace BinRenderer::Vulkan
