#pragma once

#include "../Core/RHIType.h"
#include "RHICommonStructs.h"

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

	struct RHIImageSubresourceRange
    {
        RHIImageAspectFlagBits aspectMask;
        uint32_t baseMipLevel;
        uint32_t levelCount;
        uint32_t baseArrayLayer;
        uint32_t layerCount;
    };

	struct RHIComponentMapping
    {
        RHIComponentSwizzle r;
        RHIComponentSwizzle g;
        RHIComponentSwizzle b;
        RHIComponentSwizzle a;
    };

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
		RHIComponentMapping components = { RHI_COMPONENT_SWIZZLE_IDENTITY, RHI_COMPONENT_SWIZZLE_IDENTITY, RHI_COMPONENT_SWIZZLE_IDENTITY, RHI_COMPONENT_SWIZZLE_IDENTITY };
		RHIImageSubresourceRange subresourceRange = { RHI_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	};

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

	struct RHIImageSubresourceLayers
    {
        RHIImageAspectFlagBits aspectMask;
        uint32_t mipLevel;
        uint32_t baseArrayLayer;
        uint32_t layerCount;
    };

	struct RHIImageBlit
    {
        RHIImageSubresourceLayers srcSubresource;
        RHIOffset3D srcOffsets[2];
        RHIImageSubresourceLayers dstSubresource;
        RHIOffset3D dstOffsets[2];
    };

	struct RHIImageFormatProperties
    {
        RHIExtent3D maxExtent;
        uint32_t maxMipLevels;
        uint32_t maxArrayLayers;
        RHISampleCountFlagBits sampleCounts;
        RHIDeviceSize maxResourceSize;
    };

} // namespace BinRenderer
