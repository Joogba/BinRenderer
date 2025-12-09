#pragma once

#include "RHIBuffer.h"
#include "RHIImage.h"
#include "RHITexture.h"
#include "RHISampler.h"
#include "RHIShader.h"
#include "RHIRenderTarget.h"
#include "RHIPipeline.h"
#include "RHIDescriptor.h"
#include "RHICommand.h"
#include "RHISync.h"
#include "RHIDevice.h"

namespace BinRenderer
{
	// 추가 유틸리티 클래스

	/**
	 * @brief 버퍼 뷰 (텍스처 버퍼용)
	 */
	class RHIBufferView
	{
	public:
		virtual ~RHIBufferView() = default;

		virtual RHIBuffer* getBuffer() const = 0;
		virtual RHIFormat getFormat() const = 0;
		virtual RHIDeviceSize getOffset() const = 0;
		virtual RHIDeviceSize getRange() const = 0;
	};

	/**
	 * @brief 파이프라인 캐시
  */
	class RHIPipelineCache
	{
	public:
		virtual ~RHIPipelineCache() = default;

		virtual void merge(uint32_t cacheCount, RHIPipelineCache** caches) = 0;
		virtual void getData(size_t* dataSize, void* data) = 0;
	};

} // namespace BinRenderer
