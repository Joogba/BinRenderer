#include "VulkanFence.h"
#include "Core/Logger.h"

namespace BinRenderer::Vulkan
{
	VulkanFence::VulkanFence(VkDevice device)
		: device_(device)
	{
	}

	VulkanFence::~VulkanFence()
	{
		destroy();
	}

	bool VulkanFence::create(bool signaled)
	{
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

		if (vkCreateFence(device_, &fenceInfo, nullptr, &fence_) != VK_SUCCESS)
		{
			return false;
		}

		return true;
	}

	void VulkanFence::destroy()
	{
		if (fence_ != VK_NULL_HANDLE)
		{
			vkDestroyFence(device_, fence_, nullptr);
			fence_ = VK_NULL_HANDLE;
		}
	}

	void VulkanFence::wait(uint64_t timeout)
	{
		vkWaitForFences(device_, 1, &fence_, VK_TRUE, timeout);
	}

	void VulkanFence::reset()
	{
		vkResetFences(device_, 1, &fence_);
	}

	bool VulkanFence::isSignaled()
	{
		return vkGetFenceStatus(device_, fence_) == VK_SUCCESS;
	}

} // namespace BinRenderer::Vulkan
