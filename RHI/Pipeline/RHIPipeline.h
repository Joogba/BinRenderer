#pragma once

#include "../Core/RHIType.h"
#include "../Core/RHIResource.h"

namespace BinRenderer
{
	class RHIPipelineLayout;
	class RHIRenderPass;

	/**
	 * @brief 파이프라인 추상 클래스
  */
	class RHIPipeline : public RHIResource
	{
	public:
		virtual ~RHIPipeline() = default;

		virtual RHIPipelineBindPoint getBindPoint() const = 0;
		virtual RHIPipelineLayout* getLayout() const = 0;
		virtual RHIRenderPass* getRenderPass() const = 0;
	};

} // namespace BinRenderer
