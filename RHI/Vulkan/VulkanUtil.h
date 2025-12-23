#pragma once

#include <vulkan/vulkan.h>
#include <unordered_map>
#include <vector>
#include "RHI/Core/RHI.h"

// ========================================
// ✅ RHI Type Conversion Guidelines
// ========================================
// 
// RHI enum values are aligned with Vulkan for zero-cost conversion.
// Direct static_cast is safe and recommended:
// 
//   VkFormat vkFormat = static_cast<VkFormat>(rhiFormat);
//   VkImageViewType vkViewType = static_cast<VkImageViewType>(rhiViewType);
// 
// No conversion utility needed for Vulkan backend!
// DX12/Metal backends should implement their own conversion if needed.
// ========================================

namespace BinRenderer::Vulkan
{
	class VulkanUtil
	{
	public:
		static uint32_t			findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
		static VkShaderModule	createShaderModule(VkDevice device, const std::vector<char>& code);
		static void				createBuffer(VkDevice device,
										VkPhysicalDevice physicalDevice,
										VkDeviceSize size,
										VkBufferUsageFlags usage,
										VkMemoryPropertyFlags properties,
										VkBuffer& buffer,
			VkDeviceMemory& bufferMemory);
		static void				copyBuffer(VkDevice device,
									VkCommandPool commandPool,
									VkQueue graphicsQueue,
									VkBuffer srcBuffer,
									VkBuffer dstBuffer,
			VkDeviceSize size);
		static void				createBufferAndInitialize(VkDevice device,
											 VkPhysicalDevice physicalDevice,
											 VkCommandPool commandPool,
											 VkQueue graphicsQueue,
											 VkDeviceSize size,
											 VkBufferUsageFlags usage,
											 VkMemoryPropertyFlags properties,
											 const void* data,
											 VkBuffer& buffer,
			VkDeviceMemory& bufferMemory);
		static void 			createImage(VkDevice device,
									 VkPhysicalDevice physicalDevice,
									 uint32_t width,
									 uint32_t height,
									 VkFormat format,
									 VkImageTiling tiling,
									 VkImageUsageFlags usage,
									 VkMemoryPropertyFlags properties,
									 VkImage& image,
			VkDeviceMemory& imageMemory);	
		static VkImageView		createImageView(VkDevice device,
										 VkImage image,
										 VkFormat format,
			VkImageAspectFlags aspectFlags);
		static void 			transitionImageLayout(VkDevice device,
											 VkCommandPool commandPool,
											 VkQueue graphicsQueue,
											 VkImage image,
											 VkFormat format,
											 VkImageLayout oldLayout,
			VkImageLayout newLayout);	

		static void				genMipmappedImage(RHI* rhi, VkImage image, uint32_t width, uint32_t height, uint32_t mip_levels);

		static VkSampler		getOrCreateMipmapSampler(VkPhysicalDevice physical_device, VkDevice device, uint32_t width, uint32_t height);
		static void				destroyMipmappedSampler(VkDevice device);
		static VkSampler		getOrCreateNearestSampler(VkPhysicalDevice physical_device, VkDevice device);
		static VkSampler		getOrCreateLinearSampler(VkPhysicalDevice physical_device, VkDevice device);
		static void				destroyNearestSampler(VkDevice device);
		static void				destroyLinearSampler(VkDevice device);

	private:
		static std::unordered_map<uint32_t, VkSampler>	m_mipmap_sampler_map;
		static VkSampler								m_nearest_sampler;
		static VkSampler								m_linear_sampler;
	};
}