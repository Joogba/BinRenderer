#include "VulkanFramebuffer.h"
#include "Vulkan/Logger.h"

namespace BinRenderer::Vulkan
{
	VulkanFramebuffer::VulkanFramebuffer(VkDevice device)
		: device_(device)
	{
	}

	VulkanFramebuffer::~VulkanFramebuffer()
	{
		destroy();
	}

	bool VulkanFramebuffer::create(VulkanRenderPass* renderPass, const std::vector<VkImageView>& attachments, uint32_t width, uint32_t height)
	{
		width_ = width;
		height_ = height;

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass->getVkRenderPass();
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = width;
		framebufferInfo.height = height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device_, &framebufferInfo, nullptr, &framebuffer_) != VK_SUCCESS)
		{
			return false;
		}

		return true;
	}

	void VulkanFramebuffer::destroy()
	{
		if (framebuffer_ != VK_NULL_HANDLE)
		{
			vkDestroyFramebuffer(device_, framebuffer_, nullptr);
			framebuffer_ = VK_NULL_HANDLE;
		}
	}

} // namespace BinRenderer::Vulkan
