#pragma once

#include "../Core/RHIType.h"
#include "RHICommandBuffer.h"

namespace BinRenderer
{
	/**
	 * @brief 커맨드 풀
	 */
	class RHICommandPool
	{
	public:
		virtual ~RHICommandPool() = default;

		virtual void reset() = 0;
		virtual RHICommandBuffer* allocateCommandBuffer(RHICommandBufferLevel level = RHI_COMMAND_BUFFER_LEVEL_PRIMARY) = 0;
	};

} // namespace BinRenderer
