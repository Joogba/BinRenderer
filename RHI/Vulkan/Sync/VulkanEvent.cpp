#include "VulkanEvent.h"
#include "Vulkan/Logger.h"

namespace BinRenderer::Vulkan
{
	VulkanEvent::VulkanEvent(VkDevice device)
		: device_(device)
	{
	}

	VulkanEvent::~VulkanEvent()
	{
		destroy();
	}

	bool VulkanEvent::create()
	{
		VkEventCreateInfo eventInfo{};
		eventInfo.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;

		if (vkCreateEvent(device_, &eventInfo, nullptr, &event_) != VK_SUCCESS)
		{
			return false;
		}

		return true;
	}

	void VulkanEvent::destroy()
	{
		if (event_ != VK_NULL_HANDLE)
		{
			vkDestroyEvent(device_, event_, nullptr);
			event_ = VK_NULL_HANDLE;
		}
	}

	void VulkanEvent::set()
	{
		vkSetEvent(device_, event_);
	}

	void VulkanEvent::reset()
	{
		vkResetEvent(device_, event_);
	}

	bool VulkanEvent::isSignaled()
	{
		return vkGetEventStatus(device_, event_) == VK_EVENT_SET;
	}

} // namespace BinRenderer::Vulkan
