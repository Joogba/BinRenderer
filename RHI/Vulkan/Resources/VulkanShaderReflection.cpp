#include "VulkanShaderReflection.h"
#include "Core/Logger.h"
#include <algorithm>

namespace BinRenderer::Vulkan
{
	// ========================================
	// Constructor / Destructor
	// ========================================

	VulkanShaderReflection::VulkanShaderReflection(const void* spirvCode, size_t codeSize)
		: spirvCode_(spirvCode)
		, codeSize_(codeSize)
	{
	}

	VulkanShaderReflection::VulkanShaderReflection(const std::vector<uint32_t>& spirvCode)
		: spirvCode_(spirvCode.data())
		, codeSize_(spirvCode.size() * sizeof(uint32_t))
	{
	}

	VulkanShaderReflection::~VulkanShaderReflection()
	{
		if (moduleCreated_)
		{
			spvReflectDestroyShaderModule(&module_);
		}
	}

	// ========================================
	// RHIShaderReflection Interface Implementation
	// ========================================

	bool VulkanShaderReflection::reflect()
	{
		// SPIRV-Reflect 모듈 생성
		SpvReflectResult result = spvReflectCreateShaderModule(codeSize_, spirvCode_, &module_);
		
		if (result != SPV_REFLECT_RESULT_SUCCESS)
		{
			printLog("[VulkanShaderReflection] ❌ Failed to create SPIRV-Reflect module");
			return false;
		}
		
		moduleCreated_ = true;

		// 셰이더 스테이지 설정
		reflectionData_.stage = convertShaderStage(static_cast<VkShaderStageFlagBits>(module_.shader_stage));
		reflectionData_.entryPoint = module_.entry_point_name ? module_.entry_point_name : "main";

		// 리플렉션 데이터 추출
		reflectDescriptorBindings();
		reflectPushConstants();
		
		if (reflectionData_.stage == RHIShaderStage::Vertex)
		{
			reflectVertexInputs();
		}
		
		if (reflectionData_.stage == RHIShaderStage::Compute)
		{
			reflectComputeWorkgroupSize();
		}

		// 리소스 사용량 계산
		reflectionData_.calculateResourceUsage();

		printLog("[VulkanShaderReflection]  Reflection complete - {} descriptor bindings, {} push constants",
			reflectionData_.resourceUsage.totalDescriptors,
			reflectionData_.pushConstants.size());

		return true;
	}

	const ShaderReflectionData& VulkanShaderReflection::getReflectionData() const
	{
		return reflectionData_;
	}

	RHIShaderStage VulkanShaderReflection::getShaderStage() const
	{
		return reflectionData_.stage;
	}

	const std::string& VulkanShaderReflection::getEntryPoint() const
	{
		return reflectionData_.entryPoint;
	}

	bool VulkanShaderReflection::validate() const
	{
		return moduleCreated_;
	}

	const std::vector<ShaderBindingInfo>* VulkanShaderReflection::getDescriptorSetBindings(uint32_t setIndex) const
	{
		return reflectionData_.getBindings(setIndex);
	}

	const std::vector<ShaderPushConstantInfo>& VulkanShaderReflection::getPushConstants() const
	{
		return reflectionData_.pushConstants;
	}

	const std::vector<ShaderVertexInputInfo>& VulkanShaderReflection::getVertexInputs() const
	{
		return reflectionData_.vertexInputs;
	}

	void VulkanShaderReflection::getComputeWorkgroupSize(uint32_t& x, uint32_t& y, uint32_t& z) const
	{
		x = reflectionData_.workgroupSizeX;
		y = reflectionData_.workgroupSizeY;
		z = reflectionData_.workgroupSizeZ;
	}

	// ========================================
	// Private Helper Functions
	// ========================================

	void VulkanShaderReflection::reflectDescriptorBindings()
	{
		uint32_t bindingCount = 0;
		SpvReflectResult result = spvReflectEnumerateDescriptorBindings(&module_, &bindingCount, nullptr);
		
		if (result != SPV_REFLECT_RESULT_SUCCESS || bindingCount == 0)
			return;

		std::vector<SpvReflectDescriptorBinding*> spvBindings(bindingCount);
		result = spvReflectEnumerateDescriptorBindings(&module_, &bindingCount, spvBindings.data());

		for (uint32_t i = 0; i < bindingCount; i++)
		{
			const SpvReflectDescriptorBinding* spvBinding = spvBindings[i];
			ShaderBindingInfo info;
			
			info.name = spvBinding->name ? spvBinding->name : "";
			info.set = spvBinding->set;
			info.binding = spvBinding->binding;
			info.descriptorType = convertDescriptorType(static_cast<VkDescriptorType>(spvBinding->descriptor_type));
			info.descriptorCount = spvBinding->count;
			info.stageFlags = reflectionData_.stage;

			// 버퍼 크기 (UBO/SSBO)
			if (spvBinding->block.size > 0)
			{
				info.bufferSize = spvBinding->block.size;
			}

			// Set별로 분류
			reflectionData_.bindings[info.set].push_back(info);
		}
	}

	void VulkanShaderReflection::reflectPushConstants()
	{
		uint32_t pushConstantCount = 0;
		SpvReflectResult result = spvReflectEnumeratePushConstantBlocks(&module_, &pushConstantCount, nullptr);
		
		if (result != SPV_REFLECT_RESULT_SUCCESS || pushConstantCount == 0)
			return;

		std::vector<SpvReflectBlockVariable*> pushConstants(pushConstantCount);
		result = spvReflectEnumeratePushConstantBlocks(&module_, &pushConstantCount, pushConstants.data());

		for (uint32_t i = 0; i < pushConstantCount; i++)
		{
			ShaderPushConstantInfo info;
			info.name = pushConstants[i]->name ? pushConstants[i]->name : "";
			info.offset = pushConstants[i]->offset;
			info.size = pushConstants[i]->size;
			info.stageFlags = reflectionData_.stage;
			
			reflectionData_.pushConstants.push_back(info);
		}
	}

	void VulkanShaderReflection::reflectVertexInputs()
	{
		uint32_t inputVarCount = 0;
		SpvReflectResult result = spvReflectEnumerateInputVariables(&module_, &inputVarCount, nullptr);
		
		if (result != SPV_REFLECT_RESULT_SUCCESS || inputVarCount == 0)
			return;

		std::vector<SpvReflectInterfaceVariable*> inputVars(inputVarCount);
		result = spvReflectEnumerateInputVariables(&module_, &inputVarCount, inputVars.data());

		for (uint32_t i = 0; i < inputVarCount; i++)
		{
			const SpvReflectInterfaceVariable* var = inputVars[i];
			
			// Built-in 변수 제외 (gl_VertexIndex 등)
			if (var->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN)
				continue;

			ShaderVertexInputInfo info;
			info.location = var->location;
			info.format = convertVertexFormat(static_cast<VkFormat>(var->format));
			info.name = var->name ? var->name : "";
			info.offset = 0; // SPIRV-Reflect는 offset 제공 안 함 (나중에 계산)
			
			// Semantic name 추출 (있으면)
			if (var->semantic)
			{
				info.semanticName = var->semantic;
			}
			
			reflectionData_.vertexInputs.push_back(info);
		}

		// Location으로 정렬
		std::sort(reflectionData_.vertexInputs.begin(), reflectionData_.vertexInputs.end(),
			[](const ShaderVertexInputInfo& a, const ShaderVertexInputInfo& b) {
				return a.location < b.location;
			});
	}

	void VulkanShaderReflection::reflectComputeWorkgroupSize()
	{
		reflectionData_.workgroupSizeX = module_.entry_points[0].local_size.x;
		reflectionData_.workgroupSizeY = module_.entry_points[0].local_size.y;
		reflectionData_.workgroupSizeZ = module_.entry_points[0].local_size.z;
	}

	// ========================================
	// Type Conversion Helpers
	// ========================================

	RHIDescriptorType VulkanShaderReflection::convertDescriptorType(VkDescriptorType vkType)
	{
		switch (vkType)
		{
		case VK_DESCRIPTOR_TYPE_SAMPLER: return RHIDescriptorType::Sampler;
		case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return RHIDescriptorType::CombinedImageSampler;
		case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE: return RHIDescriptorType::SampledImage;
		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: return RHIDescriptorType::StorageImage;
		case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: return RHIDescriptorType::UniformTexelBuffer;
		case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: return RHIDescriptorType::StorageTexelBuffer;
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: return RHIDescriptorType::UniformBuffer;
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: return RHIDescriptorType::StorageBuffer;
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: return RHIDescriptorType::UniformBufferDynamic;
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: return RHIDescriptorType::StorageBufferDynamic;
		case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: return RHIDescriptorType::InputAttachment;
		case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: return RHIDescriptorType::AccelerationStructure;
		default: return RHIDescriptorType::UniformBuffer;
		}
	}

	RHIShaderStage VulkanShaderReflection::convertShaderStage(VkShaderStageFlagBits vkStage)
	{
		switch (vkStage)
		{
		case VK_SHADER_STAGE_VERTEX_BIT: return RHIShaderStage::Vertex;
		case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: return RHIShaderStage::TessellationControl;
		case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return RHIShaderStage::TessellationEvaluation;
		case VK_SHADER_STAGE_GEOMETRY_BIT: return RHIShaderStage::Geometry;
		case VK_SHADER_STAGE_FRAGMENT_BIT: return RHIShaderStage::Fragment;
		case VK_SHADER_STAGE_COMPUTE_BIT: return RHIShaderStage::Compute;
		default: return RHIShaderStage::Fragment;
		}
	}

	RHIVertexFormat VulkanShaderReflection::convertVertexFormat(VkFormat vkFormat)
	{
		switch (vkFormat)
		{
		case VK_FORMAT_R32_SFLOAT: return RHIVertexFormat::R32_Float;
		case VK_FORMAT_R32G32_SFLOAT: return RHIVertexFormat::R32G32_Float;
		case VK_FORMAT_R32G32B32_SFLOAT: return RHIVertexFormat::R32G32B32_Float;
		case VK_FORMAT_R32G32B32A32_SFLOAT: return RHIVertexFormat::R32G32B32A32_Float;
		case VK_FORMAT_R32_SINT: return RHIVertexFormat::R32_Sint;
		case VK_FORMAT_R32G32_SINT: return RHIVertexFormat::R32G32_Sint;
		case VK_FORMAT_R32G32B32_SINT: return RHIVertexFormat::R32G32B32_Sint;
		case VK_FORMAT_R32G32B32A32_SINT: return RHIVertexFormat::R32G32B32A32_Sint;
		case VK_FORMAT_R32_UINT: return RHIVertexFormat::R32_Uint;
		case VK_FORMAT_R32G32_UINT: return RHIVertexFormat::R32G32_Uint;
		case VK_FORMAT_R32G32B32_UINT: return RHIVertexFormat::R32G32B32_Uint;
		case VK_FORMAT_R32G32B32A32_UINT: return RHIVertexFormat::R32G32B32A32_Uint;
		case VK_FORMAT_R8G8B8A8_UNORM: return RHIVertexFormat::R8G8B8A8_Unorm;
		case VK_FORMAT_R8G8B8A8_SNORM: return RHIVertexFormat::R8G8B8A8_Snorm;
		default: return RHIVertexFormat::Undefined;
		}
	}

	RHIImageLayout VulkanShaderReflection::convertImageLayout(VkImageLayout vkLayout)
	{
		switch (vkLayout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED: return RHIImageLayout::Undefined;
		case VK_IMAGE_LAYOUT_GENERAL: return RHIImageLayout::General;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return RHIImageLayout::ColorAttachment;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return RHIImageLayout::DepthStencilAttachment;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL: return RHIImageLayout::DepthStencilReadOnly;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return RHIImageLayout::ShaderReadOnly;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: return RHIImageLayout::TransferSrc;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: return RHIImageLayout::TransferDst;
		case VK_IMAGE_LAYOUT_PREINITIALIZED: return RHIImageLayout::Preinitialized;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: return RHIImageLayout::PresentSrc;
		default: return RHIImageLayout::Undefined;
		}
	}

	RHIAccessFlags VulkanShaderReflection::convertAccessFlags(VkAccessFlags2 vkAccess)
	{
		RHIAccessFlags result = RHIAccessFlags::None;

		if (vkAccess & VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT)
			result = result | RHIAccessFlags::IndirectCommandRead;
		if (vkAccess & VK_ACCESS_2_INDEX_READ_BIT)
			result = result | RHIAccessFlags::IndexRead;
		if (vkAccess & VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT)
			result = result | RHIAccessFlags::VertexAttributeRead;
		if (vkAccess & VK_ACCESS_2_UNIFORM_READ_BIT)
			result = result | RHIAccessFlags::UniformRead;
		if (vkAccess & VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT)
			result = result | RHIAccessFlags::InputAttachmentRead;
		if (vkAccess & VK_ACCESS_2_SHADER_READ_BIT)
			result = result | RHIAccessFlags::ShaderRead;
		if (vkAccess & VK_ACCESS_2_SHADER_WRITE_BIT)
			result = result | RHIAccessFlags::ShaderWrite;
		if (vkAccess & VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT)
			result = result | RHIAccessFlags::ColorAttachmentRead;
		if (vkAccess & VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT)
			result = result | RHIAccessFlags::ColorAttachmentWrite;
		if (vkAccess & VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT)
			result = result | RHIAccessFlags::DepthStencilAttachmentRead;
		if (vkAccess & VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
			result = result | RHIAccessFlags::DepthStencilAttachmentWrite;
		if (vkAccess & VK_ACCESS_2_TRANSFER_READ_BIT)
			result = result | RHIAccessFlags::TransferRead;
		if (vkAccess & VK_ACCESS_2_TRANSFER_WRITE_BIT)
			result = result | RHIAccessFlags::TransferWrite;
		if (vkAccess & VK_ACCESS_2_HOST_READ_BIT)
			result = result | RHIAccessFlags::HostRead;
		if (vkAccess & VK_ACCESS_2_HOST_WRITE_BIT)
			result = result | RHIAccessFlags::HostWrite;
		if (vkAccess & VK_ACCESS_2_MEMORY_READ_BIT)
			result = result | RHIAccessFlags::MemoryRead;
		if (vkAccess & VK_ACCESS_2_MEMORY_WRITE_BIT)
			result = result | RHIAccessFlags::MemoryWrite;

		return result;
	}

} // namespace BinRenderer::Vulkan
