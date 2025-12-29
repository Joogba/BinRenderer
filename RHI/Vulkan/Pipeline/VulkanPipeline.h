#pragma once

#include "RHI/Core/RHIHandle.h"
#include "../../Pipeline/RHIPipeline.h"
#include "../../Structs/RHIStructs.h"
#include "../Resources/VulkanShader.h"
#include "VulkanPipelineLayout.h"
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
		VulkanPipeline(VkDevice device, const RHIPipelineCreateInfo& createInfo, const std::vector<VulkanShader*>& shaders, RHIPipelineLayout* layout);
		~VulkanPipeline() override;

		void destroy();

		// RHIPipeline 인터페이스 구현
		RHIPipelineBindPoint getBindPoint() const override { return bindPoint_; }
		RHIPipelineLayout* getLayout() const override { return layout_; }
		RHIRenderPass* getRenderPass() const override { return reinterpret_cast<RHIRenderPass*>(renderPass_); }

		// Vulkan 네이티브 접근
		VkPipeline getVkPipeline() const { return pipeline_; }
		VkPipelineLayout getVkPipelineLayout() const;

	private:
		VkDevice device_;
		VkPipeline pipeline_ = VK_NULL_HANDLE;
		RHIPipelineLayout* layout_ = nullptr;
		VulkanRenderPass* renderPass_ = nullptr;
		RHIPipelineBindPoint bindPoint_ = RHI_PIPELINE_BIND_POINT_GRAPHICS;

		bool createGraphicsPipeline(const RHIPipelineCreateInfo& createInfo, const std::vector<VulkanShader*>& shaders);
	};

} // namespace BinRenderer::Vulkan
