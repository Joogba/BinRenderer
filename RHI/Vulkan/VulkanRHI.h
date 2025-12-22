#pragma once

#include "../Core/RHI.h"
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
		uint32_t getCurrentImageIndex() const override;	// ✅ Swapchain image index 가져오기

		// 스왑체인 접근
		RHISwapchain* getSwapchain() const override { return swapchain_.get(); }
		RHIImageView* getSwapchainImageView(uint32_t index) const;

		// 리소스 생성
		RHIBuffer* createBuffer(const RHIBufferCreateInfo& createInfo) override;
		RHIImage* createImage(const RHIImageCreateInfo& createInfo) override;
		RHIShader* createShader(const RHIShaderCreateInfo& createInfo) override;
		RHIPipeline* createPipeline(const RHIPipelineCreateInfo& createInfo) override;
		RHIImageView* createImageView(RHIImage* image, const RHIImageViewCreateInfo& createInfo) override;
		RHISampler* createSampler(const RHISamplerCreateInfo& createInfo) override;

		// ✅ Descriptor Set 생성
		RHIDescriptorSetLayout* createDescriptorSetLayout(const RHIDescriptorSetLayoutCreateInfo& createInfo) override;
		RHIDescriptorPool* createDescriptorPool(const RHIDescriptorPoolCreateInfo& createInfo) override;
		RHIDescriptorSet* allocateDescriptorSet(RHIDescriptorPool* pool, RHIDescriptorSetLayout* layout) override;

		// 리소스 해제
		void destroyBuffer(RHIBuffer* buffer) override;
		void destroyImage(RHIImage* image) override;
		void destroyShader(RHIShader* shader) override;
		void destroyPipeline(RHIPipeline* pipeline) override;
		void destroyImageView(RHIImageView* imageView) override;
		void destroySampler(RHISampler* sampler) override;

		// ✅ Descriptor Set 해제
		void destroyDescriptorSetLayout(RHIDescriptorSetLayout* layout) override;
		void destroyDescriptorPool(RHIDescriptorPool* pool) override;

		// 버퍼 매핑
		void* mapBuffer(RHIBuffer* buffer) override;
		void unmapBuffer(RHIBuffer* buffer) override;
		void flushBuffer(RHIBuffer* buffer, RHIDeviceSize offset = 0, RHIDeviceSize size = 0) override;

		// 커맨드 기록
		void beginCommandRecording() override;
		void endCommandRecording() override;
		void submitCommands() override;

		// 드로우 커맨드
		void cmdBindPipeline(RHIPipeline* pipeline) override;
		void cmdBindVertexBuffer(RHIBuffer* buffer, RHIDeviceSize offset = 0) override;
		void cmdBindIndexBuffer(RHIBuffer* buffer, RHIDeviceSize offset = 0) override;
		void cmdBindDescriptorSets(RHIPipelineLayout* layout, RHIDescriptorSet** sets, uint32_t setCount) override;
		void cmdPushConstants(RHIPipelineLayout* layout, RHIShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues) override;
		void cmdSetViewport(const RHIViewport& viewport) override;
		void cmdSetScissor(const RHIRect2D& scissor) override;
		void cmdDraw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) override;
		void cmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) override;

		// ✅ Descriptor Sets 바인딩 (Pipeline 사용)
		void cmdBindDescriptorSets(RHIPipeline* pipeline, uint32_t firstSet, RHIDescriptorSet** sets, uint32_t setCount) override;

		// ✅ Push Constants (Pipeline 사용)
		void cmdPushConstants(RHIPipeline* pipeline, RHIShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues) override;

		// ✅ Dynamic Rendering
		void cmdBeginRendering(uint32_t width, uint32_t height, RHIImageView* colorAttachment, RHIImageView* depthAttachment = nullptr) override;
		void cmdEndRendering() override;

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
		std::vector<VkFence> imagesInFlight_;  // ✅ 각 swapchain image가 사용 중인 fence 추적

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

		// 헬퍼 함수
		void createSyncObjects();
		void destroySyncObjects();
		void createSurface();
		void createSwapchain();
		void createTransferCommandPool();
		void destroySwapchain();
	};

} // namespace BinRenderer::Vulkan
