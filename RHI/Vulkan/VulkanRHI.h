#pragma once

#include "../Core/RHI.h"
#include "../Core/RHIHandle.h"
#include "../Core/RHIResourcePool.h"
#include "../Commands/RHICommandBuffer.h"
#include "../Commands/RHICommandPool.h"
#include "../Commands/RHICommandQueue.h"
#include "../Resources/RHITexture.h"

#include "Core/VulkanContext.h"
#include "Core/VulkanSwapchain.h"
#include "Commands/VulkanCommandPool.h"
#include "Commands/VulkanCommandBuffer.h"

#include <memory>
#include <vector>

namespace BinRenderer::Vulkan
{
	/**
	 * @brief Vulkan RHI 구현
	 */
	class VulkanRHI : public RHI
	{
	public:
		VulkanRHI() = default;
		~VulkanRHI() override;

		// 초기화 및 생명주기
		bool initialize(const RHIInitInfo& initInfo) override;
		void shutdown() override;
		void waitIdle() override;

		// 프레임 관리
		bool beginFrame(uint32_t& imageIndex) override;
		void endFrame(uint32_t imageIndex) override;
		uint32_t getCurrentFrameIndex() const override;
		uint32_t getCurrentImageIndex() const override;	//  Swapchain image index 가져오기

		// 스왑체인 접근
		RHISwapchain* getSwapchain() const override { return swapchain_.get(); }
		RHIImageViewHandle getSwapchainImageView(uint32_t index) const;

		// 리소스 생성
		RHIBufferHandle createBuffer(const RHIBufferCreateInfo& createInfo) override;
		RHIImageHandle createImage(const RHIImageCreateInfo& createInfo) override;
		RHIShaderHandle createShader(const RHIShaderCreateInfo& createInfo) override;
		RHIPipelineHandle createPipeline(const RHIPipelineCreateInfo& createInfo) override;
		RHIImageViewHandle createImageView(RHIImageHandle image, const RHIImageViewCreateInfo& createInfo) override;
		RHISamplerHandle createSampler(const RHISamplerCreateInfo& createInfo) override;

		//  Descriptor Set 생성
		RHIDescriptorSetLayoutHandle createDescriptorSetLayout(const RHIDescriptorSetLayoutCreateInfo& createInfo) override;
		RHIDescriptorPoolHandle createDescriptorPool(const RHIDescriptorPoolCreateInfo& createInfo) override;
		RHIDescriptorSetHandle allocateDescriptorSet(RHIDescriptorPoolHandle pool, RHIDescriptorSetLayoutHandle layout) override;
		
		void updateDescriptorSet(RHIDescriptorSetHandle set, uint32_t binding, RHIBufferHandle buffer, RHIDeviceSize offset, RHIDeviceSize range) override;
		void updateDescriptorSet(RHIDescriptorSetHandle set, uint32_t binding, RHIImageViewHandle imageView, RHISamplerHandle sampler) override;

		// 리소스 해제
		void destroyBuffer(RHIBufferHandle buffer) override;
		void destroyImage(RHIImageHandle image) override;
		void destroyShader(RHIShaderHandle shader) override;
		void destroyPipeline(RHIPipelineHandle pipeline) override;
		void destroyImageView(RHIImageViewHandle imageView) override;
		void destroySampler(RHISamplerHandle sampler) override;

		//  Descriptor Set 해제
		void destroyDescriptorSetLayout(RHIDescriptorSetLayoutHandle layout) override;
		void destroyDescriptorPool(RHIDescriptorPoolHandle pool) override;

		// 버퍼 매핑
		void* mapBuffer(RHIBufferHandle buffer) override;
		void unmapBuffer(RHIBufferHandle buffer) override;
		void flushBuffer(RHIBufferHandle buffer, RHIDeviceSize offset = 0, RHIDeviceSize size = 0) override;

		// 커맨드 기록
		void beginCommandRecording() override;
		void endCommandRecording() override;
		void submitCommands() override;

		// 드로우 커맨드
		void cmdBindPipeline(RHIPipelineHandle pipeline) override;
		void cmdBindVertexBuffer(RHIBufferHandle buffer, RHIDeviceSize offset = 0) override;
		void cmdBindIndexBuffer(RHIBufferHandle buffer, RHIDeviceSize offset = 0) override;
		void cmdBindDescriptorSets(RHIPipelineLayout* layout, const RHIDescriptorSetHandle* sets, uint32_t setCount) override;
		void cmdPushConstants(RHIPipelineLayout* layout, RHIShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues) override;
		void cmdSetViewport(const RHIViewport& viewport) override;
		void cmdSetScissor(const RHIRect2D& scissor) override;
		void cmdDraw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) override;
		void cmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) override;

		//  Descriptor Sets 바인딩 (Pipeline 사용)
		void cmdBindDescriptorSets(RHIPipelineHandle pipeline, uint32_t firstSet, const RHIDescriptorSetHandle* sets, uint32_t setCount) override;

		//  Push Constants (Pipeline 사용)
		void cmdPushConstants(RHIPipelineHandle pipeline, RHIShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues) override;

		//  Dynamic Rendering
		void cmdBeginRendering(uint32_t width, uint32_t height, RHIImageViewHandle colorAttachment, RHIImageViewHandle depthAttachment = {}) override;
		void cmdEndRendering() override;

		//  Image Layout Transition
		void cmdTransitionImageLayout(
			RHIImageHandle image,
			RHIImageLayout oldLayout,
			RHIImageLayout newLayout,
			RHIImageAspectFlagBits aspectMask = RHI_IMAGE_ASPECT_COLOR_BIT,
			uint32_t baseMipLevel = 0,
			uint32_t levelCount = 1,
			uint32_t baseArrayLayer = 0,
			uint32_t layerCount = 1
		) override;

		//  Buffer to Image Copy
		void cmdCopyBufferToImage(
			RHIBufferHandle srcBuffer,
			RHIImageHandle dstImage,
			RHIImageLayout dstImageLayout,
			uint32_t regionCount,
			const RHIBufferImageCopy* pRegions
		) override;

		//  Texture 생성 (Image + View + Sampler)
		RHITextureHandle createTexture(RHIImageHandle image, RHIImageViewHandle view, RHISamplerHandle sampler) override;
		void destroyTexture(RHITextureHandle texture) override;

		// API 타입
		RHIApiType getApiType() const override { return RHIApiType::Vulkan; }

		// Vulkan-specific public methods
		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	private:
		RHIInitInfo initInfo_;
		std::unique_ptr<VulkanContext> context_;
		std::unique_ptr<VulkanSwapchain> swapchain_;

		// 동기화 객체
		std::vector<VkSemaphore> imageAvailableSemaphores_;
		std::vector<VkSemaphore> renderFinishedSemaphores_;
		std::vector<VkFence> inFlightFences_;
		std::vector<VkFence> imagesInFlight_;  //  각 swapchain image가 사용 중인 fence 추적

		// 프레임 관리
		uint32_t currentFrameIndex_ = 0;
		uint32_t currentImageIndex_ = 0;
		uint32_t maxFramesInFlight_ = 2;

		// 표면
		VkSurfaceKHR surface_ = VK_NULL_HANDLE;

		// 커맨드 풀 및 버퍼
		std::unique_ptr<VulkanCommandPool> commandPool_;
		std::vector<VulkanCommandBuffer*> commandBuffers_;
		VkCommandPool transferCommandPool_ = VK_NULL_HANDLE;

		// 리소스 풀
		RHIResourcePool<RHIBuffer, RHIBufferHandle> bufferPool;
		RHIResourcePool<RHIImage, RHIImageHandle> imagePool;
		RHIResourcePool<RHIShader, RHIShaderHandle> shaderPool;
		RHIResourcePool<RHIPipeline, RHIPipelineHandle> pipelinePool;
		RHIResourcePool<RHIImageView, RHIImageViewHandle> imageViewPool;
		RHIResourcePool<RHISampler, RHISamplerHandle> samplerPool;
		RHIResourcePool<RHIDescriptorSet, RHIDescriptorSetHandle> descriptorSetPool;
		RHIResourcePool<RHIDescriptorSetLayout, RHIDescriptorSetLayoutHandle> descriptorSetLayoutPool;
		RHIResourcePool<RHIDescriptorPool, RHIDescriptorPoolHandle> descriptorPoolPool;
		RHIResourcePool<RHICommandBuffer, RHICommandBufferHandle> commandBufferPool;
		RHIResourcePool<RHITexture, RHITextureHandle> texturePool;

		// 스왑체인 이미지 뷰 핸들 캐싱
		std::vector<RHIImageViewHandle> swapchainImageViewHandles_;

		// 헬퍼 함수
		void createSyncObjects();
		void destroySyncObjects();
		void createSurface();
		void createSwapchain();
		void createTransferCommandPool();
		void destroySwapchain();
	};

} // namespace BinRenderer::Vulkan
