#pragma once

#include "RHIDefinitions.h"
#include "RHIType.h"
#include "RHISwapchain.h"
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
		virtual uint32_t getCurrentImageIndex() const = 0;

		// 스왑체인 접근
		virtual RHISwapchain* getSwapchain() const = 0;

		// 리소스 생성
		virtual RHIBuffer* createBuffer(const RHIBufferCreateInfo& createInfo) = 0;
		virtual RHIImage* createImage(const RHIImageCreateInfo& createInfo) = 0;
		virtual RHIShader* createShader(const RHIShaderCreateInfo& createInfo) = 0;
		virtual RHIPipeline* createPipeline(const RHIPipelineCreateInfo& createInfo) = 0;
		virtual RHIImageView* createImageView(RHIImage* image, const RHIImageViewCreateInfo& createInfo) = 0;
		virtual RHISampler* createSampler(const RHISamplerCreateInfo& createInfo) = 0;

		// ✅ Descriptor Set 생성
		virtual RHIDescriptorSetLayout* createDescriptorSetLayout(const RHIDescriptorSetLayoutCreateInfo& createInfo) = 0;
		virtual RHIDescriptorPool* createDescriptorPool(const RHIDescriptorPoolCreateInfo& createInfo) = 0;
		virtual RHIDescriptorSet* allocateDescriptorSet(RHIDescriptorPool* pool, RHIDescriptorSetLayout* layout) = 0;

		// 리소스 해제
		virtual void destroyBuffer(RHIBuffer* buffer) = 0;
		virtual void destroyImage(RHIImage* image) = 0;
		virtual void destroyShader(RHIShader* shader) = 0;
		virtual void destroyPipeline(RHIPipeline* pipeline) = 0;
		virtual void destroyImageView(RHIImageView* imageView) = 0;
		virtual void destroySampler(RHISampler* sampler) = 0;

		// ✅ Descriptor Set 해제
		virtual void destroyDescriptorSetLayout(RHIDescriptorSetLayout* layout) = 0;
		virtual void destroyDescriptorPool(RHIDescriptorPool* pool) = 0;

		// 버퍼 매핑
		virtual void* mapBuffer(RHIBuffer* buffer) = 0;
		virtual void unmapBuffer(RHIBuffer* buffer) = 0;
		virtual void flushBuffer(RHIBuffer* buffer, RHIDeviceSize offset = 0, RHIDeviceSize size = 0) = 0;

		// 커맨드 기록
		virtual void beginCommandRecording() = 0;
		virtual void endCommandRecording() = 0;
		virtual void submitCommands() = 0;

		// 드로우 커맨드
		virtual void cmdBindPipeline(RHIPipeline* pipeline) = 0;
		virtual void cmdBindVertexBuffer(RHIBuffer* buffer, RHIDeviceSize offset = 0) = 0;
		virtual void cmdBindIndexBuffer(RHIBuffer* buffer, RHIDeviceSize offset = 0) = 0;
		virtual void cmdBindDescriptorSets(RHIPipelineLayout* layout, RHIDescriptorSet** sets, uint32_t setCount) = 0;
		virtual void cmdPushConstants(RHIPipelineLayout* layout, RHIShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues) = 0;
		virtual void cmdSetViewport(const RHIViewport& viewport) = 0;
		virtual void cmdSetScissor(const RHIRect2D& scissor) = 0;
		virtual void cmdDraw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) = 0;
		virtual void cmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) = 0;

		// ✅ Descriptor Sets 바인딩 (Pipeline 사용)
		virtual void cmdBindDescriptorSets(RHIPipeline* pipeline, uint32_t firstSet, RHIDescriptorSet** sets, uint32_t setCount) = 0;

		// ✅ Push Constants (Pipeline 사용)
		virtual void cmdPushConstants(RHIPipeline* pipeline, RHIShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues) = 0;

		// ✅ Dynamic Rendering
		virtual void cmdBeginRendering(uint32_t width, uint32_t height, RHIImageView* colorAttachment, RHIImageView* depthAttachment = nullptr) = 0;
		virtual void cmdEndRendering() = 0;

		// ✅ Image Layout Transition
		virtual void cmdTransitionImageLayout(
			RHIImage* image,
			RHIImageLayout oldLayout,
			RHIImageLayout newLayout,
			RHIImageAspectFlagBits aspectMask = RHI_IMAGE_ASPECT_COLOR_BIT,
			uint32_t baseMipLevel = 0,
			uint32_t levelCount = 1,
			uint32_t baseArrayLayer = 0,
			uint32_t layerCount = 1
		) = 0;

		// API 타입
		virtual RHIApiType getApiType() const = 0;
	};

} // namespace BinRenderer