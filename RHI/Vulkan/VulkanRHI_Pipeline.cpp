#include "VulkanRHI.h"
#include "Resources/VulkanShader.h"
#include "Pipeline/VulkanPipeline.h"
#include "Pipeline/VulkanPipelineLayout.h"
#include "Pipeline/VulkanDescriptor.h"
#include "Resources/VulkanBuffer.h" // For updateDescriptorSet
#include "Resources/VulkanImage.h" // For updateDescriptorSet
#include "Resources/VulkanSampler.h" // For updateDescriptorSet
#include "Conversion/TypeConversion.h"
#include "Core/Logger.h"
#include "RHI/RHIPipelineStructs.h" // For RHIInstanceHelper

namespace BinRenderer::Vulkan
{
	RHIShaderHandle VulkanRHI::createShader(const RHIShaderCreateInfo& createInfo)
	{
		auto* vulkanShader = new VulkanShader(context_->getDevice());
		if (!vulkanShader->create(createInfo))
		{
			delete vulkanShader;
			return {};
		}
		return shaderPool.insert(vulkanShader);
	}

	RHIPipelineLayoutHandle VulkanRHI::createPipelineLayout(const RHIPipelineLayoutCreateInfo& createInfo)
	{
		//  Descriptor Set Layouts 변환
		std::vector<VkDescriptorSetLayout> vkDescriptorSetLayouts;
		vkDescriptorSetLayouts.reserve(createInfo.setLayouts.size());
		
		for (const auto& handle : createInfo.setLayouts)
		{
			RHIDescriptorSetLayout* layout = descriptorSetLayoutPool.get(handle);
			if (layout)
			{
				auto* vulkanLayout = static_cast<VulkanDescriptorSetLayout*>(layout);
				vkDescriptorSetLayouts.push_back(vulkanLayout->getVkDescriptorSetLayout());
			}
			else
			{
				printLog("❌ ERROR: Invalid descriptor set layout handle in createPipelineLayout");
				return {};
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

		VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;
		if (vkCreatePipelineLayout(context_->getDevice(), &layoutInfo, nullptr, &vkPipelineLayout) != VK_SUCCESS)
		{
			printLog("❌ ERROR: Failed to create pipeline layout");
			return {};
		}

		auto* vulkanPipelineLayout = new VulkanPipelineLayout(context_->getDevice(), vkPipelineLayout);
		vulkanPipelineLayout->setSetLayoutCount(static_cast<uint32_t>(vkDescriptorSetLayouts.size()));

		return pipelineLayoutPool.insert(vulkanPipelineLayout);
	}

	RHIPipelineHandle VulkanRHI::createPipeline(const RHIPipelineCreateInfo& createInfo)
	{
		// 1. Pipeline Layout 핸들 해석 (Private Pool 접근)
		RHIPipelineLayout* layout = pipelineLayoutPool.get(createInfo.layout);
		if (!layout)
		{
			printLog("❌ ERROR: Invalid pipeline layout handle in createPipeline");
			return {};
		}
		VulkanPipelineLayout* vulkanLayout = static_cast<VulkanPipelineLayout*>(layout);

		// 2. Shader 핸들 해석 (Private Pool 접근)
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		for (const auto& shaderHandle : createInfo.shaderStages)
		{
			RHIShader* shader = shaderPool.get(shaderHandle);
			if (!shader)
			{
				printLog("ERROR: Invalid shader handle in createPipeline");
				return {};
			}
			VulkanShader* vulkanShader = static_cast<VulkanShader*>(shader);
			shaderStages.push_back(vulkanShader->getStageCreateInfo());
		}

		// 3. Status Setup
		// Vertex Input State
		std::vector<VkVertexInputBindingDescription> vertexBindings;
		std::vector<VkVertexInputAttributeDescription> vertexAttributes;

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

		// GPU Instancing
		if (createInfo.enableInstancing)
		{
			auto instanceBinding = RHIInstanceHelper::getInstanceBinding();
			VkVertexInputBindingDescription vkInstanceBinding{};
			vkInstanceBinding.binding = instanceBinding.binding;
			vkInstanceBinding.stride = instanceBinding.stride;
			vkInstanceBinding.inputRate = static_cast<VkVertexInputRate>(instanceBinding.inputRate);
			vertexBindings.push_back(vkInstanceBinding);

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
		}

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindings.size());
		vertexInputInfo.pVertexBindingDescriptions = vertexBindings.data();
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributes.size());
		vertexInputInfo.pVertexAttributeDescriptions = vertexAttributes.data();

		// Input Assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = static_cast<VkPrimitiveTopology>(createInfo.inputAssemblyState.topology);
		inputAssembly.primitiveRestartEnable = createInfo.inputAssemblyState.primitiveRestartEnable ? VK_TRUE : VK_FALSE;

		// Viewport
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = createInfo.viewportState.viewportCount;
		viewportState.scissorCount = createInfo.viewportState.scissorCount;

		// Rasterization
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = createInfo.rasterizationState.depthClampEnable ? VK_TRUE : VK_FALSE;
		rasterizer.rasterizerDiscardEnable = createInfo.rasterizationState.rasterizerDiscardEnable ? VK_TRUE : VK_FALSE;
		rasterizer.polygonMode = static_cast<VkPolygonMode>(createInfo.rasterizationState.polygonMode);
		rasterizer.lineWidth = createInfo.rasterizationState.lineWidth;
		rasterizer.cullMode = static_cast<VkCullModeFlags>(createInfo.rasterizationState.cullMode);
		rasterizer.frontFace = static_cast<VkFrontFace>(createInfo.rasterizationState.frontFace);
		rasterizer.depthBiasEnable = createInfo.rasterizationState.depthBiasEnable ? VK_TRUE : VK_FALSE;

		// Multisample
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = createInfo.multisampleState.sampleShadingEnable ? VK_TRUE : VK_FALSE;
		multisampling.rasterizationSamples = static_cast<VkSampleCountFlagBits>(createInfo.multisampleState.rasterizationSamples);
		multisampling.minSampleShading = createInfo.multisampleState.minSampleShading;

		// Depth Stencil
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = createInfo.depthStencilState.depthTestEnable ? VK_TRUE : VK_FALSE;
		depthStencil.depthWriteEnable = createInfo.depthStencilState.depthWriteEnable ? VK_TRUE : VK_FALSE;
		depthStencil.depthCompareOp = static_cast<VkCompareOp>(createInfo.depthStencilState.depthCompareOp);
		depthStencil.stencilTestEnable = createInfo.depthStencilState.stencilTestEnable ? VK_TRUE : VK_FALSE;

		// Color Blend
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

		// Dynamic States
		std::vector<VkDynamicState> dynamicStates;
		for (auto state : createInfo.dynamicStates)
		{
			dynamicStates.push_back(static_cast<VkDynamicState>(state));
		}

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		// Dynamic Rendering
		VkPipelineRenderingCreateInfo renderingInfo{};
		std::vector<VkFormat> colorFormats;

		if (createInfo.useDynamicRendering)
		{
			for (auto format : createInfo.colorAttachmentFormats)
			{
				colorFormats.push_back(static_cast<VkFormat>(format));
			}

			renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
			renderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorFormats.size());
			renderingInfo.pColorAttachmentFormats = colorFormats.empty() ? nullptr : colorFormats.data();
			renderingInfo.depthAttachmentFormat = static_cast<VkFormat>(createInfo.depthAttachmentFormat);
			renderingInfo.stencilAttachmentFormat = static_cast<VkFormat>(createInfo.stencilAttachmentFormat);
		}

		// Graphics Pipeline Creation
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
		pipelineInfo.layout = vulkanLayout->getVkPipelineLayout();
		
		if (createInfo.useDynamicRendering)
		{
			pipelineInfo.pNext = &renderingInfo;
			pipelineInfo.renderPass = VK_NULL_HANDLE;
		}
		else
		{
			pipelineInfo.pNext = nullptr;
			pipelineInfo.renderPass = VK_NULL_HANDLE;
		}

		VkPipeline pipeline = VK_NULL_HANDLE;
		if (vkCreateGraphicsPipelines(context_->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
		{
			printLog("❌ ERROR: Failed to create graphics pipeline");
			return {};
		}

		// Create Wrapper
		auto* vulkanPipeline = new VulkanPipeline(context_->getDevice(), pipeline, layout, RHI_PIPELINE_BIND_POINT_GRAPHICS);
		return pipelinePool.insert(vulkanPipeline);
	}

	RHIDescriptorSetLayoutHandle VulkanRHI::createDescriptorSetLayout(const RHIDescriptorSetLayoutCreateInfo& createInfo)
	{
		auto* layout = new VulkanDescriptorSetLayout(context_->getDevice());
		
		std::vector<VkDescriptorSetLayoutBinding> vkBindings;
		vkBindings.reserve(createInfo.bindings.size());
		
		for (const auto& binding : createInfo.bindings)
		{
			VkDescriptorSetLayoutBinding vkBinding{};
			vkBinding.binding = binding.binding;
			vkBinding.descriptorType = static_cast<VkDescriptorType>(binding.descriptorType);
			vkBinding.descriptorCount = binding.descriptorCount;
			vkBinding.stageFlags = static_cast<VkShaderStageFlags>(binding.stageFlags);
			vkBinding.pImmutableSamplers = nullptr;
			
			vkBindings.push_back(vkBinding);
		}
		
		if (!layout->create(vkBindings))
		{
			delete layout;
			return {};
		}
		
		return descriptorSetLayoutPool.insert(layout);
	}

	RHIDescriptorPoolHandle VulkanRHI::createDescriptorPool(const RHIDescriptorPoolCreateInfo& createInfo)
	{
		auto* pool = new VulkanDescriptorPool(context_->getDevice());
		
		std::vector<VkDescriptorPoolSize> vkPoolSizes;
		vkPoolSizes.reserve(createInfo.poolSizes.size());
		
		for (const auto& poolSize : createInfo.poolSizes)
		{
			VkDescriptorPoolSize vkPoolSize{};
			vkPoolSize.type = static_cast<VkDescriptorType>(poolSize.type);
			vkPoolSize.descriptorCount = poolSize.descriptorCount;
			
			vkPoolSizes.push_back(vkPoolSize);
		}
		
		if (!pool->create(createInfo.maxSets, vkPoolSizes))
		{
			delete pool;
			return {};
		}
		
		return descriptorPoolPool.insert(pool);
	}

	RHIDescriptorSetHandle VulkanRHI::allocateDescriptorSet(RHIDescriptorPoolHandle poolHandle, RHIDescriptorSetLayoutHandle layoutHandle)
	{
		RHIDescriptorPool* pool = descriptorPoolPool.get(poolHandle);
		RHIDescriptorSetLayout* layout = descriptorSetLayoutPool.get(layoutHandle);

		if (!pool || !layout)
		{
			printLog("❌ ERROR: Invalid pool or layout in allocateDescriptorSet");
			return {};
		}
		
		auto* vulkanPool = static_cast<VulkanDescriptorPool*>(pool);
		RHIDescriptorSet* set = vulkanPool->allocateDescriptorSet(layout);
		if (!set) return {};

		return descriptorSetPool.insert(set);
	}

	void VulkanRHI::updateDescriptorSet(RHIDescriptorSetHandle setHandle, uint32_t binding, RHIBufferHandle bufferHandle, size_t offset, size_t range)
	{
		RHIDescriptorSet* set = descriptorSetPool.get(setHandle);
		RHIBuffer* buffer = bufferPool.get(bufferHandle);

		if (set && buffer)
		{
			set->updateBuffer(binding, buffer, offset, range);
		}
		else
		{
			printLog("❌ ERROR: Invalid set or buffer handle in updateDescriptorSet (Buffer)");
		}
	}

	void VulkanRHI::updateDescriptorSet(RHIDescriptorSetHandle setHandle, uint32_t binding, RHIImageViewHandle imageViewHandle, RHISamplerHandle samplerHandle)
	{
		RHIDescriptorSet* set = descriptorSetPool.get(setHandle);
		RHIImageView* imageView = imageViewPool.get(imageViewHandle);
		RHISampler* sampler = samplerPool.get(samplerHandle);

		if (set && imageView)
		{
			set->updateImage(binding, imageView, sampler);
		}
		else
		{
			printLog("❌ ERROR: Invalid set or imageView handle in updateDescriptorSet (Image)");
		}
	}

	void VulkanRHI::destroyShader(RHIShaderHandle shader) { shaderPool.remove(shader); }
	void VulkanRHI::destroyPipeline(RHIPipelineHandle pipeline) { pipelinePool.remove(pipeline); }
	void VulkanRHI::destroyPipelineLayout(RHIPipelineLayoutHandle layout) { pipelineLayoutPool.remove(layout); }

	void VulkanRHI::destroyDescriptorSetLayout(RHIDescriptorSetLayoutHandle layoutHandle)
	{
		RHIDescriptorSetLayout* layout = descriptorSetLayoutPool.get(layoutHandle);
		if (layout)
		{
			auto* vulkanLayout = static_cast<VulkanDescriptorSetLayout*>(layout);
			vulkanLayout->destroy();
			delete vulkanLayout;
			descriptorSetLayoutPool.remove(layoutHandle);
		}
	}

	void VulkanRHI::destroyDescriptorPool(RHIDescriptorPoolHandle poolHandle)
	{
		RHIDescriptorPool* pool = descriptorPoolPool.get(poolHandle);
		if (pool)
		{
			auto* vulkanPool = static_cast<VulkanDescriptorPool*>(pool);
			vulkanPool->destroy();
			delete vulkanPool;
			descriptorPoolPool.remove(poolHandle);
		}
	}

} // namespace BinRenderer::Vulkan
