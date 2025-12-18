#pragma once

#include "../../Resources/RHIBuffer.h"
#include "../../Structs/RHIBufferCreateInfo.h"
#include <vulkan/vulkan.h>

namespace BinRenderer::Vulkan
{
	class VulkanDevice;

	/**
	 * @brief Vulkan 버퍼 구현
	 */
	class VulkanBuffer : public RHIBuffer
	{
	public:
		VulkanBuffer(VkDevice device, VkPhysicalDevice physicalDevice);
		~VulkanBuffer() override;

		bool create(const RHIBufferCreateInfo& createInfo);
		void destroy();

		// RHIBuffer 인터페이스 구현
		void* map() override;
		void unmap() override;
		void flush(RHIDeviceSize offset = 0, RHIDeviceSize size = 0);
		void updateData(const void* data, RHIDeviceSize size, RHIDeviceSize offset = 0) override;

		RHIDeviceSize getSize() const override { return size_; }
		RHIBufferUsageFlags getUsage() const override { return usage_; }

		// Vulkan 네이티브 접근
		VkBuffer getVkBuffer() const { return buffer_; }
		VkDeviceMemory getVkMemory() const { return memory_; }

	private:
		VkDevice device_;
		VkPhysicalDevice physicalDevice_;
		VkBuffer buffer_ = VK_NULL_HANDLE;
		VkDeviceMemory memory_ = VK_NULL_HANDLE;

		RHIDeviceSize size_ = 0;
		RHIBufferUsageFlags usage_ = 0;
		void* mappedPtr_ = nullptr;

		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	};

} // namespace BinRenderer::Vulkan
