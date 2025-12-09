#pragma once

#include "../Core/RHIType.h"
#include "RHIPipeline.h"
#include "RHIBuffer.h"
#include "RHIDescriptor.h"

namespace BinRenderer
{
	/**
	 * @brief 커맨드 버퍼 추상 클래스
  */
	class RHICommandBuffer
	{
	public:
		virtual ~RHICommandBuffer() = default;

		virtual void begin() = 0;
		virtual void end() = 0;
		virtual void reset() = 0;

		virtual void bindPipeline(RHIPipeline* pipeline) = 0;
		virtual void bindVertexBuffer(uint32_t binding, RHIBuffer* buffer, RHIDeviceSize offset = 0) = 0;
		virtual void bindIndexBuffer(RHIBuffer* buffer, RHIDeviceSize offset = 0) = 0;
		virtual void bindDescriptorSets(RHIPipelineLayout* layout, uint32_t firstSet, uint32_t setCount, RHIDescriptorSet** sets) = 0;

		virtual void draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) = 0;
		virtual void drawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) = 0;
		virtual void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;
	};

	/**
	 * @brief 커맨드 풀 추상 클래스
	 */
	class RHICommandPool
	{
	public:
		virtual ~RHICommandPool() = default;

		virtual void reset() = 0;
		virtual RHICommandBuffer* allocateCommandBuffer(RHICommandBufferLevel level = RHI_COMMAND_BUFFER_LEVEL_PRIMARY) = 0;
	};

} // namespace BinRenderer
