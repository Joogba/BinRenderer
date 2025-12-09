#pragma once

#include "../Core/RHIType.h"
#include "RHIShader.h"
#include <vector>

namespace BinRenderer
{
	class RHIPipelineLayout;
	class RHIRenderPass;

	/**
	 * @brief 파이프라인 추상 클래스
	 */
	class RHIPipeline
	{
	public:
		virtual ~RHIPipeline() = default;

		virtual RHIPipelineBindPoint getBindPoint() const = 0;
		virtual RHIPipelineLayout* getLayout() const = 0;
		virtual RHIRenderPass* getRenderPass() const = 0;
	};

	/**
	 * @brief 파이프라인 레이아웃 추상 클래스
	 */
	class RHIPipelineLayout
	{
	public:
		virtual ~RHIPipelineLayout() = default;

		virtual uint32_t getSetLayoutCount() const = 0;
	};

} // namespace BinRenderer
