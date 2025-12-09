#pragma once

#include "../Core/RHIType.h"

namespace BinRenderer
{
	/**
	 * @brief 버퍼 생성 정보
	 */
	struct RHIBufferCreateInfo
	{
		RHIDeviceSize size = 0;
		RHIBufferUsageFlags usage = 0;
		RHIMemoryPropertyFlags memoryProperties = 0;
		const void* initialData = nullptr;
	};

} // namespace BinRenderer
