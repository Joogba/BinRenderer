#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

namespace BinRenderer::Vulkan
{
	/**
	 * @brief Vulkan 컨텍스트 (디바이스, 인스턴스, 큐 관리)
	 */
	class VulkanContext
	{
	public:
		VulkanContext();
		~VulkanContext();

		// 초기화
		bool initialize(const std::vector<const char*>& instanceExtensions, bool enableValidation = true);
		void shutdown();

		// 디바이스 및 큐 접근
		VkInstance getInstance() const { return instance_; }
		VkPhysicalDevice getPhysicalDevice() const { return physicalDevice_; }
		VkDevice getDevice() const { return device_; }

		VkQueue getGraphicsQueue() const { return graphicsQueue_; }
		VkQueue getPresentQueue() const { return presentQueue_; }
		VkQueue getComputeQueue() const { return computeQueue_; }

		uint32_t getGraphicsQueueFamily() const { return graphicsQueueFamily_; }
		uint32_t getPresentQueueFamily() const { return presentQueueFamily_; }
		uint32_t getComputeQueueFamily() const { return computeQueueFamily_; }

		// 유틸리티
		void waitIdle();

	private:
		VkInstance instance_ = VK_NULL_HANDLE;
		VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
		VkDevice device_ = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT debugMessenger_ = VK_NULL_HANDLE;

		VkQueue graphicsQueue_ = VK_NULL_HANDLE;
		VkQueue presentQueue_ = VK_NULL_HANDLE;
		VkQueue computeQueue_ = VK_NULL_HANDLE;

		uint32_t graphicsQueueFamily_ = 0;
		uint32_t presentQueueFamily_ = 0;
		uint32_t computeQueueFamily_ = 0;

		bool validationEnabled_ = false;

		// 초기화 헬퍼
		bool createInstance(const std::vector<const char*>& extensions);
		bool pickPhysicalDevice();
		bool createLogicalDevice();
		bool setupDebugMessenger();

		// 큐 패밀리 찾기
		struct QueueFamilyIndices
		{
			uint32_t graphicsFamily = UINT32_MAX;
			uint32_t presentFamily = UINT32_MAX;
			uint32_t computeFamily = UINT32_MAX;

			bool isComplete() const {
				return graphicsFamily != UINT32_MAX &&
					presentFamily != UINT32_MAX &&
					computeFamily != UINT32_MAX;
			}
		};

		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface = VK_NULL_HANDLE);
	};

} // namespace BinRenderer::Vulkan
