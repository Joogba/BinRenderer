#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <functional>

namespace BinRenderer::Vulkan
{
	/**
	 * @brief Vulkan 디버그 유틸리티
  */
	class VulkanDebug
	{
	public:
		using DebugCallback = std::function<void(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
			VkDebugUtilsMessageTypeFlagsEXT type,
			const char* message)>;

		// 디버그 메신저 설정
		static VkResult setupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* debugMessenger);
		static void destroyDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger);

		// 커스텀 콜백 설정
		static void setDebugCallback(DebugCallback callback);

		// 객체 이름 설정 (디버깅용)
		static void setObjectName(VkDevice device, uint64_t object, VkObjectType objectType, const char* name);

		// 커맨드 버퍼 레이블
		static void beginCommandBufferLabel(VkCommandBuffer commandBuffer, const char* labelName, float r = 1.0f, float g = 1.0f, float b = 1.0f);
		static void endCommandBufferLabel(VkCommandBuffer commandBuffer);

		// 큐 레이블
		static void beginQueueLabel(VkQueue queue, const char* labelName, float r = 1.0f, float g = 1.0f, float b = 1.0f);
		static void endQueueLabel(VkQueue queue);

		// 검증 레이어 지원 확인
		static bool checkValidationLayerSupport(const char** layerNames, uint32_t layerCount);

		// 필요한 확장 가져오기
		static std::vector<const char*> getRequiredExtensions(bool enableValidationLayers);

	private:
		static DebugCallback s_debugCallback;

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);

		static VkResult createDebugUtilsMessengerEXT(VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugUtilsMessengerEXT* pDebugMessenger);

		static void destroyDebugUtilsMessengerEXT(VkInstance instance,
			VkDebugUtilsMessengerEXT debugMessenger,
			const VkAllocationCallbacks* pAllocator);
	};

} // namespace BinRenderer::Vulkan
