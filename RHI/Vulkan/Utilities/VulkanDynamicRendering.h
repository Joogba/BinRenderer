#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace BinRenderer::Vulkan
{
	/**
	 * @brief Dynamic Rendering (Vulkan 1.3+) 헬퍼
	 * 
	 * Legacy VkRenderPass 없이 렌더링 가능
	 */
	namespace DynamicRendering
	{
		/**
		 * @brief Color attachment 정보
		 */
		struct ColorAttachment
		{
			VkImageView imageView = VK_NULL_HANDLE;
			VkImageLayout imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			VkResolveModeFlagBits resolveMode = VK_RESOLVE_MODE_NONE;
			VkImageView resolveImageView = VK_NULL_HANDLE;
			VkImageLayout resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			VkClearValue clearValue = {};

			ColorAttachment() = default;
			ColorAttachment(VkImageView view, VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
				: imageView(view), imageLayout(layout) {}
		};

		/**
		 * @brief Depth/Stencil attachment 정보
		 */
		struct DepthStencilAttachment
		{
			VkImageView imageView = VK_NULL_HANDLE;
			VkImageLayout imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			VkResolveModeFlagBits resolveMode = VK_RESOLVE_MODE_NONE;
			VkImageView resolveImageView = VK_NULL_HANDLE;
			VkImageLayout resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			VkClearValue clearValue = {};

			DepthStencilAttachment() = default;
			DepthStencilAttachment(VkImageView view, VkImageLayout layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
				: imageView(view), imageLayout(layout) {}
		};

		/**
		 * @brief Rendering 정보
		 */
		struct RenderingInfo
		{
			VkRect2D renderArea = {};
			uint32_t layerCount = 1;
			uint32_t viewMask = 0;
			std::vector<ColorAttachment> colorAttachments;
			DepthStencilAttachment* depthAttachment = nullptr;
			DepthStencilAttachment* stencilAttachment = nullptr;
		};

		/**
		 * @brief Dynamic Rendering 시작
		 * @param cmd Command buffer
		 * @param info Rendering 정보
		 */
		void beginRendering(VkCommandBuffer cmd, const RenderingInfo& info);

		/**
		 * @brief Dynamic Rendering 종료
		 * @param cmd Command buffer
		 */
		void endRendering(VkCommandBuffer cmd);

		/**
		 * @brief 간단한 color attachment 생성 (clear + store)
		 */
		inline ColorAttachment makeColorAttachment(
			VkImageView imageView,
			const VkClearColorValue& clearColor = { 0.0f, 0.0f, 0.0f, 1.0f })
		{
			ColorAttachment attachment;
			attachment.imageView = imageView;
			attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment.clearValue.color = clearColor;
			return attachment;
		}

		/**
		 * @brief 간단한 depth attachment 생성 (clear + store)
		 */
		inline DepthStencilAttachment makeDepthAttachment(
			VkImageView imageView,
			float clearDepth = 1.0f,
			uint32_t clearStencil = 0)
		{
			DepthStencilAttachment attachment;
			attachment.imageView = imageView;
			attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment.clearValue.depthStencil = { clearDepth, clearStencil };
			return attachment;
		}
	}

} // namespace BinRenderer::Vulkan
