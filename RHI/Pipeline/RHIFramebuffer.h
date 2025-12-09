#pragma once

#include "../Core/RHIType.h"

namespace BinRenderer
{
	/**
	 * @brief 프레임버퍼
	 */
	class RHIFramebuffer
	{
	public:
		virtual ~RHIFramebuffer() = default;

		virtual uint32_t getWidth() const = 0;
		virtual uint32_t getHeight() const = 0;
		virtual uint32_t getLayers() const = 0;
	};

} // namespace BinRenderer
