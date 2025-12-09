#pragma once

#include "../Core/RHIType.h"

namespace BinRenderer
{
	/**
	 * @brief 파이프라인 레이아웃
   */
	class RHIPipelineLayout
	{
	public:
		virtual ~RHIPipelineLayout() = default;

		virtual uint32_t getSetLayoutCount() const = 0;
	};

} // namespace BinRenderer
