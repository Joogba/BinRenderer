#pragma once

#include "../../Synchronization/RHISemaphore.h"
#include <vulkan/vulkan.h>

namespace BinRenderer::Vulkan
{
	/**
  * @brief Vulkan Semaphore 구현
	 */
	class VulkanSemaphore : public RHISemaphore
	{
	public:
		VulkanSemaphore(VkDevice device);
		~VulkanSemaphore() override;

		bool create();
		void destroy();

		// Vulkan 네이티브 접근
		VkSemaphore getVkSemaphore() const { return semaphore_; }

	private:
		VkDevice device_;
		VkSemaphore semaphore_ = VK_NULL_HANDLE;
	};

} // namespace BinRenderer::Vulkan
