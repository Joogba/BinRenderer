#pragma once

#include "../Core/RHIType.h"
#include "../Resources/RHIBuffer.h"
#include "../Resources/RHIImage.h"
#include <vector>

namespace BinRenderer
{
	/**
	 * @brief 디스크립터 셋 dx12는 디스크립터 테이블
	 */
	class RHIDescriptorSet
	{
	public:
		virtual ~RHIDescriptorSet() = default;

		virtual void updateBuffer(uint32_t binding, RHIBuffer* buffer, RHIDeviceSize offset = 0, RHIDeviceSize range = 0) = 0;
		virtual void updateImage(uint32_t binding, RHIImageView* imageView, class RHISampler* sampler = nullptr) = 0;
		
		// Bindless descriptor array support
		virtual void updateImageArray(uint32_t binding, uint32_t arrayIndex, RHIImageView* imageView, class RHISampler* sampler = nullptr) = 0;
		virtual void updateImageArrayBatch(uint32_t binding, const std::vector<RHIImageView*>& imageViews, class RHISampler* sampler = nullptr) = 0;
	};

	/**
	 * @brief 디스크립터 셋 레이아웃
	 */
	class RHIDescriptorSetLayout
	{
	public:
		virtual ~RHIDescriptorSetLayout() = default;

		virtual uint32_t getBindingCount() const = 0;
	};

	/**
	 * @brief 디스크립터 풀
	 * directx11 은 디스크립터 힙으로
   */
	class RHIDescriptorPool
	{
	public:
		virtual ~RHIDescriptorPool() = default;

		virtual void reset() = 0;
		virtual RHIDescriptorSet* allocateDescriptorSet(RHIDescriptorSetLayout* layout) = 0;
	};

} // namespace BinRenderer
