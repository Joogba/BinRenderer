#pragma once

#include "../Core/RHIType.h"
#include "RHIStructs.h"

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

	/**
	 * @brief 버퍼 복사 정보
	 */
	struct RHIBufferCopy
	{
		RHIDeviceSize srcOffset;
		RHIDeviceSize dstOffset;
		RHIDeviceSize size;
	};

	/**
	 * @brief 버퍼 → 이미지 복사 정보
	 */
	struct RHIBufferImageCopy
	{
		RHIDeviceSize bufferOffset;
		uint32_t bufferRowLength;
		uint32_t bufferImageHeight;
		RHIImageSubresourceLayers imageSubresource;
		RHIOffset3D imageOffset;
		RHIExtent3D imageExtent;
	};

} // namespace BinRenderer
