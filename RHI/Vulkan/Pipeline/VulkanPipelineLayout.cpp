#include "VulkanPipelineLayout.h"

namespace BinRenderer::Vulkan
{
	VulkanPipelineLayout::VulkanPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout)
		: device_(device), pipelineLayout_(pipelineLayout)
	{
	}

	VulkanPipelineLayout::~VulkanPipelineLayout()
	{
		if (pipelineLayout_ != VK_NULL_HANDLE)
		{
			vkDestroyPipelineLayout(device_, pipelineLayout_, nullptr);
			pipelineLayout_ = VK_NULL_HANDLE;
		}
	}

} // namespace BinRenderer::Vulkan
