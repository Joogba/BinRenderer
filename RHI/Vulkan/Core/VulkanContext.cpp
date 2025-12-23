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

		// ✅ 디바이스 속성 저장
		vkGetPhysicalDeviceProperties(physicalDevice_, &deviceProperties_);
		vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &memoryProperties_);
		vkGetPhysicalDeviceFeatures(physicalDevice_, &deviceFeatures_);

		printLog("Selected GPU: {}", deviceProperties_.deviceName);

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

		// ✅ Vulkan 1.2 Features: Bindless Descriptor Arrays (non-uniform indexing)
		VkPhysicalDeviceVulkan12Features vulkan12Features{};
		vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		vulkan12Features.descriptorIndexing = VK_TRUE;
		vulkan12Features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
		vulkan12Features.runtimeDescriptorArray = VK_TRUE;
		vulkan12Features.descriptorBindingVariableDescriptorCount = VK_TRUE;
		vulkan12Features.descriptorBindingPartiallyBound = VK_TRUE;

		// ✅ Vulkan 1.3 Features: Dynamic Rendering & Synchronization2
		VkPhysicalDeviceSynchronization2Features sync2Features{};
		sync2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
		sync2Features.synchronization2 = VK_TRUE;
		sync2Features.pNext = &vulkan12Features;  // Chain to Vulkan 1.2 features

		VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
		dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
		dynamicRenderingFeatures.dynamicRendering = VK_TRUE;
		dynamicRenderingFeatures.pNext = &sync2Features;

		VkPhysicalDeviceFeatures2 deviceFeatures2{};
		deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		deviceFeatures2.features.samplerAnisotropy = VK_TRUE;
		deviceFeatures2.pNext = &dynamicRenderingFeatures;


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
		createInfo.pEnabledFeatures = nullptr; // ✅ deviceFeatures2 사용 시 nullptr
		createInfo.pNext = &deviceFeatures2;   // ✅ Feature chain 연결
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

		printLog("✅ Vulkan features enabled:");
		printLog("   - Dynamic Rendering (1.3)");
		printLog("   - Synchronization2 (1.3)");
		printLog("   - Descriptor Indexing (1.2)");
		printLog("   - Bindless Descriptor Arrays (1.2)");

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

	// ========================================
	// ✅ 헬퍼 메서드 구현
	// ========================================

	uint32_t VulkanContext::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
	{
		for (uint32_t i = 0; i < memoryProperties_.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) &&
				(memoryProperties_.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		printLog("ERROR: Failed to find suitable memory type!");
		return 0;
	}

	VkFormat VulkanContext::findSupportedFormat(
		const std::vector<VkFormat>& candidates,
		VkImageTiling tiling,
		VkFormatFeatureFlags features) const
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice_, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		printLog("ERROR: Failed to find supported format!");
		return VK_FORMAT_UNDEFINED;
	}

	VkFormat VulkanContext::findDepthFormat() const
	{
		std::vector<VkFormat> candidates = {
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT
		};

		return findSupportedFormat(
			candidates,
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	VkSampleCountFlagBits VulkanContext::getMaxUsableSampleCount() const
	{
		VkSampleCountFlags counts =
			deviceProperties_.limits.framebufferColorSampleCounts &
			deviceProperties_.limits.framebufferDepthSampleCounts;

		if (counts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
		if (counts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
		if (counts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
		if (counts & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
		if (counts & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
		if (counts & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;

		return VK_SAMPLE_COUNT_1_BIT;
	}

} // namespace BinRenderer::Vulkan
