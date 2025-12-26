#include "VulkanShader.h"
#include "Core/Logger.h"

namespace BinRenderer::Vulkan
{
	VulkanShader::VulkanShader(VkDevice device)
		: device_(device)
	{
	}

	VulkanShader::~VulkanShader()
	{
		destroy();
	}

	bool VulkanShader::create(const RHIShaderCreateInfo& createInfo)
	{
		stage_ = createInfo.stage;
		name_ = createInfo.name;
		entryPoint_ = createInfo.entryPoint;

		// Vulkan 셰이더 모듈 생성
		VkShaderModuleCreateInfo moduleInfo{};
		moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleInfo.codeSize = createInfo.code.size() * sizeof(uint32_t);
		moduleInfo.pCode = createInfo.code.data();

		if (vkCreateShaderModule(device_, &moduleInfo, nullptr, &shaderModule_) != VK_SUCCESS)
		{
			return false;
		}

		return true;
	}

	void VulkanShader::destroy()
	{
		if (shaderModule_ != VK_NULL_HANDLE)
		{
			vkDestroyShaderModule(device_, shaderModule_, nullptr);
			shaderModule_ = VK_NULL_HANDLE;
		}
	}

	VkPipelineShaderStageCreateInfo VulkanShader::getStageCreateInfo() const
	{
		VkPipelineShaderStageCreateInfo stageInfo{};
		stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageInfo.stage = static_cast<VkShaderStageFlagBits>(stage_);
		stageInfo.module = shaderModule_;
		stageInfo.pName = entryPoint_.c_str();
		return stageInfo;
	}

} // namespace BinRenderer::Vulkan
