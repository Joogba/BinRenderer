#pragma once

#include "../../Synchronization/RHIEvent.h"
#include <vulkan/vulkan.h>

namespace BinRenderer::Vulkan
{
	/**
	 * @brief Vulkan Event 구현
  */
	class VulkanEvent : public RHIEvent
	{
	public:
		VulkanEvent(VkDevice device);
		~VulkanEvent() override;

		bool create();
		void destroy();

		// RHIEvent 인터페이스 구현
		void set() override;
		void reset() override;
		bool isSignaled() override;

		// Vulkan 네이티브 접근
		VkEvent getVkEvent() const { return event_; }

	private:
		VkDevice device_;
		VkEvent event_ = VK_NULL_HANDLE;
	};

} // namespace BinRenderer::Vulkan
