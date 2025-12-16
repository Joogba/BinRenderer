#include "VulkanMemoryAllocator.h"
#include "Vulkan/Logger.h"

namespace BinRenderer::Vulkan
{
	VulkanMemoryAllocator::VulkanMemoryAllocator(VkDevice device, VkPhysicalDevice physicalDevice)
		: device_(device), physicalDevice_(physicalDevice)
	{
		vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &memoryProperties_);
	}

	VulkanMemoryAllocator::~VulkanMemoryAllocator()
	{
		// 남은 메모리 정리
		for (auto& pair : allocations_)
		{
			vkFreeMemory(device_, pair.first, nullptr);
		}
		allocations_.clear();
	}

	VkDeviceMemory VulkanMemoryAllocator::allocateBufferMemory(VkBuffer buffer, VkMemoryPropertyFlags properties)
	{
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device_, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		VkDeviceMemory memory;
		if (vkAllocateMemory(device_, &allocInfo, nullptr, &memory) != VK_SUCCESS)
		{
			return VK_NULL_HANDLE;
		}

		// 통계 업데이트
		stats_.totalAllocated += memRequirements.size;
		stats_.allocationCount++;
		allocations_[memory] = memRequirements.size;

		return memory;
	}

	VkDeviceMemory VulkanMemoryAllocator::allocateImageMemory(VkImage image, VkMemoryPropertyFlags properties)
	{
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device_, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		VkDeviceMemory memory;
		if (vkAllocateMemory(device_, &allocInfo, nullptr, &memory) != VK_SUCCESS)
		{
			return VK_NULL_HANDLE;
		}

		// 통계 업데이트
		stats_.totalAllocated += memRequirements.size;
		stats_.allocationCount++;
		allocations_[memory] = memRequirements.size;

		return memory;
	}

	void VulkanMemoryAllocator::freeMemory(VkDeviceMemory memory)
	{
		if (memory == VK_NULL_HANDLE)
		{
			return;
		}

		auto it = allocations_.find(memory);
		if (it != allocations_.end())
		{
			stats_.totalFreed += it->second;
			allocations_.erase(it);
		}

		vkFreeMemory(device_, memory, nullptr);
	}

	uint32_t VulkanMemoryAllocator::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		for (uint32_t i = 0; i < memoryProperties_.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) &&
				(memoryProperties_.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		exitWithMessage("Failed to find suitable memory type!");
		return 0;
	}

	void VulkanMemoryAllocator::resetStats()
	{
		stats_ = Stats{};
	}

} // namespace BinRenderer::Vulkan
