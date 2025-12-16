#pragma once

#include "../../Pipeline/RHIRenderPass.h"
#include <vulkan/vulkan.h>
#include <vector>

namespace BinRenderer::Vulkan
{
	/**
  * @brief Vulkan 렌더 패스 구현
	 */
	class VulkanRenderPass : public RHIRenderPass
	{
	public:
		VulkanRenderPass(VkDevice device);
		~VulkanRenderPass() override;

		bool createSimple(VkFormat colorFormat, VkFormat depthFormat = VK_FORMAT_UNDEFINED);
		void destroy();

		// RHIRenderPass 인터페이스 구현
		uint32_t getAttachmentCount() const override { return attachmentCount_; }
		uint32_t getSubpassCount() const override { return 1; }

		// Vulkan 네이티브 접근
		VkRenderPass getVkRenderPass() const { return renderPass_; }

	private:
		VkDevice device_;
		VkRenderPass renderPass_ = VK_NULL_HANDLE;
		uint32_t attachmentCount_ = 0;
	};

} // namespace BinRenderer::Vulkan
