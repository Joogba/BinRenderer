#include "VulkanShaderReflection.h"
#include "Vulkan/Logger.h"
#include <algorithm>

namespace BinRenderer::Vulkan
{
	ShaderReflectionData VulkanShaderReflection::reflect(
		const uint32_t* spirvCode,
		size_t codeSize,
		VkShaderStageFlagBits shaderStage)
	{
		ShaderReflectionData result;

		// SPIRV-Reflect 모듈 생성
		SpvReflectShaderModule module;
		SpvReflectResult spvResult = spvReflectCreateShaderModule(codeSize, spirvCode, &module);
		
		if (spvResult != SPV_REFLECT_RESULT_SUCCESS)
		{
			printLog("ERROR: Failed to create SPIRV-Reflect module");
			return result;
		}

		// 1. Descriptor bindings 추출
		uint32_t bindingCount = 0;
		spvResult = spvReflectEnumerateDescriptorBindings(&module, &bindingCount, nullptr);
		
		if (spvResult == SPV_REFLECT_RESULT_SUCCESS && bindingCount > 0)
		{
			std::vector<SpvReflectDescriptorBinding*> reflectBindings(bindingCount);
			spvResult = spvReflectEnumerateDescriptorBindings(&module, &bindingCount, reflectBindings.data());

			for (uint32_t i = 0; i < bindingCount; i++)
			{
				const SpvReflectDescriptorBinding* binding = reflectBindings[i];
				ShaderBindingInfo info;
				
				extractBindings(binding, shaderStage, info);
				
				// Set별로 분류
				result.bindings[binding->set].push_back(info);
			}
		}

		// 2. Push constants 추출
		uint32_t pushConstantCount = 0;
		spvResult = spvReflectEnumeratePushConstantBlocks(&module, &pushConstantCount, nullptr);
		
		if (spvResult == SPV_REFLECT_RESULT_SUCCESS && pushConstantCount > 0)
		{
			std::vector<SpvReflectBlockVariable*> pushConstants(pushConstantCount);
			spvResult = spvReflectEnumeratePushConstantBlocks(&module, &pushConstantCount, pushConstants.data());

			for (uint32_t i = 0; i < pushConstantCount; i++)
			{
				ShaderPushConstantInfo info;
				info.name = pushConstants[i]->name ? pushConstants[i]->name : "";
				info.offset = pushConstants[i]->offset;
				info.size = pushConstants[i]->size;
				info.stageFlags = static_cast<VkShaderStageFlags>(shaderStage);
				
				result.pushConstants.push_back(info);
			}
		}

		// 3. Vertex inputs 추출 (Vertex Shader만)
		if (shaderStage == VK_SHADER_STAGE_VERTEX_BIT)
		{
			uint32_t inputVarCount = 0;
			spvResult = spvReflectEnumerateInputVariables(&module, &inputVarCount, nullptr);
			
			if (spvResult == SPV_REFLECT_RESULT_SUCCESS && inputVarCount > 0)
			{
				std::vector<SpvReflectInterfaceVariable*> inputVars(inputVarCount);
				spvResult = spvReflectEnumerateInputVariables(&module, &inputVarCount, inputVars.data());

				for (uint32_t i = 0; i < inputVarCount; i++)
				{
					const SpvReflectInterfaceVariable* var = inputVars[i];
					
					// Built-in 변수 제외
					if (var->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN)
					{
						continue;
					}

					ShaderVertexInputInfo info;
					info.location = var->location;
					info.format = static_cast<VkFormat>(var->format);
					info.name = var->name ? var->name : "";
					
					result.vertexInputs.push_back(info);
				}

				// Location으로 정렬
				std::sort(result.vertexInputs.begin(), result.vertexInputs.end(),
					[](const ShaderVertexInputInfo& a, const ShaderVertexInputInfo& b) {
						return a.location < b.location;
					});
			}
		}

		// 4. Compute workgroup size 추출 (Compute Shader만)
		if (shaderStage == VK_SHADER_STAGE_COMPUTE_BIT)
		{
			result.workgroupSizeX = module.entry_points[0].local_size.x;
			result.workgroupSizeY = module.entry_points[0].local_size.y;
			result.workgroupSizeZ = module.entry_points[0].local_size.z;
		}

		spvReflectDestroyShaderModule(&module);
		return result;
	}

	std::map<uint32_t, std::vector<ShaderBindingInfo>> VulkanShaderReflection::mergeBindings(
		const std::vector<ShaderReflectionData>& reflections)
	{
		std::map<uint32_t, std::vector<ShaderBindingInfo>> merged;

		for (const auto& reflection : reflections)
		{
			for (const auto& [setIndex, bindings] : reflection.bindings)
			{
				for (const auto& binding : bindings)
				{
					// 이미 존재하는 바인딩인지 확인
					auto& setBindings = merged[setIndex];
					auto it = std::find_if(setBindings.begin(), setBindings.end(),
						[&](const ShaderBindingInfo& b) {
							return b.binding == binding.binding;
						});

					if (it != setBindings.end())
					{
						// Stage flags 병합
						it->stageFlags |= binding.stageFlags;
					}
					else
					{
						// 새로운 바인딩 추가
						setBindings.push_back(binding);
					}
				}
			}
		}

		// Binding index로 정렬
		for (auto& [setIndex, bindings] : merged)
		{
			std::sort(bindings.begin(), bindings.end(),
				[](const ShaderBindingInfo& a, const ShaderBindingInfo& b) {
					return a.binding < b.binding;
				});
		}

		return merged;
	}

	std::vector<VkDescriptorSetLayoutBinding> VulkanShaderReflection::createLayoutBindings(
		const std::map<uint32_t, std::vector<ShaderBindingInfo>>& bindings,
		uint32_t setIndex)
	{
		std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

		auto it = bindings.find(setIndex);
		if (it == bindings.end())
		{
			return layoutBindings;
		}

		for (const auto& binding : it->second)
		{
			VkDescriptorSetLayoutBinding layoutBinding{};
			layoutBinding.binding = binding.binding;
			layoutBinding.descriptorType = binding.descriptorType;
			layoutBinding.descriptorCount = binding.descriptorCount;
			layoutBinding.stageFlags = binding.stageFlags;
			layoutBinding.pImmutableSamplers = nullptr;

			layoutBindings.push_back(layoutBinding);
		}

		return layoutBindings;
	}

	std::vector<VkPushConstantRange> VulkanShaderReflection::createPushConstantRanges(
		const std::vector<ShaderReflectionData>& reflections)
	{
		std::vector<VkPushConstantRange> ranges;

		for (const auto& reflection : reflections)
		{
			for (const auto& pushConstant : reflection.pushConstants)
			{
				// 이미 존재하는 범위인지 확인
				auto it = std::find_if(ranges.begin(), ranges.end(),
					[&](const VkPushConstantRange& r) {
						return r.offset == pushConstant.offset && r.size == pushConstant.size;
					});

				if (it != ranges.end())
				{
					// Stage flags 병합
					it->stageFlags |= pushConstant.stageFlags;
				}
				else
				{
					// 새로운 범위 추가
					VkPushConstantRange range{};
					range.stageFlags = pushConstant.stageFlags;
					range.offset = pushConstant.offset;
					range.size = pushConstant.size;
					ranges.push_back(range);
				}
			}
		}

		return ranges;
	}

	std::vector<VkVertexInputAttributeDescription> VulkanShaderReflection::createVertexInputAttributes(
		const ShaderReflectionData& reflection)
	{
		std::vector<VkVertexInputAttributeDescription> attributes;

		uint32_t currentOffset = 0;
		for (const auto& input : reflection.vertexInputs)
		{
			VkVertexInputAttributeDescription attr{};
			attr.location = input.location;
			attr.binding = 0;  // Default binding
			attr.format = input.format;
			attr.offset = currentOffset;

			attributes.push_back(attr);

			// 다음 offset 계산 (format 크기 기반)
			switch (input.format)
			{
			case VK_FORMAT_R32_SFLOAT:
			case VK_FORMAT_R32_SINT:
			case VK_FORMAT_R32_UINT:
				currentOffset += 4;
				break;
			case VK_FORMAT_R32G32_SFLOAT:
			case VK_FORMAT_R32G32_SINT:
			case VK_FORMAT_R32G32_UINT:
				currentOffset += 8;
				break;
			case VK_FORMAT_R32G32B32_SFLOAT:
			case VK_FORMAT_R32G32B32_SINT:
			case VK_FORMAT_R32G32B32_UINT:
				currentOffset += 12;
				break;
			case VK_FORMAT_R32G32B32A32_SFLOAT:
			case VK_FORMAT_R32G32B32A32_SINT:
			case VK_FORMAT_R32G32B32A32_UINT:
				currentOffset += 16;
				break;
			default:
				currentOffset += 16;  // Default
				break;
			}
		}

		return attributes;
	}

	// ========================================
	// Private 헬퍼 메서드
	// ========================================

	void VulkanShaderReflection::extractBindings(
		const SpvReflectDescriptorBinding* binding,
		VkShaderStageFlagBits shaderStage,
		ShaderBindingInfo& outInfo)
	{
		outInfo.name = binding->name ? binding->name : "";
		outInfo.set = binding->set;
		outInfo.binding = binding->binding;
		outInfo.descriptorType = static_cast<VkDescriptorType>(binding->descriptor_type);
		outInfo.descriptorCount = binding->count;
		outInfo.stageFlags = static_cast<VkShaderStageFlags>(shaderStage);

		// Image 관련 정보
		if (binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE ||
			binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
			binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		{
			determineImageAccess(binding, shaderStage, outInfo);
		}
	}

	void VulkanShaderReflection::determineImageAccess(
		const SpvReflectDescriptorBinding* binding,
		VkShaderStageFlagBits shaderStage,
		ShaderBindingInfo& outInfo)
	{
		VkDescriptorType descriptorType = static_cast<VkDescriptorType>(binding->descriptor_type);

		// Layout 결정
		if (descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
		{
			outInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		}
		else if (descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
			descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		{
			outInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		// Access flags 결정
		bool hasNonWritable = (binding->decoration_flags & SPV_REFLECT_DECORATION_NON_WRITABLE) != 0;
		bool hasNonReadable = (binding->decoration_flags & SPV_REFLECT_DECORATION_NON_READABLE) != 0;

		if (descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
		{
			if (hasNonReadable && !hasNonWritable)
			{
				outInfo.accessFlags = VK_ACCESS_2_SHADER_WRITE_BIT;
				outInfo.writeOnly = true;
			}
			else
			{
				outInfo.accessFlags = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
				outInfo.writeOnly = false;
			}
		}
		else
		{
			outInfo.accessFlags = VK_ACCESS_2_SHADER_READ_BIT;
			outInfo.writeOnly = false;
		}

		// Pipeline stage 결정
		outInfo.stageFlags2 = getStageFlagsFromShaderStage(shaderStage);
	}

	VkPipelineStageFlags2 VulkanShaderReflection::getStageFlagsFromShaderStage(VkShaderStageFlagBits stage)
	{
		switch (stage)
		{
		case VK_SHADER_STAGE_VERTEX_BIT:
			return VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
		case VK_SHADER_STAGE_FRAGMENT_BIT:
			return VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		case VK_SHADER_STAGE_COMPUTE_BIT:
			return VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
		case VK_SHADER_STAGE_GEOMETRY_BIT:
			return VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT;
		case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
			return VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT;
		case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
			return VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT;
		default:
			return VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
		}
	}

} // namespace BinRenderer::Vulkan
