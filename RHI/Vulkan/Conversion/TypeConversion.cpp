#include "TypeConversion.h"

namespace BinRenderer::Vulkan
{
	VkFormat TypeConversion::toVkFormat(RHIFormat format)
	{
		return static_cast<VkFormat>(format);
	}

	RHIFormat TypeConversion::fromVkFormat(VkFormat format)
	{
		return static_cast<RHIFormat>(format);
	}

	VkBufferUsageFlags TypeConversion::toVkBufferUsage(RHIBufferUsageFlags usage)
	{
		return static_cast<VkBufferUsageFlags>(usage);
	}

	VkImageUsageFlags TypeConversion::toVkImageUsage(RHIImageUsageFlags usage)
	{
		return static_cast<VkImageUsageFlags>(usage);
	}

	VkMemoryPropertyFlags TypeConversion::toVkMemoryProperties(RHIMemoryPropertyFlags props)
	{
		return static_cast<VkMemoryPropertyFlags>(props);
	}

	VkShaderStageFlags TypeConversion::toVkShaderStage(RHIShaderStageFlags stage)
	{
		return static_cast<VkShaderStageFlags>(stage);
	}

	VkSampleCountFlagBits TypeConversion::toVkSampleCount(RHISampleCountFlagBits samples)
	{
		return static_cast<VkSampleCountFlagBits>(samples);
	}

	VkImageTiling TypeConversion::toVkImageTiling(RHIImageTiling tiling)
	{
		return static_cast<VkImageTiling>(tiling);
	}

} // namespace BinRenderer::Vulkan
