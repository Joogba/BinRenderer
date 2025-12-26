#pragma once

#include "../Core/RHIType.h"
#include "../Core/RHIHandle.h"

namespace BinRenderer
{
	/**
		* @brief 텍스처 리소스 (Image + ImageView + Sampler 조합)
		*/
	class RHITexture
	{
	public:
		virtual ~RHITexture() = default;

		virtual RHIImageHandle getImage() const = 0;
		virtual RHIImageViewHandle getImageView() const = 0;
		virtual RHISamplerHandle getSampler() const = 0;

		virtual uint32_t getWidth() const = 0;
		virtual uint32_t getHeight() const = 0;
		virtual uint32_t getMipLevels() const = 0;
	};

} // namespace BinRenderer
