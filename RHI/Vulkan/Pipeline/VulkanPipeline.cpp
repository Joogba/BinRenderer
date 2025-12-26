#include "VulkanPipeline.h"
#include "../Resources/VulkanShader.h"
#include "VulkanRenderPass.h"
#include "VulkanDescriptor.h"
#include "../../Structs/RHIStructs.h"
#include "Core/Logger.h"

namespace BinRenderer::Vulkan
{
	VulkanPipeline::VulkanPipeline(VkDevice device)
		: device_(device)
	{
	}

	VulkanPipeline::~VulkanPipeline()
	{
		destroy();
	}

	bool VulkanPipeline::create(const RHIPipelineCreateInfo& createInfo, const std::vector<RHIDescriptorSetLayout*>& resolvedLayouts, vector<VulkanShader*>& shaders)
	{
		// 렌더 패스 저장
		if (createInfo.renderPass)
		{
			renderPass_ = static_cast<VulkanRenderPass*>(createInfo.renderPass);
		}

		// 파이프라인 레이아웃 생성
		if (!createPipelineLayout(createInfo, resolvedLayouts))
		{
			return false;
		}

		// 그래픽스 파이프라인 생성
		if (!createGraphicsPipeline(createInfo,shaders))
		{
			return false;
		}

		bindPoint_ = RHI_PIPELINE_BIND_POINT_GRAPHICS;
		return true;
	}

	void VulkanPipeline::destroy()
	{
		if (pipeline_ != VK_NULL_HANDLE)
		{
			vkDestroyPipeline(device_, pipeline_, nullptr);
			pipeline_ = VK_NULL_HANDLE;
		}

		if (pipelineLayout_ != VK_NULL_HANDLE)
		{
			vkDestroyPipelineLayout(device_, pipelineLayout_, nullptr);
			pipelineLayout_ = VK_NULL_HANDLE;
		}
	}

	bool VulkanPipeline::createPipelineLayout(const RHIPipelineCreateInfo& createInfo, const std::vector<RHIDescriptorSetLayout*>& resolvedLayouts)
	{
		//  Descriptor Set Layouts 변환
		std::vector<VkDescriptorSetLayout> vkDescriptorSetLayouts;
		vkDescriptorSetLayouts.reserve(resolvedLayouts.size());
		
		for (auto* layout : resolvedLayouts)
		{
			if (layout)
			{
				auto* vulkanLayout = static_cast<VulkanDescriptorSetLayout*>(layout);
				vkDescriptorSetLayouts.push_back(vulkanLayout->getVkDescriptorSetLayout());
			}
		}

		//  Push Constant Ranges 변환
		std::vector<VkPushConstantRange> vkPushConstantRanges;
		vkPushConstantRanges.reserve(createInfo.pushConstantRanges.size());
		
		for (const auto& range : createInfo.pushConstantRanges)
		{
			VkPushConstantRange vkRange{};
			vkRange.stageFlags = static_cast<VkShaderStageFlags>(range.stageFlags);
			vkRange.offset = range.offset;
			vkRange.size = range.size;
			vkPushConstantRanges.push_back(vkRange);
		}

		// Pipeline Layout 생성
		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = static_cast<uint32_t>(vkDescriptorSetLayouts.size());
		layoutInfo.pSetLayouts = vkDescriptorSetLayouts.empty() ? nullptr : vkDescriptorSetLayouts.data();
		layoutInfo.pushConstantRangeCount = static_cast<uint32_t>(vkPushConstantRanges.size());
		layoutInfo.pPushConstantRanges = vkPushConstantRanges.empty() ? nullptr : vkPushConstantRanges.data();

		if (vkCreatePipelineLayout(device_, &layoutInfo, nullptr, &pipelineLayout_) != VK_SUCCESS)
		{
			printLog("❌ ERROR: Failed to create pipeline layout");
			return false;
		}

		printLog(" Pipeline layout created with {} descriptor set layouts, {} push constant ranges",
			vkDescriptorSetLayouts.size(), vkPushConstantRanges.size());

		return true;
	}

	bool VulkanPipeline::createGraphicsPipeline(const RHIPipelineCreateInfo& createInfo, vector<VulkanShader*>& shaders)
	{
		// 셰이더 스테이지
		// rhi의 ShaderPool에서 get한다음 캐스팅해야함
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		for (auto* shader : shaders)
			shaderStages.push_back(shader->getStageCreateInfo());

		// 버텍스 입력 상태
		std::vector<VkVertexInputBindingDescription> vertexBindings;
		std::vector<VkVertexInputAttributeDescription> vertexAttributes;

		// 기본 vertex bindings/attributes 추가
		for (const auto& binding : createInfo.vertexInputState.bindings)
		{
			VkVertexInputBindingDescription vkBinding{};
			vkBinding.binding = binding.binding;
			vkBinding.stride = binding.stride;
			vkBinding.inputRate = static_cast<VkVertexInputRate>(binding.inputRate);
			vertexBindings.push_back(vkBinding);
		}

	
		for (const auto& attribute : createInfo.vertexInputState.attributes)
		{
			VkVertexInputAttributeDescription vkAttribute{};
			vkAttribute.location = attribute.location;
			vkAttribute.binding = attribute.binding;
			vkAttribute.format = static_cast<VkFormat>(attribute.format);
			vkAttribute.offset = attribute.offset;
			vertexAttributes.push_back(vkAttribute);
		}

		//  GPU Instancing: enableInstancing이 true면 instance binding/attributes 자동 추가
		if (createInfo.enableInstancing)
		{
			// Instance binding 추가 (binding = 1)
			auto instanceBinding = RHIInstanceHelper::getInstanceBinding();
			VkVertexInputBindingDescription vkInstanceBinding{};
			vkInstanceBinding.binding = instanceBinding.binding;
			vkInstanceBinding.stride = instanceBinding.stride;
			vkInstanceBinding.inputRate = static_cast<VkVertexInputRate>(instanceBinding.inputRate);
			vertexBindings.push_back(vkInstanceBinding);

			// Instance attributes 추가 (location 10-14)
			auto instanceAttributes = RHIInstanceHelper::getInstanceAttributes();
			for (const auto& attr : instanceAttributes)
			{
				VkVertexInputAttributeDescription vkAttr{};
				vkAttr.location = attr.location;
				vkAttr.binding = attr.binding;
				vkAttr.format = static_cast<VkFormat>(attr.format);
				vkAttr.offset = attr.offset;
				vertexAttributes.push_back(vkAttr);
			}

			printLog(" GPU Instancing enabled: {} bindings, {} attributes",
				vertexBindings.size(), vertexAttributes.size());
		}

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindings.size());
		vertexInputInfo.pVertexBindingDescriptions = vertexBindings.data();
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributes.size());
		vertexInputInfo.pVertexAttributeDescriptions = vertexAttributes.data();

		// 입력 어셈블리 상태
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = static_cast<VkPrimitiveTopology>(createInfo.inputAssemblyState.topology);
		inputAssembly.primitiveRestartEnable = createInfo.inputAssemblyState.primitiveRestartEnable ? VK_TRUE : VK_FALSE;

		// 뷰포트 상태 (다이나믹)
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = createInfo.viewportState.viewportCount;
		viewportState.scissorCount = createInfo.viewportState.scissorCount;

		// 래스터라이제이션 상태
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = createInfo.rasterizationState.depthClampEnable ? VK_TRUE : VK_FALSE;
		rasterizer.rasterizerDiscardEnable = createInfo.rasterizationState.rasterizerDiscardEnable ? VK_TRUE : VK_FALSE;
		rasterizer.polygonMode = static_cast<VkPolygonMode>(createInfo.rasterizationState.polygonMode);
		rasterizer.lineWidth = createInfo.rasterizationState.lineWidth;
		rasterizer.cullMode = static_cast<VkCullModeFlags>(createInfo.rasterizationState.cullMode);
		rasterizer.frontFace = static_cast<VkFrontFace>(createInfo.rasterizationState.frontFace);
		rasterizer.depthBiasEnable = createInfo.rasterizationState.depthBiasEnable ? VK_TRUE : VK_FALSE;

		// 멀티샘플 상태
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = createInfo.multisampleState.sampleShadingEnable ? VK_TRUE : VK_FALSE;
		multisampling.rasterizationSamples = static_cast<VkSampleCountFlagBits>(createInfo.multisampleState.rasterizationSamples);
		multisampling.minSampleShading = createInfo.multisampleState.minSampleShading;

		// 깊이 스텐실 상태
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = createInfo.depthStencilState.depthTestEnable ? VK_TRUE : VK_FALSE;
		depthStencil.depthWriteEnable = createInfo.depthStencilState.depthWriteEnable ? VK_TRUE : VK_FALSE;
		depthStencil.depthCompareOp = static_cast<VkCompareOp>(createInfo.depthStencilState.depthCompareOp);
		depthStencil.stencilTestEnable = createInfo.depthStencilState.stencilTestEnable ? VK_TRUE : VK_FALSE;

		// 컬러 블렌드 상태
		std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
		for (const auto& attachment : createInfo.colorBlendState.attachments)
		{
			VkPipelineColorBlendAttachmentState colorBlendAttachment{};
			colorBlendAttachment.colorWriteMask = attachment.colorWriteMask;
			colorBlendAttachment.blendEnable = attachment.blendEnable ? VK_TRUE : VK_FALSE;
			colorBlendAttachment.srcColorBlendFactor = static_cast<VkBlendFactor>(attachment.srcColorBlendFactor);
			colorBlendAttachment.dstColorBlendFactor = static_cast<VkBlendFactor>(attachment.dstColorBlendFactor);
			colorBlendAttachment.colorBlendOp = static_cast<VkBlendOp>(attachment.colorBlendOp);
			colorBlendAttachment.srcAlphaBlendFactor = static_cast<VkBlendFactor>(attachment.srcAlphaBlendFactor);
			colorBlendAttachment.dstAlphaBlendFactor = static_cast<VkBlendFactor>(attachment.dstAlphaBlendFactor);
			colorBlendAttachment.alphaBlendOp = static_cast<VkBlendOp>(attachment.alphaBlendOp);
			colorBlendAttachments.push_back(colorBlendAttachment);
		}

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = createInfo.colorBlendState.logicOpEnable ? VK_TRUE : VK_FALSE;
		colorBlending.logicOp = static_cast<VkLogicOp>(createInfo.colorBlendState.logicOp);
		colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
		colorBlending.pAttachments = colorBlendAttachments.data();

		// 다이나믹 스테이트
		std::vector<VkDynamicState> dynamicStates;
		for (auto state : createInfo.dynamicStates)
		{
			dynamicStates.push_back(static_cast<VkDynamicState>(state));
		}

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		// ========================================
		//  Dynamic Rendering (Vulkan 1.3+)
		// ========================================
		VkPipelineRenderingCreateInfo renderingInfo{};
		std::vector<VkFormat> colorFormats;

		if (createInfo.useDynamicRendering)
		{
			// Color attachment formats
			for (auto format : createInfo.colorAttachmentFormats)
			{
				colorFormats.push_back(static_cast<VkFormat>(format));
			}

			renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
			renderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorFormats.size());
			renderingInfo.pColorAttachmentFormats = colorFormats.empty() ? nullptr : colorFormats.data();
			renderingInfo.depthAttachmentFormat = static_cast<VkFormat>(createInfo.depthAttachmentFormat);
			renderingInfo.stencilAttachmentFormat = static_cast<VkFormat>(createInfo.stencilAttachmentFormat);

			printLog(" Dynamic Rendering enabled: {} color attachments, depth format: {}",
				colorFormats.size(), static_cast<int>(createInfo.depthAttachmentFormat));
		}

		// 그래픽스 파이프라인 생성
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = dynamicStates.empty() ? nullptr : &dynamicState;
		pipelineInfo.layout = pipelineLayout_;
		
		//  Dynamic Rendering 사용 시 pNext에 renderingInfo 연결, 아니면 legacy renderPass 사용
		if (createInfo.useDynamicRendering)
		{
			pipelineInfo.pNext = &renderingInfo;
			pipelineInfo.renderPass = VK_NULL_HANDLE;
			pipelineInfo.subpass = 0;
		}
		else
		{
			pipelineInfo.pNext = nullptr;
			pipelineInfo.renderPass = renderPass_ ? renderPass_->getVkRenderPass() : VK_NULL_HANDLE;
			pipelineInfo.subpass = createInfo.subpass;
		}

		if (vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline_) != VK_SUCCESS)
		{
			return false;
		}

		return true;
	}

} // namespace BinRenderer::Vulkan
