#pragma once

#include "../Core/RHIType.h"

namespace BinRenderer
{
	/**
	 * @brief 샘플러 생성 정보
	 */
	struct RHISamplerCreateInfo
	{
		RHIFilter magFilter = RHI_FILTER_LINEAR;
		RHIFilter minFilter = RHI_FILTER_LINEAR;
		RHISamplerMipmapMode mipmapMode = RHI_SAMPLER_MIPMAP_MODE_LINEAR;
		RHISamplerAddressMode addressModeU = RHI_SAMPLER_ADDRESS_MODE_REPEAT;
		RHISamplerAddressMode addressModeV = RHI_SAMPLER_ADDRESS_MODE_REPEAT;
		RHISamplerAddressMode addressModeW = RHI_SAMPLER_ADDRESS_MODE_REPEAT;
		float mipLodBias = 0.0f;
		bool anisotropyEnable = false;
		float maxAnisotropy = 1.0f;
		bool compareEnable = false;
		RHICompareOp compareOp = RHI_COMPARE_OP_NEVER;
		float minLod = 0.0f;
		float maxLod = 1.0f;
		RHIBorderColor borderColor = RHI_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
	};

} // namespace BinRenderer
