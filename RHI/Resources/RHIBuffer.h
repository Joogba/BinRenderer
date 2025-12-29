#pragma once

#include "../Core/RHIType.h"
#include "../Core/RHIResource.h"

namespace BinRenderer
{
	/**
	 * @brief 버퍼 리소스 추상 클래스
	 */
	class RHIBuffer : public RHIResource
	{
	public:
		virtual ~RHIBuffer() = default;

		virtual void* map() = 0;
		virtual void unmap() = 0;
		virtual void updateData(const void* data, RHIDeviceSize size, RHIDeviceSize offset = 0) = 0;

		virtual RHIDeviceSize getSize() const = 0;
		virtual RHIBufferUsageFlags getUsage() const = 0;
	};

} // namespace BinRenderer
