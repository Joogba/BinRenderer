#pragma once

#include "../../Core/RHIType.h"
#include <vulkan/vulkan.h>

namespace BinRenderer::Vulkan
{
	/**
	 * @brief RHI 타입과 Vulkan 타입 간 변환
  */
	class TypeConversion
	{
	public:
		// Format 변환
		static VkFormat toVkFormat(RHIFormat format);
		static RHIFormat fromVkFormat(VkFormat format);

		// Buffer Usage 변환
		static VkBufferUsageFlags toVkBufferUsage(RHIBufferUsageFlags usage);

		// Image Usage 변환
		static VkImageUsageFlags toVkImageUsage(RHIImageUsageFlags usage);

		// Memory Properties 변환
		static VkMemoryPropertyFlags toVkMemoryProperties(RHIMemoryPropertyFlags props);

		// Shader Stage 변환
		static VkShaderStageFlags toVkShaderStage(RHIShaderStageFlags stage);

		// Sample Count 변환
		static VkSampleCountFlagBits toVkSampleCount(RHISampleCountFlagBits samples);

		// Image Tiling 변환
		static VkImageTiling toVkImageTiling(RHIImageTiling tiling);

	private:
		TypeConversion() = delete;
	};

} // namespace BinRenderer::Vulkan
