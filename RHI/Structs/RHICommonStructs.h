#pragma once

#include "../Core/RHIType.h"
#include <cstdint>

namespace BinRenderer
{
	// Forward declarations
	class RHIBuffer;
	class RHIBufferView;
	class RHICommandBuffer;
	class RHICommandPool;
	class RHIDescriptorPool;
	class RHIDescriptorSet;
	class RHIDescriptorSetLayout;
	class RHIDevice;
	class RHIDeviceMemory;
	class RHIEvent;
	class RHIFence;
	class RHIFramebuffer;
	class RHIImage;
	class RHIImageView;
	class RHIInstance;
	class RHIQueue;
	class RHIPhysicalDevice;
	class RHIPipeline;
	class RHIPipelineCache;
	class RHIPipelineLayout;
	class RHIRenderPass;
	class RHISampler;
	class RHISemaphore;
	class RHIShader;

	struct RHIOffset2D
	{
		int32_t x;
		int32_t y;
	};

	struct RHIOffset3D
	{
		int32_t x;
		int32_t y;
		int32_t z;
	};

	struct RHIExtent2D
	{
		uint32_t width;
		uint32_t height;
	};

	struct RHIExtent3D
	{
		uint32_t width;
		uint32_t height;
		uint32_t depth;
	};

	struct RHIRect2D
	{
		RHIOffset2D offset;
		RHIExtent2D extent;
	};

	struct RHIViewport
	{
		float x;
		float y;
		float width;
		float height;
		float minDepth;
		float maxDepth;
	};

} // namespace BinRenderer
