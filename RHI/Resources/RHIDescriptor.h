#pragma once

#include "../Core/RHIType.h"
#include "RHIBuffer.h"
#include "RHIImage.h"
#include <vector>

namespace BinRenderer
{
	/**
		 * @brief 디스크립터 셋 추상 클래스
		 */
	class RHIDescriptorSet
	{
	public:
		virtual ~RHIDescriptorSet() = default;

		virtual void updateBuffer(uint32_t binding, RHIBuffer* buffer, RHIDeviceSize offset = 0, RHIDeviceSize range = 0) = 0;
		virtual void updateImage(uint32_t binding, RHIImageView* imageView, class RHISampler* sampler = nullptr) = 0;
	};

	/**
	 * @brief 디스크립터 셋 레이아웃 추상 클래스
   */
	class RHIDescriptorSetLayout
	{
	public:
		virtual ~RHIDescriptorSetLayout() = default;

		virtual uint32_t getBindingCount() const = 0;
	};

	/**
	 * @brief 디스크립터 풀 추상 클래스
	 */
	class RHIDescriptorPool
	{
	public:
		virtual ~RHIDescriptorPool() = default;

		virtual void reset() = 0;
		virtual RHIDescriptorSet* allocateDescriptorSet(RHIDescriptorSetLayout* layout) = 0;
	};

} // namespace BinRenderer
