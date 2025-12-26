#pragma once

#include "../Core/RHIType.h"
#include "../Core/RHIHandle.h"
#include "RHICommonStructs.h"
#include "RHIImageStructs.h"

namespace BinRenderer
{
	struct RHIFenceCreateInfo
    {
        RHIFenceCreateFlags flags;
    };

	struct RHISemaphoreCreateInfo
    {
        RHISemaphoreCreateFlags flags;
    };

	struct RHISubmitInfo
    {
        uint32_t waitSemaphoreCount;
        const RHISemaphore* const* pWaitSemaphores;
        const RHIPipelineStageFlags* pWaitDstStageMask;
        uint32_t commandBufferCount;
        const RHICommandBuffer* const* pCommandBuffers;
        uint32_t signalSemaphoreCount;
        const RHISemaphore* const* pSignalSemaphores;
	};

	struct RHIMemoryBarrier
    {
        RHIAccessFlags srcAccessMask;
        RHIAccessFlags dstAccessMask;
    };

	struct RHIBufferMemoryBarrier
    {
        RHIAccessFlags srcAccessMask;
        RHIAccessFlags dstAccessMask;
        uint32_t srcQueueFamilyIndex;
        uint32_t dstQueueFamilyIndex;
        RHIBuffer* buffer;
        RHIDeviceSize offset;
        RHIDeviceSize size;
    };

	struct RHIImageMemoryBarrier
    {
        RHIAccessFlags srcAccessMask;
        RHIAccessFlags dstAccessMask;
        RHIImageLayout oldLayout;
        RHIImageLayout newLayout;
        uint32_t srcQueueFamilyIndex;
        uint32_t dstQueueFamilyIndex;
        RHIImage* image;
        RHIImageSubresourceRange subresourceRange;
    };

} // namespace BinRenderer
