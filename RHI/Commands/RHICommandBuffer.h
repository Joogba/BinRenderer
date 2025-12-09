#pragma once

#include "../Core/RHIType.h"
#include "../Pipeline/RHIPipeline.h"
#include "../Pipeline/RHIPipelineLayout.h"
#include "../Pipeline/RHIDescriptor.h"
#include "../Resources/RHIBuffer.h"

namespace BinRenderer
{
	/**
	 * @brief 커맨드 버퍼 (= 커맨드 리스트)
	 */
	class RHICommandBuffer
	{
	public:
		virtual ~RHICommandBuffer() = default;

		virtual void begin() = 0;
		virtual void end() = 0;
		virtual void reset() = 0;

		// 파이프라인
		virtual void bindPipeline(RHIPipeline* pipeline) = 0;

		// 버퍼
		virtual void bindVertexBuffer(uint32_t binding, RHIBuffer* buffer, RHIDeviceSize offset = 0) = 0;
		virtual void bindIndexBuffer(RHIBuffer* buffer, RHIDeviceSize offset = 0) = 0;

		// 디스크립터
		virtual void bindDescriptorSets(RHIPipelineLayout* layout, uint32_t firstSet, uint32_t setCount, RHIDescriptorSet** sets) = 0;

		// 드로우
		virtual void draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) = 0;
		virtual void drawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) = 0;
		virtual void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;
	};

} // namespace BinRenderer
