#pragma once

#include "../../Resources/RHIShader.h"
#include "RHI/Structs/RHIStructs.h"
#include <vulkan/vulkan.h>

namespace BinRenderer::Vulkan
{
	/**
	 * @brief Vulkan 셰이더 구현
  */
	class VulkanShader : public RHIShader
	{
	public:
		VulkanShader(VkDevice device);
		~VulkanShader() override;

		bool create(const RHIShaderCreateInfo& createInfo);
		void destroy();

		// RHIShader 인터페이스 구현
		RHIShaderStageFlags getStage() const override { return stage_; }
		const std::string& getName() const override { return name_; }
		const std::string& getEntryPoint() const override { return entryPoint_; }

		// Vulkan 네이티브 접근
		VkShaderModule getVkShaderModule() const { return shaderModule_; }
		VkPipelineShaderStageCreateInfo getStageCreateInfo() const;

	private:
		VkDevice device_;
		VkShaderModule shaderModule_ = VK_NULL_HANDLE;

		RHIShaderStageFlags stage_ = 0;
		std::string name_;
		std::string entryPoint_;
	};

} // namespace BinRenderer::Vulkan
