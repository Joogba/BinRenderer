#pragma once

#include "../Core/RHIType.h"

namespace BinRenderer
{
	/**
	 * @brief 이미지 리소스 추상 클래스
	 */
	class RHIImage
	{
	public:
		virtual ~RHIImage() = default;

		virtual uint32_t getWidth() const = 0;
		virtual uint32_t getHeight() const = 0;
		virtual uint32_t getDepth() const = 0;
		virtual uint32_t getMipLevels() const = 0;
		virtual uint32_t getArrayLayers() const = 0;
		virtual RHIFormat getFormat() const = 0;
		virtual RHISampleCountFlagBits getSamples() const = 0;
	};

	/**
	 * @brief 이미지 뷰 추상 클래스
	 */
	class RHIImageView
	{
	public:
		virtual ~RHIImageView() = default;

		virtual RHIImage* getImage() const = 0;
		virtual RHIImageViewType getViewType() const = 0;
		virtual RHIFormat getFormat() const = 0;
	};

} // namespace BinRenderer
