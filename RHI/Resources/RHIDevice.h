#pragma once

#include "../Core/RHIType.h"

namespace BinRenderer
{
	/**
* @brief 디바이스 메모리 추상 클래스
	 */
	class RHIDeviceMemory
	{
	public:
		virtual ~RHIDeviceMemory() = default;

		virtual void* map(RHIDeviceSize offset, RHIDeviceSize size) = 0;
		virtual void unmap() = 0;
		virtual RHIDeviceSize getSize() const = 0;
		virtual RHIMemoryPropertyFlags getProperties() const = 0;
	};

	/**
	 * @brief 물리 디바이스 추상 클래스
  */
	class RHIPhysicalDevice
	{
	public:
		virtual ~RHIPhysicalDevice() = default;

		virtual RHIPhysicalDeviceType getDeviceType() const = 0;
		virtual const char* getDeviceName() const = 0;
	};

	/**
	 * @brief 논리 디바이스 추상 클래스
   */
	class RHIDevice
	{
	public:
		virtual ~RHIDevice() = default;

		virtual void waitIdle() = 0;
		virtual RHIPhysicalDevice* getPhysicalDevice() const = 0;
	};

	/**
	 * @brief 인스턴스 추상 클래스
	 */
	class RHIInstance
	{
	public:
		virtual ~RHIInstance() = default;
	};

} // namespace BinRenderer
