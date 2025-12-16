#pragma once

#include "../../Pipeline/RHIFramebuffer.h"
#include "VulkanRenderPass.h"
#include <vulkan/vulkan.h>
#include <vector>

namespace BinRenderer::Vulkan
{
	/**
		 * @brief Vulkan 프레임버퍼 구현
		 */
	class VulkanFramebuffer : public RHIFramebuffer
	{
	public:
		VulkanFramebuffer(VkDevice device);
		~VulkanFramebuffer() override;

		bool create(VulkanRenderPass* renderPass, const std::vector<VkImageView>& attachments, uint32_t width, uint32_t height);
		void destroy();

		// RHIFramebuffer 인터페이스 구현
		uint32_t getWidth() const override { return width_; }
		uint32_t getHeight() const override { return height_; }
		uint32_t getLayers() const override { return 1; }

		// Vulkan 네이티브 접근
		VkFramebuffer getVkFramebuffer() const { return framebuffer_; }

	private:
		VkDevice device_;
		VkFramebuffer framebuffer_ = VK_NULL_HANDLE;
		uint32_t width_ = 0;
		uint32_t height_ = 0;
	};

} // namespace BinRenderer::Vulkan
