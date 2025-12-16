#pragma once

#include "../../Pipeline/RHIPipeline.h"
#include "../../Structs/RHIPipelineCreateInfo.h"
#include "../Resources/VulkanShader.h"
#include <vulkan/vulkan.h>
#include <vector>

namespace BinRenderer::Vulkan
{
	class VulkanRenderPass;

	/**
	 * @brief Vulkan 파이프라인 구현
  */
	class VulkanPipeline : public RHIPipeline
	{
	public:
		VulkanPipeline(VkDevice device);
		~VulkanPipeline() override;

		bool create(const RHIPipelineCreateInfo& createInfo);
		void destroy();

		// RHIPipeline 인터페이스 구현
		RHIPipelineBindPoint getBindPoint() const override { return bindPoint_; }
		RHIPipelineLayout* getLayout() const override { return nullptr; } // TODO: VulkanPipelineLayout 구현
		RHIRenderPass* getRenderPass() const override { return reinterpret_cast<RHIRenderPass*>(renderPass_); }

		// Vulkan 네이티브 접근
		VkPipeline getVkPipeline() const { return pipeline_; }
		VkPipelineLayout getVkPipelineLayout() const { return pipelineLayout_; }

	private:
		VkDevice device_;
		VkPipeline pipeline_ = VK_NULL_HANDLE;
		VkPipelineLayout pipelineLayout_ = VK_NULL_HANDLE;
		VulkanRenderPass* renderPass_ = nullptr;
		RHIPipelineBindPoint bindPoint_ = RHI_PIPELINE_BIND_POINT_GRAPHICS;

		bool createPipelineLayout(const RHIPipelineCreateInfo& createInfo);
		bool createGraphicsPipeline(const RHIPipelineCreateInfo& createInfo);
	};

} // namespace BinRenderer::Vulkan
