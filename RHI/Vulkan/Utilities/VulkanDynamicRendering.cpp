#include "VulkanDynamicRendering.h"
#include "Vulkan/Logger.h"

namespace BinRenderer::Vulkan
{
	namespace DynamicRendering
	{
		void beginRendering(VkCommandBuffer cmd, const RenderingInfo& info)
		{
			// Color attachments 변환
			std::vector<VkRenderingAttachmentInfo> colorAttachmentInfos;
			colorAttachmentInfos.reserve(info.colorAttachments.size());

			for (const auto& attachment : info.colorAttachments)
			{
				VkRenderingAttachmentInfo attachmentInfo{};
				attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
				attachmentInfo.imageView = attachment.imageView;
				attachmentInfo.imageLayout = attachment.imageLayout;
				attachmentInfo.resolveMode = attachment.resolveMode;
				attachmentInfo.resolveImageView = attachment.resolveImageView;
				attachmentInfo.resolveImageLayout = attachment.resolveImageLayout;
				attachmentInfo.loadOp = attachment.loadOp;
				attachmentInfo.storeOp = attachment.storeOp;
				attachmentInfo.clearValue = attachment.clearValue;

				colorAttachmentInfos.push_back(attachmentInfo);
			}

			// Depth attachment 변환
			VkRenderingAttachmentInfo depthAttachmentInfo{};
			if (info.depthAttachment)
			{
				depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
				depthAttachmentInfo.imageView = info.depthAttachment->imageView;
				depthAttachmentInfo.imageLayout = info.depthAttachment->imageLayout;
				depthAttachmentInfo.resolveMode = info.depthAttachment->resolveMode;
				depthAttachmentInfo.resolveImageView = info.depthAttachment->resolveImageView;
				depthAttachmentInfo.resolveImageLayout = info.depthAttachment->resolveImageLayout;
				depthAttachmentInfo.loadOp = info.depthAttachment->loadOp;
				depthAttachmentInfo.storeOp = info.depthAttachment->storeOp;
				depthAttachmentInfo.clearValue = info.depthAttachment->clearValue;
			}

			// Stencil attachment 변환
			VkRenderingAttachmentInfo stencilAttachmentInfo{};
			if (info.stencilAttachment)
			{
				stencilAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
				stencilAttachmentInfo.imageView = info.stencilAttachment->imageView;
				stencilAttachmentInfo.imageLayout = info.stencilAttachment->imageLayout;
				stencilAttachmentInfo.resolveMode = info.stencilAttachment->resolveMode;
				stencilAttachmentInfo.resolveImageView = info.stencilAttachment->resolveImageView;
				stencilAttachmentInfo.resolveImageLayout = info.stencilAttachment->resolveImageLayout;
				stencilAttachmentInfo.loadOp = info.stencilAttachment->loadOp;
				stencilAttachmentInfo.storeOp = info.stencilAttachment->storeOp;
				stencilAttachmentInfo.clearValue = info.stencilAttachment->clearValue;
			}

			// Rendering info 생성
			VkRenderingInfo renderingInfo{};
			renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
			renderingInfo.renderArea = info.renderArea;
			renderingInfo.layerCount = info.layerCount;
			renderingInfo.viewMask = info.viewMask;
			renderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentInfos.size());
			renderingInfo.pColorAttachments = colorAttachmentInfos.empty() ? nullptr : colorAttachmentInfos.data();
			renderingInfo.pDepthAttachment = info.depthAttachment ? &depthAttachmentInfo : nullptr;
			renderingInfo.pStencilAttachment = info.stencilAttachment ? &stencilAttachmentInfo : nullptr;

			// ✅ Dynamic Rendering 시작
			vkCmdBeginRendering(cmd, &renderingInfo);
		}

		void endRendering(VkCommandBuffer cmd)
		{
			// ✅ Dynamic Rendering 종료
			vkCmdEndRendering(cmd);
		}
	}

} // namespace BinRenderer::Vulkan
