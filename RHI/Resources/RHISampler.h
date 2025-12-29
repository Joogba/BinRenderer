#pragma once

#include "../Core/RHIType.h"
#include "../Core/RHIResource.h"

namespace BinRenderer
{
	/**
  * @brief 샘플러 리소스 추상 클래스
	 */
	class RHISampler : public RHIResource
	{
	public:
		virtual ~RHISampler() = default;

		virtual RHIFilter getMinFilter() const = 0;
		virtual RHIFilter getMagFilter() const = 0;
		virtual RHISamplerMipmapMode getMipmapMode() const = 0;
		virtual RHISamplerAddressMode getAddressModeU() const = 0;
		virtual RHISamplerAddressMode getAddressModeV() const = 0;
		virtual RHISamplerAddressMode getAddressModeW() const = 0;
	};

} // namespace BinRenderer
