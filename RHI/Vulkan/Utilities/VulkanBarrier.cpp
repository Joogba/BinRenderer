#include "VulkanBarrier.h"
#include "Vulkan/Logger.h"

namespace BinRenderer::Vulkan
{
	VulkanBarrier::VulkanBarrier(VkImage image, VkFormat format, uint32_t mipLevels, uint32_t arrayLayers)
		: image_(image)
		, format_(format)
		, mipLevels_(mipLevels)
		, arrayLayers_(arrayLayers)
	{
	}

	VulkanBarrier::VulkanBarrier(VulkanBarrier&& other) noexcept
		: image_(other.image_)
		, format_(other.format_)
		, mipLevels_(other.mipLevels_)
		, arrayLayers_(other.arrayLayers_)
		, currentLayout_(other.currentLayout_)
		, currentAccess_(other.currentAccess_)
		, currentStage_(other.currentStage_)
	{
		// Reset moved-from object
		other.image_ = VK_NULL_HANDLE;
		other.format_ = VK_FORMAT_UNDEFINED;
		other.mipLevels_ = 1;
		other.arrayLayers_ = 1;
		other.currentLayout_ = VK_IMAGE_LAYOUT_UNDEFINED;
		other.currentAccess_ = VK_ACCESS_2_NONE;
		other.currentStage_ = VK_PIPELINE_STAGE_2_NONE;
	}

	VulkanBarrier& VulkanBarrier::operator=(VulkanBarrier&& other) noexcept
	{
		if (this != &other)
		{
			image_ = other.image_;
			format_ = other.format_;
			mipLevels_ = other.mipLevels_;
			arrayLayers_ = other.arrayLayers_;
			currentLayout_ = other.currentLayout_;
			currentAccess_ = other.currentAccess_;
			currentStage_ = other.currentStage_;

			// Reset moved-from object
			other.image_ = VK_NULL_HANDLE;
			other.format_ = VK_FORMAT_UNDEFINED;
			other.mipLevels_ = 1;
			other.arrayLayers_ = 1;
			other.currentLayout_ = VK_IMAGE_LAYOUT_UNDEFINED;
			other.currentAccess_ = VK_ACCESS_2_NONE;
			other.currentStage_ = VK_PIPELINE_STAGE_2_NONE;
		}
		return *this;
	}

	void VulkanBarrier::setImage(VkImage image, VkFormat format, uint32_t mipLevels, uint32_t arrayLayers)
	{
		image_ = image;
		format_ = format;
		mipLevels_ = mipLevels;
		arrayLayers_ = arrayLayers;
	}

	void VulkanBarrier::transitionLayout(
		VkCommandBuffer cmd,
		VkImageLayout newLayout,
		VkAccessFlags2 newAccess,
		VkPipelineStageFlags2 newStage,
		uint32_t baseMipLevel,
		uint32_t levelCount,
		uint32_t baseArrayLayer,
		uint32_t layerCount)
	{
		// Validate image
		if (image_ == VK_NULL_HANDLE)
		{
			printLog("ERROR: Cannot transition invalid image");
			return;
		}

		// Resolve VK_REMAINING_* values
		uint32_t actualLevelCount = (levelCount == VK_REMAINING_MIP_LEVELS) 
			? (mipLevels_ - baseMipLevel) 
			: levelCount;
		uint32_t actualLayerCount = (layerCount == VK_REMAINING_ARRAY_LAYERS)
			? (arrayLayers_ - baseArrayLayer)
			: layerCount;

		// Skip redundant transitions
		if (currentLayout_ == newLayout && 
			currentAccess_ == newAccess && 
			baseMipLevel == 0 &&
			actualLevelCount == mipLevels_ && 
			baseArrayLayer == 0 &&
			actualLayerCount == arrayLayers_)
		{
			return;
		}

		// Validate transition
		if (!isValidTransition(currentLayout_, newLayout))
		{
			printLog("WARNING: Invalid layout transition {} -> {}", 
				static_cast<int>(currentLayout_), 
				static_cast<int>(newLayout));
		}

		// Create barrier
		VkImageMemoryBarrier2 barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
		barrier.srcStageMask = (currentStage_ != VK_PIPELINE_STAGE_2_NONE)
			? currentStage_
			: VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
		barrier.dstStageMask = newStage;
		barrier.srcAccessMask = currentAccess_;
		barrier.dstAccessMask = newAccess;
		barrier.oldLayout = currentLayout_;
		barrier.newLayout = newLayout;
		barrier.image = image_;
		barrier.subresourceRange.aspectMask = getAspectMask();
		barrier.subresourceRange.baseMipLevel = baseMipLevel;
		barrier.subresourceRange.levelCount = actualLevelCount;
		barrier.subresourceRange.baseArrayLayer = baseArrayLayer;
		barrier.subresourceRange.layerCount = actualLayerCount;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		VkDependencyInfo depInfo{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
		depInfo.imageMemoryBarrierCount = 1;
		depInfo.pImageMemoryBarriers = &barrier;

		vkCmdPipelineBarrier2(cmd, &depInfo);

		// Update state only if transitioning the entire image
		if (baseMipLevel == 0 && actualLevelCount == mipLevels_ &&
			baseArrayLayer == 0 && actualLayerCount == arrayLayers_)
		{
			currentLayout_ = newLayout;
			currentAccess_ = newAccess;
			currentStage_ = newStage;
		}
	}

	VkImageMemoryBarrier2 VulkanBarrier::prepareBarrier(
		VkImageLayout targetLayout,
		VkAccessFlags2 targetAccess,
		VkPipelineStageFlags2 targetStage)
	{
		VkImageMemoryBarrier2 barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
		barrier.srcStageMask = (currentStage_ != VK_PIPELINE_STAGE_2_NONE)
			? currentStage_
			: VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
		barrier.dstStageMask = targetStage;
		barrier.srcAccessMask = currentAccess_;
		barrier.dstAccessMask = targetAccess;
		barrier.oldLayout = currentLayout_;
		barrier.newLayout = targetLayout;
		barrier.image = image_;
		barrier.subresourceRange.aspectMask = getAspectMask();
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels_;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = arrayLayers_;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		// Update state
		currentLayout_ = targetLayout;
		currentAccess_ = targetAccess;
		currentStage_ = targetStage;

		return barrier;
	}

	// ========================================
	// 편의 메서드 구현
	// ========================================

	void VulkanBarrier::transitionToTransferDst(VkCommandBuffer cmd)
	{
		transitionLayout(
			cmd,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_ACCESS_2_TRANSFER_WRITE_BIT,
			VK_PIPELINE_STAGE_2_TRANSFER_BIT
		);
	}

	void VulkanBarrier::transitionToShaderReadOnly(VkCommandBuffer cmd)
	{
		transitionLayout(
			cmd,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_2_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT
		);
	}

	void VulkanBarrier::transitionToColorAttachment(VkCommandBuffer cmd)
	{
		transitionLayout(
			cmd,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
		);
	}

	void VulkanBarrier::transitionToDepthAttachment(VkCommandBuffer cmd)
	{
		transitionLayout(
			cmd,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT
		);
	}

	void VulkanBarrier::transitionColorToShaderRead(VkCommandBuffer cmd)
	{
		transitionLayout(
			cmd,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_2_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT
		);
	}

	void VulkanBarrier::transitionShaderReadToColor(VkCommandBuffer cmd)
	{
		transitionLayout(
			cmd,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
		);
	}

	void VulkanBarrier::transitionColorToPresent(VkCommandBuffer cmd)
	{
		transitionLayout(
			cmd,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_ACCESS_2_NONE,
			VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT
		);
	}

	// ========================================
	// Private 메서드
	// ========================================

	VkImageAspectFlags VulkanBarrier::getAspectMask() const
	{
		// Depth/Stencil formats
		if (format_ >= VK_FORMAT_D16_UNORM && format_ <= VK_FORMAT_D32_SFLOAT_S8_UINT)
		{
			// Depth + Stencil
			if (format_ == VK_FORMAT_D32_SFLOAT_S8_UINT || format_ == VK_FORMAT_D24_UNORM_S8_UINT ||
				format_ == VK_FORMAT_D16_UNORM_S8_UINT)
			{
				return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			}
			// Depth only
			return VK_IMAGE_ASPECT_DEPTH_BIT;
		}

		// Color format
		return VK_IMAGE_ASPECT_COLOR_BIT;
	}

	bool VulkanBarrier::isValidTransition(VkImageLayout oldLayout, VkImageLayout newLayout) const
	{
		// UNDEFINED can transition to anything
		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED)
		{
			return true;
		}

		// Same layout is always valid (no-op)
		if (oldLayout == newLayout)
		{
			return true;
		}

		// PREINITIALIZED can only transition to GENERAL or TRANSFER_DST
		if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED)
		{
			return newLayout == VK_IMAGE_LAYOUT_GENERAL ||
				newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		}

		// All other transitions are valid
		return true;
	}

	// ========================================
	// 전역 헬퍼 함수 구현
	// ========================================

	namespace BarrierHelpers
	{
		void transitionImageLayout(
			VkCommandBuffer cmd,
			VkImage image,
			VkFormat format,
			VkImageLayout oldLayout,
			VkImageLayout newLayout,
			uint32_t mipLevels,
			uint32_t arrayLayers)
		{
			// Get aspect mask
			VkImageAspectFlags aspectMask = getImageAspect(format);

			// Get access info for layouts
			auto oldInfo = getLayoutAccessInfo(oldLayout);
			auto newInfo = getLayoutAccessInfo(newLayout);

			// Create barrier
			VkImageMemoryBarrier2 barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
			barrier.srcStageMask = oldInfo.stageMask;
			barrier.dstStageMask = newInfo.stageMask;
			barrier.srcAccessMask = oldInfo.accessMask;
			barrier.dstAccessMask = newInfo.accessMask;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.image = image;
			barrier.subresourceRange.aspectMask = aspectMask;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = mipLevels;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = arrayLayers;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			VkDependencyInfo depInfo{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
			depInfo.imageMemoryBarrierCount = 1;
			depInfo.pImageMemoryBarriers = &barrier;

			vkCmdPipelineBarrier2(cmd, &depInfo);
		}

		VkImageAspectFlags getImageAspect(VkFormat format)
		{
			// Depth/Stencil formats
			if (format >= VK_FORMAT_D16_UNORM && format <= VK_FORMAT_D32_SFLOAT_S8_UINT)
			{
				// Depth + Stencil
				if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT ||
					format == VK_FORMAT_D16_UNORM_S8_UINT)
				{
					return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
				}
				// Depth only
				return VK_IMAGE_ASPECT_DEPTH_BIT;
			}

			// Color format
			return VK_IMAGE_ASPECT_COLOR_BIT;
		}

		LayoutAccessInfo getLayoutAccessInfo(VkImageLayout layout)
		{
			LayoutAccessInfo info{};

			switch (layout)
			{
			case VK_IMAGE_LAYOUT_UNDEFINED:
				info.accessMask = VK_ACCESS_2_NONE;
				info.stageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
				break;

			case VK_IMAGE_LAYOUT_GENERAL:
				info.accessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
				info.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
				break;

			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				info.accessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
				info.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
				break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				info.accessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				info.stageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
				break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
				info.accessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
				info.stageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
				break;

			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				info.accessMask = VK_ACCESS_2_SHADER_READ_BIT;
				info.stageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				info.accessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
				info.stageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				info.accessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
				info.stageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
				break;

			case VK_IMAGE_LAYOUT_PREINITIALIZED:
				info.accessMask = VK_ACCESS_2_HOST_WRITE_BIT;
				info.stageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
				break;

			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
				info.accessMask = VK_ACCESS_2_NONE;
				info.stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
				break;

			default:
				info.accessMask = VK_ACCESS_2_NONE;
				info.stageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
				break;
			}

			return info;
		}
	}

} // namespace BinRenderer::Vulkan
