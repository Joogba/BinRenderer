#pragma once

#include "../Core/RHIType.h"

namespace BinRenderer
{
	/**
	 * @brief 이미지 뷰 생성 정보
	 */
	struct RHIImageViewCreateInfo
	{
		RHIImageViewType viewType = RHI_IMAGE_VIEW_TYPE_2D;
		RHIFormat format = RHI_FORMAT_UNDEFINED;
		RHIImageAspectFlagBits aspectMask = RHI_IMAGE_ASPECT_COLOR_BIT;
		uint32_t baseMipLevel = 0;
		uint32_t levelCount = 1;
		uint32_t baseArrayLayer = 0;
		uint32_t layerCount = 1;
	};

} // namespace BinRenderer
