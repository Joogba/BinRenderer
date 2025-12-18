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
		bool initialize(const std::vector<const char*>& instanceExtensions, bool enableValidation = true, bool requireSwapchain = true);
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

		// ========================================
		// ✅ 헬퍼 메서드 (레거시 Context에서 마이그레이션)
		// ========================================

		/**
		 * @brief 메모리 타입 인덱스 찾기
		 * @param typeFilter 메모리 타입 필터
		 * @param properties 필요한 메모리 속성
		 * @return 메모리 타입 인덱스
		 */
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

		/**
		 * @brief 지원되는 포맷 찾기
		 * @param candidates 후보 포맷 리스트
		 * @param tiling 타일링 모드
		 * @param features 필요한 포맷 기능
		 * @return 지원되는 포맷
		 */
		VkFormat findSupportedFormat(
			const std::vector<VkFormat>& candidates,
			VkImageTiling tiling,
			VkFormatFeatureFlags features) const;

		/**
		 * @brief Depth 포맷 찾기 (자동 선택)
		 * @return Depth/Stencil 포맷
		 */
		VkFormat findDepthFormat() const;

		/**
		 * @brief 최대 사용 가능한 MSAA 샘플 개수
		 * @return 샘플 개수
		 */
		VkSampleCountFlagBits getMaxUsableSampleCount() const;

		/**
		 * @brief 물리 디바이스 속성 접근
		 */
		const VkPhysicalDeviceProperties& getDeviceProperties() const { return deviceProperties_; }
		const VkPhysicalDeviceMemoryProperties& getMemoryProperties() const { return memoryProperties_; }
		const VkPhysicalDeviceFeatures& getDeviceFeatures() const { return deviceFeatures_; }

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
		bool requireSwapchain_ = true;  // ✅ 추가: 스왑체인 필요 여부

		// ✅ 디바이스 속성 저장
		VkPhysicalDeviceProperties deviceProperties_{};
		VkPhysicalDeviceMemoryProperties memoryProperties_{};
		VkPhysicalDeviceFeatures deviceFeatures_{};

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
