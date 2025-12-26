#include "VulkanCommandPool.h"
#include "VulkanCommandBuffer.h"
#include "Core/Logger.h"

namespace BinRenderer::Vulkan
{
	VulkanCommandPool::VulkanCommandPool(VkDevice device)
		: device_(device)
	{
	}

	VulkanCommandPool::~VulkanCommandPool()
	{
		destroy();
	}

	bool VulkanCommandPool::create(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndex;
		poolInfo.flags = flags;

		if (vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool_) != VK_SUCCESS)
		{
			return false;
		}

		return true;
	}

	void VulkanCommandPool::destroy()
	{
		// 할당된 커맨드 버퍼 정리
		for (auto* buffer : allocatedBuffers_)
		{
			delete buffer;
		}
		allocatedBuffers_.clear();

		if (commandPool_ != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(device_, commandPool_, nullptr);
			commandPool_ = VK_NULL_HANDLE;
		}
	}

	void VulkanCommandPool::reset()
	{
		if (commandPool_ != VK_NULL_HANDLE)
		{
			vkResetCommandPool(device_, commandPool_, 0);
		}
	}

	RHICommandBuffer* VulkanCommandPool::allocateCommandBuffer(RHICommandBufferLevel level)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool_;
		allocInfo.level = static_cast<VkCommandBufferLevel>(level);
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer vkCommandBuffer;
		if (vkAllocateCommandBuffers(device_, &allocInfo, &vkCommandBuffer) != VK_SUCCESS)
		{
			return nullptr;
		}

		auto* commandBuffer = new VulkanCommandBuffer(device_, vkCommandBuffer, this);
		allocatedBuffers_.push_back(commandBuffer);
		return commandBuffer;
	}

	std::vector<VulkanCommandBuffer*> VulkanCommandPool::allocateCommandBuffers(uint32_t count, RHICommandBufferLevel level)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool_;
		allocInfo.level = static_cast<VkCommandBufferLevel>(level);
		allocInfo.commandBufferCount = count;

		std::vector<VkCommandBuffer> vkCommandBuffers(count);
		if (vkAllocateCommandBuffers(device_, &allocInfo, vkCommandBuffers.data()) != VK_SUCCESS)
		{
			return {};
		}

		std::vector<VulkanCommandBuffer*> commandBuffers;
		for (auto vkCmdBuffer : vkCommandBuffers)
		{
			auto* commandBuffer = new VulkanCommandBuffer(device_, vkCmdBuffer, this);
			allocatedBuffers_.push_back(commandBuffer);
			commandBuffers.push_back(commandBuffer);
		}

		return commandBuffers;
	}

} // namespace BinRenderer::Vulkan
