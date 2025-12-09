#pragma once

#include "../Core/RHIType.h"

namespace BinRenderer
{
	/**
		 * @brief Fence 동기화 객체
		 */
	class RHIFence
	{
	public:
		virtual ~RHIFence() = default;

		virtual void wait(uint64_t timeout = UINT64_MAX) = 0;
		virtual void reset() = 0;
		virtual bool isSignaled() = 0;
	};

} // namespace BinRenderer
