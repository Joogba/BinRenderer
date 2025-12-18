#include "VulkanPipelineLayout.h"

namespace BinRenderer::Vulkan
{
	VulkanPipelineLayout::VulkanPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout)
		: device_(device), pipelineLayout_(pipelineLayout)
	{
	}

	VulkanPipelineLayout::~VulkanPipelineLayout()
	{
		// 파이프라인 레이아웃은 VulkanPipeline에서 관리되므로 여기서는 삭제하지 않음
		// 만약 독립적으로 생성된 경우라면:
		// if (pipelineLayout_ != VK_NULL_HANDLE)
		// {
   //     vkDestroyPipelineLayout(device_, pipelineLayout_, nullptr);
		// }
	}

} // namespace BinRenderer::Vulkan
