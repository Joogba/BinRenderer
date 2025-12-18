#pragma once

#include <vulkan/vulkan.h>

#include "../Core/RHI.h"
#include "Core/VulkanContext.h"
#include "Commands/VulkanCommandBuffer.h"
#include "Commands/VulkanCommandPool.h"
#include "Resources/VulkanBuffer.h"
#include "Resources/VulkanImage.h"
#include "Resources/VulkanShader.h"

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

		// RHI 인터페이스 구현
		bool initialize(const RHIInitInfo& initInfo) override;
		void shutdown() override;
		void waitIdle() override;

		bool beginFrame(uint32_t& imageIndex) override;
		void endFrame(uint32_t imageIndex) override;
		uint32_t getCurrentFrameIndex() const override;

		// 스왑체인 접근 (임시: nullptr 반환)
		RHISwapchain* getSwapchain() const override { return nullptr; } // TODO: VulkanSwapchain 래퍼 구현

		RHIBuffer* createBuffer(const RHIBufferCreateInfo& createInfo) override;
		RHIImage* createImage(const RHIImageCreateInfo& createInfo) override;
		RHIShader* createShader(const RHIShaderCreateInfo& createInfo) override;
		RHIPipeline* createPipeline(const RHIPipelineCreateInfo& createInfo) override;
		RHIImageView* createImageView(RHIImage* image, const RHIImageViewCreateInfo& createInfo) override;
		RHISampler* createSampler(const RHISamplerCreateInfo& createInfo) override;

		void destroyBuffer(RHIBuffer* buffer) override;
		void destroyImage(RHIImage* image) override;
		void destroyShader(RHIShader* shader) override;
		void destroyPipeline(RHIPipeline* pipeline) override;
		void destroyImageView(RHIImageView* imageView) override;
		void destroySampler(RHISampler* sampler) override;

		// 버퍼 매핑
		void* mapBuffer(RHIBuffer* buffer) override;
		void unmapBuffer(RHIBuffer* buffer) override;
		void flushBuffer(RHIBuffer* buffer, RHIDeviceSize offset = 0, RHIDeviceSize size = 0) override;

		void beginCommandRecording() override;
		void endCommandRecording() override;
		void submitCommands() override;

		void cmdBindPipeline(RHIPipeline* pipeline) override;
		void cmdBindVertexBuffer(RHIBuffer* buffer, RHIDeviceSize offset = 0) override;
		void cmdBindIndexBuffer(RHIBuffer* buffer, RHIDeviceSize offset = 0) override;
		void cmdBindDescriptorSets(RHIPipelineLayout* layout, RHIDescriptorSet** sets, uint32_t setCount) override;
		void cmdPushConstants(RHIPipelineLayout* layout, RHIShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues) override;
		void cmdSetViewport(const RHIViewport& viewport) override;
		void cmdSetScissor(const RHIRect2D& scissor) override;
		void cmdDraw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) override;
		void cmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) override;

		RHIApiType getApiType() const override { return RHIApiType::Vulkan; }

		// Vulkan 네이티브 접근 (RHI/Vulkan 클래스)
		VulkanContext* getContext() const { return context_.get(); }
		VkSwapchainKHR getVulkanSwapchain() const { return swapchain_; }

		// 단일 시간 커맨드 헬퍼 (텍스처 로딩 등에 사용)
		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);
		VkCommandPool getTransferCommandPool() const { return transferCommandPool_; }

	private:
		// RHI/Vulkan 클래스 사용
		std::unique_ptr<VulkanContext> context_;
		std::unique_ptr<VulkanCommandPool> commandPool_;

		// Vulkan 네이티브 객체 (스왑체인은 나중에 래퍼로 교체)
		VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
		VkSurfaceKHR surface_ = VK_NULL_HANDLE;
		std::vector<VkImage> swapchainImages_;
		std::vector<VkImageView> swapchainImageViews_;
		VkFormat swapchainImageFormat_ = VK_FORMAT_UNDEFINED;
		VkExtent2D swapchainExtent_ = {};

		// 커맨드 버퍼 (RHI/Vulkan 래퍼)
		std::vector<VulkanCommandBuffer*> commandBuffers_;

		// 동기화 객체
		std::vector<VkSemaphore> imageAvailableSemaphores_;
		std::vector<VkSemaphore> renderFinishedSemaphores_;
		std::vector<VkFence> inFlightFences_;

		VkCommandPool transferCommandPool_ = VK_NULL_HANDLE;

		RHIInitInfo initInfo_;
		uint32_t currentFrameIndex_ = 0;
		uint32_t maxFramesInFlight_ = 2;
		uint32_t currentImageIndex_ = 0;

		void createSurface();
		void createSwapchain();
		void createSwapchainImageViews();
		void destroySwapchain();
		void createSyncObjects();
		void createTransferCommandPool();
	};

} // namespace BinRenderer::Vulkan