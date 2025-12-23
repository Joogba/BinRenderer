#pragma once

#include "../Core/RHIType.h"

namespace BinRenderer
{
	/**
		* @brief 이미지 생성 정보
		*/
	struct RHIImageCreateInfo
	{
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t depth = 1;
		uint32_t mipLevels = 1;
		uint32_t arrayLayers = 1;
		RHIFormat format = RHI_FORMAT_UNDEFINED;
		RHIImageUsageFlags usage = 0;
		RHISampleCountFlagBits samples = RHI_SAMPLE_COUNT_1_BIT;
		RHIImageTiling tiling = RHI_IMAGE_TILING_OPTIMAL;
		uint32_t flags = 0;  // RHI_IMAGE_CREATE_CUBE_COMPATIBLE_BIT 등
	};

} // namespace BinRenderer
