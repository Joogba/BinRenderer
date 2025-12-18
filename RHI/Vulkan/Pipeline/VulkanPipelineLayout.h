#pragma once

#include "../../Pipeline/RHIPipelineLayout.h"
#include <vulkan/vulkan.h>

namespace BinRenderer::Vulkan
{
	/**
	 * @brief Vulkan 파이프라인 레이아웃 구현
   */
	class VulkanPipelineLayout : public RHIPipelineLayout
	{
	public:
		VulkanPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout);
		~VulkanPipelineLayout() override;

		// RHIPipelineLayout 인터페이스 구현
		uint32_t getSetLayoutCount() const override { return setLayoutCount_; }

		// Vulkan 네이티브 접근
		VkPipelineLayout getVkPipelineLayout() const { return pipelineLayout_; }

		void setSetLayoutCount(uint32_t count) { setLayoutCount_ = count; }

	private:
		VkDevice device_;
		VkPipelineLayout pipelineLayout_;
		uint32_t setLayoutCount_ = 0;
	};

} // namespace BinRenderer::Vulkan
