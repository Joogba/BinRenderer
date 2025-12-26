#pragma once

#include "../Core/RHIType.h"
#include "../Core/RHIHandle.h"
#include "RHIStructs.h"

namespace BinRenderer
{
	struct RHIAttachmentDescription
    {
        RHIFormat format;
        RHISampleCountFlagBits samples;
        RHIAttachmentLoadOp loadOp;
        RHIAttachmentStoreOp storeOp;
        RHIAttachmentLoadOp stencilLoadOp;
        RHIAttachmentStoreOp stencilStoreOp;
        RHIImageLayout initialLayout;
        RHIImageLayout finalLayout;
    };

	struct RHIAttachmentReference
	{
		uint32_t attachment;
		RHIImageLayout layout;
	};

	struct RHISubpassDescription
    {
        RHISubpassDescriptionFlags flags;
        RHIPipelineBindPoint pipelineBindPoint;
        uint32_t inputAttachmentCount;
        const RHIAttachmentReference* pInputAttachments;
		uint32_t colorAttachmentCount;
		const RHIAttachmentReference* pColorAttachments;
		const RHIAttachmentReference* pResolveAttachments;
		const RHIAttachmentReference* pDepthStencilAttachment;
		uint32_t preserveAttachmentCount;
		const uint32_t* pPreserveAttachments;
    };

	struct RHISubpassDependency
    {
        uint32_t srcSubpass;
        uint32_t dstSubpass;
        RHIPipelineStageFlags srcStageMask;
        RHIPipelineStageFlags dstStageMask;
        RHIAccessFlags srcAccessMask;
        RHIAccessFlags dstAccessMask;
        RHISubpassDependencyFlags dependencyFlags;
    };

	struct RHIRenderPassCreateInfo
    {
        RHIRenderPassCreateFlags flags;
        uint32_t attachmentCount;
        const RHIAttachmentDescription* pAttachments;
        uint32_t subpassCount;
        const RHISubpassDescription* pSubpasses;
        uint32_t dependencyCount;
        const RHISubpassDependency* pDependencies;
    };

	struct RHIFramebufferCreateInfo
    {
        RHIFramebufferCreateFlags flags;
        RHIRenderPass* renderPass;
        uint32_t attachmentCount;
        const RHIImageView* pAttachments;
        uint32_t width;
        uint32_t height;
        uint32_t layers;
    };
    union RHIClearColorValue
    {
        float float32[4];
        int32_t int32[4];
        uint32_t uint32[4];
    };
    struct RHIClearDepthStencilValue
    {
        float depth;
        uint32_t stencil;
    };
    union RHIClearValue
    {
        RHIClearColorValue color;
        RHIClearDepthStencilValue depthStencil;
    };

	struct RHIRenderPassBeginInfo
	{
		RHIRenderPass* renderPass;
		RHIFramebuffer* framebuffer;
		RHIRect2D renderArea;
		uint32_t clearValueCount;
		const union RHIClearValue* pClearValues;
	};


    

} // namespace BinRenderer
