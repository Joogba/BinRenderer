#pragma once

#include "../Core/RHI.h"
#include "../../Vulkan/Context.h"
#include "../../Vulkan/Swapchain.h"
#include "../../Vulkan/CommandBuffer.h"

#include <memory>
#include <vector>

namespace BinRenderer::Vulkan
{
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

        RHIBuffer* createBuffer(const RHIBufferCreateInfo& createInfo) override;
        RHIImage* createImage(const RHIImageCreateInfo& createInfo) override;
        RHIShader* createShader(const RHIShaderCreateInfo& createInfo) override;
        RHIPipeline* createPipeline(const RHIPipelineCreateInfo& createInfo) override;

        void destroyBuffer(RHIBuffer* buffer) override;
        void destroyImage(RHIImage* image) override;
        void destroyShader(RHIShader* shader) override;
        void destroyPipeline(RHIPipeline* pipeline) override;

        void beginCommandRecording() override;
        void endCommandRecording() override;
        void submitCommands() override;

        void cmdBindPipeline(RHIPipeline* pipeline) override;
        void cmdBindVertexBuffer(RHIBuffer* buffer, RHIDeviceSize offset = 0) override;
        void cmdBindIndexBuffer(RHIBuffer* buffer, RHIDeviceSize offset = 0) override;
        void cmdBindDescriptorSets(RHIPipelineLayout* layout, RHIDescriptorSet** sets, uint32_t setCount) override;
        void cmdSetViewport(const RHIViewport& viewport) override;
        void cmdSetScissor(const RHIRect2D& scissor) override;
        void cmdDraw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) override;
        void cmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) override;

        RHIApiType getApiType() const override { return RHIApiType::Vulkan; }

        // Vulkan 네이티브 접근
        Vulkan::Context* getContext() const { return context_.get(); }
        Vulkan::Swapchain* getSwapchain() const { return swapchain_.get(); }

    private:
        std::unique_ptr<Vulkan::Context> context_;
        std::unique_ptr<Vulkan::Swapchain> swapchain_;
        VkSurfaceKHR surface_{ VK_NULL_HANDLE };

        std::vector<Vulkan::CommandBuffer> commandBuffers_;
        
        std::vector<VkSemaphore> imageAvailableSemaphores_;
        std::vector<VkSemaphore> renderFinishedSemaphores_;
        std::vector<VkFence> inFlightFences_;

        RHIInitInfo initInfo_;
        uint32_t currentFrameIndex_{ 0 };
        uint32_t maxFramesInFlight_{ 2 };

        void createSurface();
        void createSyncObjects();
    };

} // namespace BinRenderer::Vulkan