#include "VulkanBuffer.h"
#include "Core/Logger.h"
#include <cstring>

namespace BinRenderer::Vulkan
{
	VulkanBuffer::VulkanBuffer(VkDevice device, VkPhysicalDevice physicalDevice)
		: device_(device), physicalDevice_(physicalDevice)
	{
	}

	VulkanBuffer::~VulkanBuffer()
	{
		destroy();
	}

	bool VulkanBuffer::create(const RHIBufferCreateInfo& createInfo)
	{
		size_ = createInfo.size;
		usage_ = createInfo.usage;

		//  RHI usage flags를 Vulkan usage flags로 변환
		VkBufferUsageFlags vkUsage = 0;
		if (createInfo.usage & RHI_BUFFER_USAGE_VERTEX_BUFFER_BIT)
			vkUsage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		if (createInfo.usage & RHI_BUFFER_USAGE_INDEX_BUFFER_BIT)
			vkUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		if (createInfo.usage & RHI_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
			vkUsage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		if (createInfo.usage & RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT)
			vkUsage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		if (createInfo.usage & RHI_BUFFER_USAGE_TRANSFER_SRC_BIT)
			vkUsage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		if (createInfo.usage & RHI_BUFFER_USAGE_TRANSFER_DST_BIT)
			vkUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		// Vulkan 버퍼 생성 정보
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = createInfo.size;
		bufferInfo.usage = vkUsage; //  변환된 usage 사용
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device_, &bufferInfo, nullptr, &buffer_) != VK_SUCCESS)
		{
			return false;
		}

		// 메모리 요구사항 가져오기
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device_, buffer_, &memRequirements);

		// 메모리 할당
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(
			memRequirements.memoryTypeBits,
			static_cast<VkMemoryPropertyFlags>(createInfo.memoryProperties)
		);

		if (vkAllocateMemory(device_, &allocInfo, nullptr, &memory_) != VK_SUCCESS)
		{
			vkDestroyBuffer(device_, buffer_, nullptr);
			buffer_ = VK_NULL_HANDLE;
			return false;
		}

		// 버퍼에 메모리 바인딩
		vkBindBufferMemory(device_, buffer_, memory_, 0);

		// 초기 데이터 복사
		if (createInfo.initialData)
		{
			void* data = map();
			memcpy(data, createInfo.initialData, createInfo.size);
			unmap();
		}

		return true;
	}

	void VulkanBuffer::destroy()
	{
		if (mappedPtr_)
		{
			unmap();
		}

		if (buffer_ != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(device_, buffer_, nullptr);
			buffer_ = VK_NULL_HANDLE;
		}

		if (memory_ != VK_NULL_HANDLE)
		{
			vkFreeMemory(device_, memory_, nullptr);
			memory_ = VK_NULL_HANDLE;
		}
	}

	void* VulkanBuffer::map()
	{
		if (!mappedPtr_)
		{
			vkMapMemory(device_, memory_, 0, size_, 0, &mappedPtr_);
		}
		return mappedPtr_;
	}

	void VulkanBuffer::unmap()
	{
		if (mappedPtr_)
		{
			vkUnmapMemory(device_, memory_);
			mappedPtr_ = nullptr;
		}
	}

	void VulkanBuffer::flush(RHIDeviceSize offset, RHIDeviceSize size)
	{
		VkMappedMemoryRange mappedRange{};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory_;
		mappedRange.offset = offset;
		mappedRange.size = (size == 0) ? VK_WHOLE_SIZE : size;
		vkFlushMappedMemoryRanges(device_, 1, &mappedRange);
	}

	void VulkanBuffer::updateData(const void* data, RHIDeviceSize size, RHIDeviceSize offset)
	{
		void* mapped = map();
		memcpy(static_cast<char*>(mapped) + offset, data, size);
		unmap();
	}

	uint32_t VulkanBuffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		exitWithMessage("Failed to find suitable memory type!");
		return 0;
	}

} // namespace BinRenderer::Vulkan
