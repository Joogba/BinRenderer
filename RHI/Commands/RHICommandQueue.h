#pragma once

#include "../Core/RHIType.h"
#include "RHICommandBuffer.h"

namespace BinRenderer
{
	/**
 * @brief 커맨드 큐
	 */
	class RHICommandQueue
	{
	public:
		virtual ~RHICommandQueue() = default;

		virtual void waitIdle() = 0;
		virtual void submit(RHICommandBuffer* commandBuffer, class RHIFence* fence = nullptr) = 0;
		virtual void submit(uint32_t submitCount, const struct RHISubmitInfo* submitInfos, class RHIFence* fence = nullptr) = 0;
	};

} // namespace BinRenderer
