#pragma once

#include "RHIDefinitions.h"
#include "RHIType.h"
#include "../Structs/RHIStructs.h"
#include "../Resources/RHIBuffer.h"
#include "../Resources/RHIImage.h"
#include "../Resources/RHIShader.h"
#include "../Pipeline/RHIPipeline.h"
#include "../Pipeline/RHIPipelineLayout.h"
#include "../Pipeline/RHIDescriptor.h"

namespace BinRenderer
{
	/**
	 * @brief 메인 RHI 인터페이스
   */
	class RHI
	{
	public:
		virtual ~RHI() = default;

		// 초기화 및 생명주기
		virtual bool initialize(const RHIInitInfo& initInfo) = 0;
		virtual void shutdown() = 0;
		virtual void waitIdle() = 0;

		// 프레임 관리
		virtual bool beginFrame(uint32_t& imageIndex) = 0;
		virtual void endFrame(uint32_t imageIndex) = 0;
		virtual uint32_t getCurrentFrameIndex() const = 0;

		// 리소스 생성
		virtual RHIBuffer* createBuffer(const RHIBufferCreateInfo& createInfo) = 0;
		virtual RHIImage* createImage(const RHIImageCreateInfo& createInfo) = 0;
		virtual RHIShader* createShader(const RHIShaderCreateInfo& createInfo) = 0;
		virtual RHIPipeline* createPipeline(const RHIPipelineCreateInfo& createInfo) = 0;

		// 리소스 해제
		virtual void destroyBuffer(RHIBuffer* buffer) = 0;
		virtual void destroyImage(RHIImage* image) = 0;
		virtual void destroyShader(RHIShader* shader) = 0;
		virtual void destroyPipeline(RHIPipeline* pipeline) = 0;

		// 커맨드 기록
		virtual void beginCommandRecording() = 0;
		virtual void endCommandRecording() = 0;
		virtual void submitCommands() = 0;

		// 드로우 커맨드
		virtual void cmdBindPipeline(RHIPipeline* pipeline) = 0;
		virtual void cmdBindVertexBuffer(RHIBuffer* buffer, RHIDeviceSize offset = 0) = 0;
		virtual void cmdBindIndexBuffer(RHIBuffer* buffer, RHIDeviceSize offset = 0) = 0;
		virtual void cmdBindDescriptorSets(RHIPipelineLayout* layout, RHIDescriptorSet** sets, uint32_t setCount) = 0;
		virtual void cmdSetViewport(const RHIViewport& viewport) = 0;
		virtual void cmdSetScissor(const RHIRect2D& scissor) = 0;
		virtual void cmdDraw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) = 0;
		virtual void cmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) = 0;

		// API 타입
		virtual RHIApiType getApiType() const = 0;
	};

	/**
	 * @brief RHI 팩토리
	 */
	class RHIFactory
	{
	public:
		static RHI* createRHI(RHIApiType apiType);
	};

} // namespace BinRenderer