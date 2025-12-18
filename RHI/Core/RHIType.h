#pragma once

#include <cstdint>
#include <memory>
#include <string>


namespace BinRenderer
{
	enum class RHIStructType : int
	{
		// Buffer
		RHIBufferCreateInfo,
		RHIBufferViewCreateInfo,
		RHIBufferCopy,
		RHIBufferImageCopy,
		RHIBufferMemoryBarrier,
		// Command
		RHICommandBufferAllocateInfo,
		RHICommandBufferBeginInfo,
		RHICommandBufferInheritanceInfo,
		RHICommandPoolCreateInfo,
		// Descriptor
		RHIDescriptorPoolSize,
		RHIDescriptorPoolCreateInfo,
		RHIDescriptorSetAllocateInfo,
		RHIDescriptorSetLayoutBinding,
		RHIDescriptorSetLayoutCreateInfo,
		RHICopyDescriptorSet,
		RHIDescriptorImageInfo,
		RHIDescriptorBufferInfo,
		// Device
		RHIDeviceCreateInfo,
		RHIDeviceQueueCreateInfo,
		// Fence
		RHIFenceCreateInfo,
		// Image
		RHIImageCreateInfo,
		RHIImageViewCreateInfo,
		RHIImageSubresourceRange,
		RHIImageMemoryBarrier,
		// Instance
		RHIApplicationInfo,
		// Memory Barrier
		RHIMemoryBarrier,
		// Pipeline
		RHIPipelineShaderStageCreateInfo,
		RHIPipelineVertexInputStateCreateInfo,
		RHIPipelineInputAssemblyStateCreateInfo,
		RHIPipelineTessellationStateCreateInfo,
		RHIPipelineViewportStateCreateInfo,
		RHIPipelineRasterizationStateCreateInfo,
		RHIPipelineMultisampleStateCreateInfo,
		RHIPipelineDepthStencilStateCreateInfo,
		RHIPipelineColorBlendStateCreateInfo,
		RHIPipelineDynamicStateCreateInfo,
		RHISpecializationMapEntry,
		// Render Pass
		RHIAttachmentDescription,
	};

	enum RHICommandBufferLevel : uint32_t
	{
		RHI_COMMAND_BUFFER_LEVEL_PRIMARY = 0,
		RHI_COMMAND_BUFFER_LEVEL_SECONDARY = 1,
	};

	enum RHIImageType : int
	{
		RHI_IMAGE_TYPE_1D = 0,
		RHI_IMAGE_TYPE_2D = 1,
		RHI_IMAGE_TYPE_3D = 2,
	};

	enum RHISharingMode : int
	{
		RHI_SHARING_MODE_EXCLUSIVE = 0,
		RHI_SHARING_MODE_CONCURRENT = 1,
	};

	enum RHIFormat : int
	{
		RHI_FORMAT_UNDEFINED = 0,
		RHI_FORMAT_R8G8_UNORM = 16,        // ✅ 추가: Metallic-Roughness용
		RHI_FORMAT_R8G8B8A8_UNORM = 37,
		RHI_FORMAT_B8G8R8A8_UNORM = 44,
		RHI_FORMAT_R16G16B16A16_SFLOAT = 97,
		RHI_FORMAT_R32_UINT = 98,          // ✅ GPU Instancing: materialOffset용
		RHI_FORMAT_R32G32B32A32_SFLOAT = 109,
		RHI_FORMAT_D32_SFLOAT = 126,
	};

	enum RHISampleCountFlagBits : int
	{
		RHI_SAMPLE_COUNT_1_BIT = 0x00000001,
		RHI_SAMPLE_COUNT_2_BIT = 0x00000002,
		RHI_SAMPLE_COUNT_4_BIT = 0x00000004,
		RHI_SAMPLE_COUNT_8_BIT = 0x00000008,
		RHI_SAMPLE_COUNT_16_BIT = 0x00000010,
		RHI_SAMPLE_COUNT_32_BIT = 0x00000020,
		RHI_SAMPLE_COUNT_64_BIT = 0x00000040,
	};

	enum RHIImageLayout : int
	{
		RHI_IMAGE_LAYOUT_UNDEFINED = 0,
		RHI_IMAGE_LAYOUT_GENERAL = 1,
		RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL = 2,
		RHI_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL = 3,
		RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL = 5,
		RHI_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL = 6,
		RHI_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL = 7,
		RHI_IMAGE_LAYOUT_PRESENT_SRC_KHR = 1000001002,
	};

	enum RHIImageViewType : int
	{
		RHI_IMAGE_VIEW_TYPE_1D = 0,
		RHI_IMAGE_VIEW_TYPE_2D = 1,
		RHI_IMAGE_VIEW_TYPE_3D = 2,
		RHI_IMAGE_VIEW_TYPE_CUBE = 3,
	};

	enum RHIPipelineBindPoint : int
	{
		RHI_PIPELINE_BIND_POINT_GRAPHICS = 0,
		RHI_PIPELINE_BIND_POINT_COMPUTE = 1,
	};

	enum RHIDescriptorType : int
	{
		RHI_DESCRIPTOR_TYPE_SAMPLER = 0,
		RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER = 1,
		RHI_DESCRIPTOR_TYPE_SAMPLED_IMAGE = 2,
		RHI_DESCRIPTOR_TYPE_STORAGE_IMAGE = 3,
		RHI_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER = 4,
		RHI_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER = 5,
		RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER = 6,
		RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER = 7,
		RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC = 8,
		RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC = 9,
		RHI_DESCRIPTOR_TYPE_INPUT_ATTACHMENT = 10,
	};

	enum RHIShaderStageFlagBits : uint32_t
	{
		RHI_SHADER_STAGE_VERTEX_BIT = 0x00000001,
		RHI_SHADER_STAGE_TESSELLATION_CONTROL_BIT = 0x00000002,
		RHI_SHADER_STAGE_TESSELLATION_EVALUATION_BIT = 0x00000004,
		RHI_SHADER_STAGE_GEOMETRY_BIT = 0x00000008,
		RHI_SHADER_STAGE_FRAGMENT_BIT = 0x00000010,
		RHI_SHADER_STAGE_COMPUTE_BIT = 0x00000020,
	};

	enum RHIPhysicalDeviceType : int
	{
		RHI_PHYSICAL_DEVICE_TYPE_OTHER = 0,
		RHI_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU = 1,
		RHI_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU = 2,
		RHI_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU = 3,
		RHI_PHYSICAL_DEVICE_TYPE_CPU = 4,
	};

	enum RHIPrimitiveTopology : int
	{
		RHI_PRIMITIVE_TOPOLOGY_POINT_LIST = 0,
		RHI_PRIMITIVE_TOPOLOGY_LINE_LIST = 1,
		RHI_PRIMITIVE_TOPOLOGY_LINE_STRIP = 2,
		RHI_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST = 3,
		RHI_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP = 4,
	};

	enum RHISamplerMipmapMode : int
	{
		RHI_SAMPLER_MIPMAP_MODE_NEAREST = 0,
		RHI_SAMPLER_MIPMAP_MODE_LINEAR = 1,
	};

	enum RHIBlendOp : int
	{
		RHI_BLEND_OP_ADD = 0,
		RHI_BLEND_OP_SUBTRACT = 1,
		RHI_BLEND_OP_REVERSE_SUBTRACT = 2,
		RHI_BLEND_OP_MIN = 3,
		RHI_BLEND_OP_MAX = 4,
	};

	enum RHIStencilOp : int
	{
		RHI_STENCIL_OP_KEEP = 0,
		RHI_STENCIL_OP_ZERO = 1,
		RHI_STENCIL_OP_REPLACE = 2,
		RHI_STENCIL_OP_INCREMENT_AND_CLAMP = 3,
		RHI_STENCIL_OP_DECREMENT_AND_CLAMP = 4,
		RHI_STENCIL_OP_INVERT = 5,
		RHI_STENCIL_OP_INCREMENT_AND_WRAP = 6,
		RHI_STENCIL_OP_DECREMENT_AND_WRAP = 7,
	};

	enum RHICompareOp : int
	{
		RHI_COMPARE_OP_NEVER = 0,
		RHI_COMPARE_OP_LESS = 1,
		RHI_COMPARE_OP_EQUAL = 2,
		RHI_COMPARE_OP_LESS_OR_EQUAL = 3,
		RHI_COMPARE_OP_GREATER = 4,
		RHI_COMPARE_OP_NOT_EQUAL = 5,
		RHI_COMPARE_OP_GREATER_OR_EQUAL = 6,
		RHI_COMPARE_OP_ALWAYS = 7,
	};

	enum RHIBlendFactor : int
	{
		RHI_BLEND_FACTOR_ZERO = 0,
		RHI_BLEND_FACTOR_ONE = 1,
		RHI_BLEND_FACTOR_SRC_COLOR = 2,
		RHI_BLEND_FACTOR_ONE_MINUS_SRC_COLOR = 3,
		RHI_BLEND_FACTOR_DST_COLOR = 4,
		RHI_BLEND_FACTOR_ONE_MINUS_DST_COLOR = 5,
		RHI_BLEND_FACTOR_SRC_ALPHA = 6,
		RHI_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA = 7,
		RHI_BLEND_FACTOR_DST_ALPHA = 8,
		RHI_BLEND_FACTOR_ONE_MINUS_DST_ALPHA = 9,
		RHI_BLEND_FACTOR_CONSTANT_COLOR = 10,
		RHI_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR = 11,
		RHI_BLEND_FACTOR_CONSTANT_ALPHA = 12,
		RHI_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA = 13,
		RHI_BLEND_FACTOR_SRC_ALPHA_SATURATE = 14,
	};

	enum RHIBufferUsageFlagBits : uint32_t
	{
		RHI_BUFFER_USAGE_VERTEX_BUFFER_BIT = 0x00000001,
		RHI_BUFFER_USAGE_INDEX_BUFFER_BIT = 0x00000002,
		RHI_BUFFER_USAGE_UNIFORM_BUFFER_BIT = 0x00000010,
		RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT = 0x00000020,
		RHI_BUFFER_USAGE_TRANSFER_SRC_BIT = 0x00000040,
		RHI_BUFFER_USAGE_TRANSFER_DST_BIT = 0x00000080,
	};

	enum RHIMemoryPropertyFlagBits : uint32_t
	{
		RHI_MEMORY_PROPERTY_DEVICE_LOCAL_BIT = 0x00000001,
		RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT = 0x00000002,
		RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT = 0x00000004,
		RHI_MEMORY_PROPERTY_HOST_CACHED_BIT = 0x00000008,
	};

	enum RHIPipelineStageFlagBits : uint32_t
	{
		RHI_PIPELINE_STAGE_TOP_OF_PIPE_BIT = 0x00000001,
		RHI_PIPELINE_STAGE_DRAW_INDIRECT_BIT = 0x00000002,
		RHI_PIPELINE_STAGE_VERTEX_INPUT_BIT = 0x00000004,
		RHI_PIPELINE_STAGE_VERTEX_SHADER_BIT = 0x00000008,
		RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT = 0x00000080,
		RHI_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT = 0x00000100,
		RHI_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT = 0x00000200,
		RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT = 0x00000400,
		RHI_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT = 0x00000800,
		RHI_PIPELINE_STAGE_TRANSFER_BIT = 0x00001000,
		RHI_PIPELINE_STAGE_ALL_GRAPHICS_BIT = 0x00004000,
		RHI_PIPELINE_STAGE_ALL_COMMANDS_BIT = 0x00008000,
	};

	enum RHIAccessFlagBits : uint32_t
	{
		RHI_ACCESS_INDIRECT_COMMAND_READ_BIT = 0x00000001,
		RHI_ACCESS_INDEX_READ_BIT = 0x00000002,
		RHI_ACCESS_VERTEX_ATTRIBUTE_READ_BIT = 0x00000004,
		RHI_ACCESS_UNIFORM_READ_BIT = 0x00000008,
		RHI_ACCESS_INPUT_ATTACHMENT_READ_BIT = 0x00000010,
		RHI_ACCESS_COLOR_ATTACHMENT_READ_BIT = 0x00000020,
		RHI_ACCESS_COLOR_ATTACHMENT_WRITE_BIT = 0x00000040,
		RHI_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT = 0x00000080,
		RHI_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT = 0x00000100,
		RHI_ACCESS_TRANSFER_READ_BIT = 0x00000200,
		RHI_ACCESS_TRANSFER_WRITE_BIT = 0x00000400,
		RHI_ACCESS_SHADER_READ_BIT = 0x00000800,
		RHI_ACCESS_SHADER_WRITE_BIT = 0x00001000,
	};

	enum RHICullModeFlagBits : uint32_t
	{
		RHI_CULL_MODE_NONE = 0x00000000,
		RHI_CULL_MODE_FRONT_BIT = 0x00000001,
		RHI_CULL_MODE_BACK_BIT = 0x00000002,
		RHI_CULL_MODE_FRONT_AND_BACK = 0x00000003,
	};

	enum RHIImageUsageFlagBits : uint32_t
	{
		RHI_IMAGE_USAGE_TRANSFER_SRC_BIT = 0x00000001,
		RHI_IMAGE_USAGE_TRANSFER_DST_BIT = 0x00000002,
		RHI_IMAGE_USAGE_SAMPLED_BIT = 0x00000004,
		RHI_IMAGE_USAGE_STORAGE_BIT = 0x00000008,
		RHI_IMAGE_USAGE_COLOR_ATTACHMENT_BIT = 0x00000010,
		RHI_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT = 0x00000020,
	};

	enum RHIFenceCreateFlagBits : uint32_t
	{
		RHI_FENCE_CREATE_SIGNALED_BIT = 0x00000001,
	};

	enum RHIComandBufferUsageFlagBits : uint32_t
	{
		RHI_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT = 0x00000001,
		RHI_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT = 0x00000002,
		RHI_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT = 0x00000004,
		RHI_COmmand_buffer_usage_Flags_MAX_ENUM = 0x7FFFFFFF,
	};

	enum RHIImageAspectFlagBits : uint32_t
	{
		RHI_IMAGE_ASPECT_COLOR_BIT = 0x00000001,
		RHI_IMAGE_ASPECT_DEPTH_BIT = 0x00000002,
		RHI_IMAGE_ASPECT_STENCIL_BIT = 0x00000004,
	};

	enum RHIAttachmentLoadOp : int
	{
		RHI_ATTACHMENT_LOAD_OP_LOAD = 0,
		RHI_ATTACHMENT_LOAD_OP_CLEAR = 1,
		RHI_ATTACHMENT_LOAD_OP_DONT_CARE = 2,
		RHI_ATTACHMENT_LOAD_OP_NONE_EXT = 1000400000,
		RHI_ATTACHMENT_LOAD_OP_MAX_ENUM = 0x7FFFFFFF
	};

	enum RHIAttachmentStoreOp : int
	{
		RHI_ATTACHMENT_STORE_OP_STORE = 0,
		RHI_ATTACHMENT_STORE_OP_DONT_CARE = 1,
		RHI_ATTACHMENT_STORE_OP_NONE_KHR = 1000301000,
		RHI_ATTACHMENT_STORE_OP_NONE_QCOM = RHI_ATTACHMENT_STORE_OP_NONE_KHR,
		RHI_ATTACHMENT_STORE_OP_NONE_EXT = RHI_ATTACHMENT_STORE_OP_NONE_KHR,
		RHI_ATTACHMENT_STORE_OP_MAX_ENUM = 0x7FFFFFFF
	};

	enum RHICommandPoolCreateFlagBits {
		RHI_COMMAND_POOL_CREATE_TRANSIENT_BIT = 0x00000001,
		RHI_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT = 0x00000002,
		RHI_COMMAND_POOL_CREATE_PROTECTED_BIT = 0x00000004,
		RHI_COMMAND_POOL_CREATE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
	};

	enum RHIImageTiling : int
	{
		RHI_IMAGE_TILING_OPTIMAL = 0,
		RHI_IMAGE_TILING_LINEAR = 1,
	};

	enum RHIComponentSwizzle : int
	{
		RHI_COMPONENT_SWIZZLE_IDENTITY = 0,
		RHI_COMPONENT_SWIZZLE_ZERO = 1,
		RHI_COMPONENT_SWIZZLE_ONE = 2,
		RHI_COMPONENT_SWIZZLE_R = 3,
		RHI_COMPONENT_SWIZZLE_G = 4,
		RHI_COMPONENT_SWIZZLE_B = 5,
		RHI_cOMPONENT_SWIZZLE_A = 6,
	};

	enum RHILogicOp : int
	{
		RHI_LOGIC_OP_CLEAR = 0,
		RHI_LOGIC_OP_AND = 1,
		RHI_LOGIC_OP_AND_REVERSE = 2,
		RHI_LOGIC_OP_COPY = 3,
		RHI_LOGIC_OP_AND_INVERTED = 4,
		RHI_LOGIC_OP_NO_OP = 5,
		RHI_LOGIC_OP_XOR = 6,
		RHI_LOGIC_OP_OR = 7,
		RHI_LOGIC_OP_NOR = 8,
		RHI_LOGIC_OP_EQUIVALENT = 9,
		RHI_LOGIC_OP_INVERT = 10,
		RHI_LOGIC_OP_OR_REVERSE = 11,
		RHI_LOGIC_OP_COPY_INVERTED = 12,
		RHI_LOGIC_OP_OR_INVERTED = 13,
		RHI_LOGIC_OP_NAND = 14,
		RHI_LOGIC_OP_SET = 15,
	};

	enum RHIPipelineDynamicState : int
	{
		RHI_DYNAMIC_STATE_VIEWPORT = 0,
		RHI_DYNAMIC_STATE_SCISSOR = 1,
		RHI_DYNAMIC_STATE_LINE_WIDTH = 2,
		RHI_DYNAMIC_STATE_DEPTH_BIAS = 3,
		RHI_DYNAMIC_STATE_BLEND_CONSTANTS = 4,
		RHI_DYNAMIC_STATE_DEPTH_BOUNDS = 5,
		RHI_DYNAMIC_STATE_STENCIL_COMPARE_MASK = 6,
		RHI_DYNAMIC_STATE_STENCIL_WRITE_MASK = 7,
		RHI_DYNAMIC_STATE_STENCIL_REFERENCE = 8,
	};

	enum RHIFrontFace : int
	{
		RHI_FRONT_FACE_COUNTER_CLOCKWISE = 0,
		RHI_FRONT_FACE_CLOCKWISE = 1,
	};

	enum RHIPolygonMode : int
	{
		RHI_POLYGON_MODE_FILL = 0,
		RHI_POLYGON_MODE_LINE = 1,
		RHI_POLYGON_MODE_POINT = 2,
	};

	enum RHIFilter : int
	{
		RHI_FILTER_NEAREST = 0,
		RHI_FILTER_LINEAR = 1,
	};

	enum RHISamplerAddressMode : int
	{
		RHI_SAMPLER_ADDRESS_MODE_REPEAT = 0,
		RHI_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT = 1,
		RHI_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE = 2,
		RHI_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER = 3,
		RHI_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE = 4,
	};

	enum RHIBorderColor : int
	{
		RHI_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK = 0,
		RHI_BORDER_COLOR_INT_TRANSPARENT_BLACK = 1,
		RHI_BORDER_COLOR_FLOAT_OPAQUE_BLACK = 2,
		RHI_BORDER_COLOR_INT_OPAQUE_BLACK = 3,
		RHI_BORDER_COLOR_FLOAT_OPAQUE_WHITE = 4,
		RHI_BORDER_COLOR_INT_OPAQUE_WHITE = 5,
	};

	enum RHIVertexInputRate : int
	{
		RHI_VERTEX_INPUT_RATE_VERTEX = 0,
		RHI_VERTEX_INPUT_RATE_INSTANCE = 1,
	};

	enum RHIPresentMode : int
	{
		RHI_PRESENT_MODE_IMMEDIATE_KHR = 0,
		RHI_PRESENT_MODE_MAILBOX_KHR = 1,
		RHI_PRESENT_MODE_FIFO_KHR = 2,
		RHI_PRESENT_MODE_FIFO_RELAXED_KHR = 3,
	};

	// Typedefs for flag types
	typedef uint32_t RHIAccessFlags;
	typedef uint32_t RHIPipelineStageFlags;
	typedef uint32_t RHIBufferUsageFlags;
	typedef uint32_t RHIMemoryPropertyFlags;
	typedef uint32_t RHIImageUsageFlags;
	typedef uint32_t RHIFenceCreateFlags;
	typedef uint32_t RHICommandBufferUsageFlags;
	typedef uint32_t RHICullModeFlags;
	typedef uint32_t RHIShaderStageFlags;
	typedef uint32_t RHISampleCountFlags;
	typedef uint32_t RHIDescriptorTypeFlags;
	typedef uint32_t RHIPrimitiveTopologyFlags;
	typedef uint32_t RHISharingModeFlags;
	typedef uint32_t RHIImageViewTypeFlags;
	typedef uint32_t RHIImageLayoutFlags;
	typedef uint32_t RHIPipelineBindPointFlags;
	typedef uint32_t RHIBlendOpFlags;
	typedef uint32_t RHIStencilOpFlags;	
	typedef uint32_t RHIBlendFactorFlags;
	typedef uint32_t RHICommandBufferLevelFlags;
	typedef uint32_t RHIPhysicalDeviceTypeFlags;
	typedef uint32_t RHIImageTypeFlags;
	typedef uint32_t RHIStructTypeFlags;
	typedef uint32_t RHISamplerMipmapModeFlags;
	typedef uint32_t RHIDescriptorSetLayoutBindingFlags;
	typedef uint64_t RHIDeviceSize;
	typedef uint32_t RHIBufferCreateFlags;
	typedef uint32_t RHIQueryPipelineStatisticFlags;
	typedef uint32_t RHIQueryControlFlags;
	typedef uint32_t RHICommandPoolCreateFlags;
	typedef uint32_t RHIDescriptorPoolCreateFlags;
	typedef uint32_t RHIDeviceCreateFlags;
 typedef uint32_t RHIDeviceQueueCreateFlags;
	typedef uint32_t RHIFormatFeatureFlags;
	typedef uint32_t RHIFramebufferCreateFlags;
	typedef uint32_t RHIPipelineCreateFlags;
	typedef uint32_t RHIImageCreateFlags;
	typedef uint32_t RHIImageViewCreateFlags;
	typedef uint32_t RHIInstanceCreateFlags;
	typedef uint32_t RHIMemoryHeapFlagBits;
	typedef uint32_t RHIPipelineColorBlendStateCreateFlags;
	typedef uint32_t RHIPipelineDepthStencilStateCreateFlags;
	typedef uint32_t RHIPipelineDynamicStateCreateFlags;
	typedef uint32_t RHIPipelineInputAssemblyStateCreateFlags;
	typedef uint32_t RHIPipelineLayoutCreateFlags;
	typedef uint32_t RHIPipelineMultisampleStateCreateFlags;
	typedef uint32_t RHISampleMask;
	typedef uint32_t RHIPipelineRasterizationStateCreateFlags;
	typedef uint32_t RHIPipelineShaderStageCreateFlags;
	typedef uint32_t RHIPipelineTessellationStateCreateFlags;
	typedef uint32_t RHIPipelineVertexInputStateCreateFlags;
	typedef uint32_t RHIPipelineViewportStateCreateFlags;
	typedef uint32_t RHIQueueFlags;
	typedef uint32_t RHIRenderPassCreateFlags;
	typedef uint32_t RHISamplerCreateFlags;
	typedef uint32_t RHISemaphoreCreateFlags;
	typedef uint32_t RHIShaderModuleCreateFlags;
	typedef uint32_t RHISubpassDependencyFlags;
	typedef uint32_t RHISubpassDescriptionFlags;
	typedef uint32_t RHIColorComponentFlags;
}

namespace BinRenderer
{
	struct MeshSourceDesc
	{
		std::string m_mesh_file;
		bool operator==(const MeshSourceDesc& other) const
		{
			return m_mesh_file == other.m_mesh_file;
		}
		size_t getHashValue() const
		{
			return std::hash<std::string> {}(m_mesh_file);
		}
	};

	struct MaterialSourceDesc
	{
		std::string m_base_color_texture_file;
		std::string m_normal_texture_file;
		std::string m_metallic_roughness_texture_file;
		std::string m_occlusion_texture_file;
		std::string m_emissive_texture_file;

		bool operator==(const MaterialSourceDesc& other) const
		{
			return m_base_color_texture_file == other.m_base_color_texture_file &&
				   m_normal_texture_file == other.m_normal_texture_file &&
				   m_metallic_roughness_texture_file == other.m_metallic_roughness_texture_file &&
				   m_occlusion_texture_file == other.m_occlusion_texture_file &&
				   m_emissive_texture_file == other.m_emissive_texture_file;
		}

		size_t getHashValue() const
		{
			// TODO : Create a better hash function at other place
			size_t h1 = std::hash<std::string> {}(m_base_color_texture_file);
			size_t h2 = std::hash<std::string> {}(m_normal_texture_file);
			size_t h3 = std::hash<std::string> {}(m_metallic_roughness_texture_file);
			size_t h4 = std::hash<std::string> {}(m_occlusion_texture_file);
			size_t h5 = std::hash<std::string> {}(m_emissive_texture_file);
			// Combine the hash values
			return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4);
		}
	};
}

template<>
struct std::hash<BinRenderer::MeshSourceDesc>
{
	size_t operator()(const BinRenderer::MeshSourceDesc& desc) const noexcept
	{
		return desc.getHashValue();
	}
};

template<>
struct std::hash<BinRenderer::MaterialSourceDesc>
{
	size_t operator()(const BinRenderer::MaterialSourceDesc& desc) const noexcept
	{
		return desc.getHashValue();
	}
};