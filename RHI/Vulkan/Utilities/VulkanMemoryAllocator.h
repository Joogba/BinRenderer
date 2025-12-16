#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>

namespace BinRenderer::Vulkan
{
	/**
	 * @brief 간단한 Vulkan 메모리 할당자
	 *
 * 실제 프로젝트에서는 VMA (Vulkan Memory Allocator) 사용 권장
	 */
	class VulkanMemoryAllocator
	{
	public:
		VulkanMemoryAllocator(VkDevice device, VkPhysicalDevice physicalDevice);
		~VulkanMemoryAllocator();

		// 버퍼 메모리 할당
		VkDeviceMemory allocateBufferMemory(VkBuffer buffer, VkMemoryPropertyFlags properties);

		// 이미지 메모리 할당
		VkDeviceMemory allocateImageMemory(VkImage image, VkMemoryPropertyFlags properties);

		// 메모리 해제
		void freeMemory(VkDeviceMemory memory);

		// 메모리 타입 찾기
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		// 통계
		struct Stats
		{
			uint64_t totalAllocated = 0;
			uint64_t totalFreed = 0;
			uint32_t allocationCount = 0;
		};

		Stats getStats() const { return stats_; }
		void resetStats();

	private:
		VkDevice device_;
		VkPhysicalDevice physicalDevice_;
		VkPhysicalDeviceMemoryProperties memoryProperties_;

		Stats stats_;
		std::unordered_map<VkDeviceMemory, VkDeviceSize> allocations_;
	};

} // namespace BinRenderer::Vulkan
