#include "VulkanDebug.h"
#include "Vulkan/Logger.h"
#include <iostream>
#include <vector>

namespace BinRenderer::Vulkan
{
	VulkanDebug::DebugCallback VulkanDebug::s_debugCallback = nullptr;

	VkResult VulkanDebug::setupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* debugMessenger)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;

		return createDebugUtilsMessengerEXT(instance, &createInfo, nullptr, debugMessenger);
	}

	void VulkanDebug::destroyDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger)
	{
		destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	void VulkanDebug::setDebugCallback(DebugCallback callback)
	{
		s_debugCallback = callback;
	}

	void VulkanDebug::setObjectName(VkDevice device, uint64_t object, VkObjectType objectType, const char* name)
	{
		VkDebugUtilsObjectNameInfoEXT nameInfo{};
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		nameInfo.objectType = objectType;
		nameInfo.objectHandle = object;
		nameInfo.pObjectName = name;

		auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
		if (func != nullptr)
		{
			func(device, &nameInfo);
		}
	}

	void VulkanDebug::beginCommandBufferLabel(VkCommandBuffer commandBuffer, const char* labelName, float r, float g, float b)
	{
		VkDebugUtilsLabelEXT labelInfo{};
		labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
		labelInfo.pLabelName = labelName;
		labelInfo.color[0] = r;
		labelInfo.color[1] = g;
		labelInfo.color[2] = b;
		labelInfo.color[3] = 1.0f;

		auto func = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkCmdBeginDebugUtilsLabelEXT");
		if (func != nullptr)
		{
			func(commandBuffer, &labelInfo);
		}
	}

	void VulkanDebug::endCommandBufferLabel(VkCommandBuffer commandBuffer)
	{
		auto func = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkCmdEndDebugUtilsLabelEXT");
		if (func != nullptr)
		{
			func(commandBuffer);
		}
	}

	void VulkanDebug::beginQueueLabel(VkQueue queue, const char* labelName, float r, float g, float b)
	{
		VkDebugUtilsLabelEXT labelInfo{};
		labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
		labelInfo.pLabelName = labelName;
		labelInfo.color[0] = r;
		labelInfo.color[1] = g;
		labelInfo.color[2] = b;
		labelInfo.color[3] = 1.0f;

		auto func = (PFN_vkQueueBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkQueueBeginDebugUtilsLabelEXT");
		if (func != nullptr)
		{
			func(queue, &labelInfo);
		}
	}

	void VulkanDebug::endQueueLabel(VkQueue queue)
	{
		auto func = (PFN_vkQueueEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkQueueEndDebugUtilsLabelEXT");
		if (func != nullptr)
		{
			func(queue);
		}
	}

	bool VulkanDebug::checkValidationLayerSupport(const char** layerNames, uint32_t layerCount)
	{
		uint32_t availableLayerCount;
		vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(availableLayerCount);
		vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers.data());

		for (uint32_t i = 0; i < layerCount; i++)
		{
			bool layerFound = false;
			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerNames[i], layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	std::vector<const char*> VulkanDebug::getRequiredExtensions(bool enableValidationLayers)
	{
		std::vector<const char*> extensions;

		if (enableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebug::debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		// ✅ printLog()를 사용하여 파일에도 기록되도록 수정
		const char* severityStr = "";
		const char* typeStr = "";

		// Severity 문자열
		if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			severityStr = "❌ ERROR";
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			severityStr = "⚠️  WARNING";
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
			severityStr = "ℹ️  INFO";
		else
			severityStr = "🔍 VERBOSE";

		// Type 문자열
		if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
			typeStr = "[Validation]";
		else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
			typeStr = "[Performance]";
		else
			typeStr = "[General]";

		// ✅ printLog() 사용 (파일 + 콘솔 출력)
		printLog("{} {} {}", severityStr, typeStr, pCallbackData->pMessage);

		// Custom callback 호출
		if (s_debugCallback)
		{
			s_debugCallback(messageSeverity, messageType, pCallbackData->pMessage);
		}

		return VK_FALSE;
	}

	VkResult VulkanDebug::createDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void VulkanDebug::destroyDebugUtilsMessengerEXT(VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}

} // namespace BinRenderer::Vulkan
