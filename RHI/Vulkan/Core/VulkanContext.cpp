#include "VulkanContext.h"
#include "../Utilities/VulkanDebug.h"
#include "Vulkan/Logger.h"
#include <set>

namespace BinRenderer::Vulkan
{
	VulkanContext::VulkanContext()
	{
	}

	VulkanContext::~VulkanContext()
	{
		shutdown();
	}

	bool VulkanContext::initialize(const std::vector<const char*>& instanceExtensions, bool enableValidation, bool requireSwapchain)
	{
		validationEnabled_ = enableValidation;
		requireSwapchain_ = requireSwapchain;  // ✅ 저장

		if (!createInstance(instanceExtensions))
		{
			return false;
		}

		if (validationEnabled_)
		{
			setupDebugMessenger();
		}

		if (!pickPhysicalDevice())
		{
			return false;
		}

		if (!createLogicalDevice())
		{
			return false;
		}

		printLog("VulkanContext initialized successfully");
		return true;
	}

	void VulkanContext::shutdown()
	{
		if (device_ != VK_NULL_HANDLE)
		{
			vkDeviceWaitIdle(device_);
			vkDestroyDevice(device_, nullptr);
			device_ = VK_NULL_HANDLE;
		}

		if (validationEnabled_ && debugMessenger_ != VK_NULL_HANDLE)
		{
			VulkanDebug::destroyDebugMessenger(instance_, debugMessenger_);
			debugMessenger_ = VK_NULL_HANDLE;
		}

		if (instance_ != VK_NULL_HANDLE)
		{
			vkDestroyInstance(instance_, nullptr);
			instance_ = VK_NULL_HANDLE;
		}
	}

	void VulkanContext::waitIdle()
	{
		if (device_ != VK_NULL_HANDLE)
		{
			vkDeviceWaitIdle(device_);
		}
	}

	bool VulkanContext::createInstance(const std::vector<const char*>& extensions)
	{
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "BinRenderer RHI";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "BinRenderer";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3;

		std::vector<const char*> instanceExtensions = extensions;

		if (validationEnabled_)
		{
			auto debugExtensions = VulkanDebug::getRequiredExtensions(true);
			instanceExtensions.insert(instanceExtensions.end(), debugExtensions.begin(), debugExtensions.end());
		}

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
		createInfo.ppEnabledExtensionNames = instanceExtensions.data();

		const char* validationLayers[] = { "VK_LAYER_KHRONOS_validation" };
		if (validationEnabled_ && VulkanDebug::checkValidationLayerSupport(validationLayers, 1))
		{
			createInfo.enabledLayerCount = 1;
			createInfo.ppEnabledLayerNames = validationLayers;
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance_) != VK_SUCCESS)
		{
			printLog("Failed to create Vulkan instance!");
			return false;
		}

		return true;
	}

	bool VulkanContext::pickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			printLog("Failed to find GPUs with Vulkan support!");
			return false;
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());

		// 간단하게 첫 번째 디바이스 선택 (실제로는 점수 매기기 필요)
		physicalDevice_ = devices[0];

		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physicalDevice_, &properties);
		printLog("Selected GPU: {}", properties.deviceName);

		return true;
	}

	bool VulkanContext::createLogicalDevice()
	{
		auto indices = findQueueFamilies(physicalDevice_);

		std::set<uint32_t> uniqueQueueFamilies = {
			indices.graphicsFamily,
			indices.presentFamily,
			indices.computeFamily
		};

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		float queuePriority = 1.0f;

		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		// ✅ 헤드리스 모드 지원: 스왑체인이 필요할 때만 확장 추가
		std::vector<const char*> deviceExtensions;
		if (requireSwapchain_) {
			deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
			printLog("  Enabling device extension: VK_KHR_swapchain");
		} else {
			printLog("  Headless mode: Skipping VK_KHR_swapchain");
		}

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.empty() ? nullptr : deviceExtensions.data();

		if (vkCreateDevice(physicalDevice_, &createInfo, nullptr, &device_) != VK_SUCCESS)
		{
			printLog("Failed to create logical device!");
			return false;
		}

		// 큐 가져오기
		vkGetDeviceQueue(device_, indices.graphicsFamily, 0, &graphicsQueue_);
		vkGetDeviceQueue(device_, indices.presentFamily, 0, &presentQueue_);
		vkGetDeviceQueue(device_, indices.computeFamily, 0, &computeQueue_);

		graphicsQueueFamily_ = indices.graphicsFamily;
		presentQueueFamily_ = indices.presentFamily;
		computeQueueFamily_ = indices.computeFamily;

		return true;
	}

	bool VulkanContext::setupDebugMessenger()
	{
		return VulkanDebug::setupDebugMessenger(instance_, &debugMessenger_) == VK_SUCCESS;
	}

	VulkanContext::QueueFamilyIndices VulkanContext::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		uint32_t i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
				indices.presentFamily = i; // 일반적으로 그래픽스 큐가 프레젠트도 지원
			}

			if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				indices.computeFamily = i;
			}

			if (indices.isComplete())
			{
				break;
			}

			i++;
		}

		return indices;
	}

} // namespace BinRenderer::Vulkan
