#pragma once

#include "../Core/RHIType.h"

namespace BinRenderer
{
	/**
   * @brief Event 동기화 객체
	 */
	class RHIEvent
	{
	public:
		virtual ~RHIEvent() = default;

		virtual void set() = 0;
		virtual void reset() = 0;
		virtual bool isSignaled() = 0;
	};

} // namespace BinRenderer
