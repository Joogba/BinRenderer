#include "VulkanPipeline.h"
#include "../Resources/VulkanShader.h"
#include "VulkanRenderPass.h"
#include "VulkanDescriptor.h"
#include "../../Structs/RHIStructs.h"
#include "Core/Logger.h"

namespace BinRenderer::Vulkan
{
	VulkanPipeline::VulkanPipeline(VkDevice device, VkPipeline pipeline, RHIPipelineLayout* layout, RHIPipelineBindPoint bindPoint)
		: device_(device), pipeline_(pipeline), layout_(layout), bindPoint_(bindPoint)
	{
	}

	VulkanPipeline::~VulkanPipeline()
	{
		destroy();
	}

	void VulkanPipeline::destroy()
	{
		if (pipeline_ != VK_NULL_HANDLE)
		{
			vkDestroyPipeline(device_, pipeline_, nullptr);
			pipeline_ = VK_NULL_HANDLE;
		}
	}

	VkPipelineLayout VulkanPipeline::getVkPipelineLayout() const
	{
		if (layout_)
		{
			return static_cast<VulkanPipelineLayout*>(layout_)->getVkPipelineLayout();
		}
		return VK_NULL_HANDLE;
	}

} // namespace BinRenderer::Vulkan
