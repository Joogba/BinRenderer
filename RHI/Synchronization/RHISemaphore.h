#pragma once

#include "../Core/RHIType.h"

namespace BinRenderer
{
	/**
	 * @brief Semaphore 동기화 객체
  */
	class RHISemaphore
	{
	public:
		virtual ~RHISemaphore() = default;
	};

} // namespace BinRenderer
