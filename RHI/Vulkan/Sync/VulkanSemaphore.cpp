#include "VulkanSemaphore.h"
#include "Vulkan/Logger.h"

namespace BinRenderer::Vulkan
{
	VulkanSemaphore::VulkanSemaphore(VkDevice device)
		: device_(device)
	{
	}

	VulkanSemaphore::~VulkanSemaphore()
	{
		destroy();
	}

	bool VulkanSemaphore::create()
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &semaphore_) != VK_SUCCESS)
		{
			return false;
		}

		return true;
	}

	void VulkanSemaphore::destroy()
	{
		if (semaphore_ != VK_NULL_HANDLE)
		{
			vkDestroySemaphore(device_, semaphore_, nullptr);
			semaphore_ = VK_NULL_HANDLE;
		}
	}

} // namespace BinRenderer::Vulkan
