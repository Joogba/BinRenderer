#pragma once

#include "../Core/RHIType.h"
#include "../Core/RHIHandle.h"
#include "RHICommonStructs.h"

namespace BinRenderer
{
	struct RHICommandPoolCreateInfo
    {
        RHICommandPoolCreateFlags flags;
        uint32_t queueFamilyIndex;
    };

	struct RHICommandBufferAllocateInfo
    {
        RHICommandPool* commandPool;
        RHICommandBufferLevel level;
        uint32_t commandBufferCount;
    };

	struct RHICommandBufferInheritanceInfo
    {
        RHIRenderPass* renderPass;
        uint32_t subpass;
        RHIFramebuffer* framebuffer;
        bool occlusionQueryEnable;
        RHIQueryControlFlags queryFlags;
        RHIQueryPipelineStatisticFlags pipelineStatistics;
    };

	struct RHICommandBufferBeginInfo
    {
        RHICommandBufferUsageFlags flags;
        const RHICommandBufferInheritanceInfo* pInheritanceInfo;
    };

} // namespace BinRenderer
