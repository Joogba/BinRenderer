#pragma once

#include <vulkan/vulkan.h>
#include "../../Commands/RHICommandBuffer.h"

namespace BinRenderer::Vulkan
{
	class VulkanCommandPool;

	/**
	 * @brief Vulkan 커맨드 버퍼 구현
	 */
	class VulkanCommandBuffer : public RHICommandBuffer
	{
	public:
		VulkanCommandBuffer(VkDevice device, VkCommandBuffer commandBuffer, VulkanCommandPool* pool);
		~VulkanCommandBuffer() override;

		// RHICommandBuffer 인터페이스 구현
		void begin() override;
		void end() override;
		void reset() override;

		void bindPipeline(RHIPipeline* pipeline) override;
		void bindVertexBuffer(uint32_t binding, RHIBuffer* buffer, RHIDeviceSize offset = 0) override;
		void bindIndexBuffer(RHIBuffer* buffer, RHIDeviceSize offset = 0) override;
		void bindDescriptorSets(RHIPipelineLayout* layout, uint32_t firstSet, uint32_t setCount, RHIDescriptorSet** sets) override;

		void draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) override;
		void drawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) override;
		void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;

		// Vulkan 네이티브 접근
		VkCommandBuffer getVkCommandBuffer() const { return commandBuffer_; }

	private:
		VkDevice device_;
		VkCommandBuffer commandBuffer_;
		VulkanCommandPool* pool_;
		bool isRecording_ = false;
	};

} // namespace BinRenderer::Vulkan
