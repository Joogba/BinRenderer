#pragma once

#include "../Core/RHIType.h"
#include "../Core/RHIHandle.h"
#include "RHIImageStructs.h"
#include "RHICommonStructs.h"
#include "RHIRenderPassStructs.h"
#include <vector>
#include <glm/glm.hpp>

namespace BinRenderer
{
	/**
	 * @brief GPU Instancing용 Per-instance 데이터
	 * 
	 * 16-byte aligned for GPU buffer
	 * Binding 1에서 VK_VERTEX_INPUT_RATE_INSTANCE로 사용
	 */
	struct InstanceData
	{
		glm::mat4 modelMatrix;       // 64 bytes - Per-instance transform
		uint32_t materialOffset;     // 4 bytes  - Per-instance material override
		uint32_t padding[3];         // 12 bytes - Padding for 16-byte alignment

		InstanceData()
			: modelMatrix(1.0f)
			, materialOffset(0)
			, padding{ 0, 0, 0 }
		{
		}

		InstanceData(const glm::mat4& model, uint32_t matOffset = 0)
			: modelMatrix(model)
			, materialOffset(matOffset)
			, padding{ 0, 0, 0 }
		{
		}
	};

	static_assert(sizeof(InstanceData) == 80, "InstanceData must be 80 bytes (64 + 4 + 12)");
	static_assert(sizeof(InstanceData) % 16 == 0, "InstanceData must be 16-byte aligned");

	/**
	 * @brief 버텍스 입력 바인딩
	 */
	struct RHIVertexInputBinding
	{
		uint32_t binding = 0;
		uint32_t stride = 0;
		RHIVertexInputRate inputRate = RHI_VERTEX_INPUT_RATE_VERTEX;
	};

	/**
	 * @brief 버텍스 입력 속성
	 */
	struct RHIVertexInputAttribute
	{
		uint32_t location = 0;
		uint32_t binding = 0;
		RHIFormat format = RHI_FORMAT_UNDEFINED;
		uint32_t offset = 0;
	};

	/**
	 * @brief 버텍스 입력 상태
	 */
	struct RHIPipelineVertexInputStateCreateInfo
	{
		std::vector<RHIVertexInputBinding> bindings;
		std::vector<RHIVertexInputAttribute> attributes;
	};

	/**
	 * @brief 입력 어셈블리 상태
	 */
	struct RHIPipelineInputAssemblyStateCreateInfo
	{
		RHIPrimitiveTopology topology = RHI_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		bool primitiveRestartEnable = false;
	};

	/**
	 * @brief 뷰포트 상태
	 */
	struct RHIPipelineViewportStateCreateInfo
	{
		uint32_t viewportCount = 1;
		const RHIViewport* pViewports = nullptr;
		uint32_t scissorCount = 1;
		const RHIRect2D* pScissors = nullptr;
	};

	/**
	 * @brief 래스터라이제이션 상태
	 */
	struct RHIPipelineRasterizationStateCreateInfo
	{
		RHICullModeFlags cullMode = RHI_CULL_MODE_BACK_BIT;
		RHIFrontFace frontFace = RHI_FRONT_FACE_COUNTER_CLOCKWISE;
		RHIPolygonMode polygonMode = RHI_POLYGON_MODE_FILL;
		float lineWidth = 1.0f;
		bool depthClampEnable = false;
		bool rasterizerDiscardEnable = false;
		bool depthBiasEnable = false;
		float depthBiasConstantFactor = 0.0f;
        float depthBiasClamp = 0.0f;
        float depthBiasSlopeFactor = 0.0f;
	};

	/**
	 * @brief 멀티샘플 상태
	 */
	struct RHIPipelineMultisampleStateCreateInfo
	{
		RHISampleCountFlagBits rasterizationSamples = RHI_SAMPLE_COUNT_1_BIT;
		bool sampleShadingEnable = false;
		float minSampleShading = 1.0f;
		const RHISampleMask** pSampleMask = nullptr;
        bool alphaToCoverageEnable = false;
        bool alphaToOneEnable = false;
	};

	struct RHIStencilOpState
    {
        RHIStencilOp failOp;
        RHIStencilOp passOp;
        RHIStencilOp depthFailOp;
        RHICompareOp compareOp;
        uint32_t compareMask;
        uint32_t writeMask;
        uint32_t reference;
	};

	/**
	 * @brief 깊이 스텐실 상태
	 */
	struct RHIPipelineDepthStencilStateCreateInfo
	{
		bool depthTestEnable = false;
		bool depthWriteEnable = false;
		RHICompareOp depthCompareOp = RHI_COMPARE_OP_LESS;
		bool depthBoundsTestEnable = false;
		bool stencilTestEnable = false;
		RHIStencilOpState front;
        RHIStencilOpState back;
        float minDepthBounds = 0.0f;
        float maxDepthBounds = 1.0f;
	};

	/**
	 * @brief 컬러 블렌드 어태치먼트
	 */
	struct RHIPipelineColorBlendAttachment
	{
		bool blendEnable = false;
		RHIBlendFactor srcColorBlendFactor = RHI_BLEND_FACTOR_ONE;
		RHIBlendFactor dstColorBlendFactor = RHI_BLEND_FACTOR_ZERO;
		RHIBlendOp colorBlendOp = RHI_BLEND_OP_ADD;
		RHIBlendFactor srcAlphaBlendFactor = RHI_BLEND_FACTOR_ONE;
		RHIBlendFactor dstAlphaBlendFactor = RHI_BLEND_FACTOR_ZERO;
		RHIBlendOp alphaBlendOp = RHI_BLEND_OP_ADD;
		RHIColorComponentFlags colorWriteMask = 0xF;
	};

	/**
	 * @brief 컬러 블렌드 상태
	 */
	struct RHIPipelineColorBlendStateCreateInfo
	{
		bool logicOpEnable = false;
		RHILogicOp logicOp = RHI_LOGIC_OP_COPY;
		std::vector<RHIPipelineColorBlendAttachment> attachments;
		float blendConstants[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	};

	struct RHIPushConstantRange
    {
        RHIShaderStageFlags stageFlags;
        uint32_t offset;
        uint32_t size;
    };

	/**
	 * @brief 파이프라인 생성 정보
	 */
	struct RHIPipelineCreateInfo
	{
		std::vector<RHIShaderHandle> shaderStages;
		RHIPipelineVertexInputStateCreateInfo vertexInputState;
		RHIPipelineInputAssemblyStateCreateInfo inputAssemblyState;
		RHIPipelineViewportStateCreateInfo viewportState;
		RHIPipelineRasterizationStateCreateInfo rasterizationState;
		RHIPipelineMultisampleStateCreateInfo multisampleState;
		RHIPipelineDepthStencilStateCreateInfo depthStencilState;
		RHIPipelineColorBlendStateCreateInfo colorBlendState;
		std::vector<RHIPipelineDynamicState> dynamicStates;
		
		// ========================================
		// Render Pass (Legacy) vs Dynamic Rendering (Vulkan 1.3+)
		// ========================================
		RHIRenderPass* renderPass = nullptr;  // Legacy render pass (VK_NULL_HANDLE면 Dynamic Rendering 사용)
		uint32_t subpass = 0;

		//  Dynamic Rendering 지원 (Vulkan 1.3+)
		bool useDynamicRendering = false;
		std::vector<RHIFormat> colorAttachmentFormats;  // Dynamic rendering color formats
		RHIFormat depthAttachmentFormat = RHI_FORMAT_UNDEFINED;  // Dynamic rendering depth format
		RHIFormat stencilAttachmentFormat = RHI_FORMAT_UNDEFINED;  // Dynamic rendering stencil format

		//  GPU Instancing 지원
		bool enableInstancing = false;

		//  Descriptor Set Layouts
		std::vector<RHIDescriptorSetLayoutHandle> descriptorSetLayouts;

		//  Push Constants
		std::vector<RHIPushConstantRange> pushConstantRanges;
	};

	struct RHIPipelineLayoutCreateInfo
    {
        RHIPipelineLayoutCreateFlags flags;
        uint32_t setLayoutCount;
        const RHIDescriptorSetLayout* pSetLayouts;
        uint32_t pushConstantRangeCount;
        const RHIPushConstantRange* pPushConstantRanges;
    };

	struct RHIPipelineTessellationStateCreateInfo
    {
        RHIPipelineTessellationStateCreateFlags flags;
        uint32_t patchControlPoints;
    };

	struct RHIPipelineShaderStageCreateInfo
    {
        RHIPipelineShaderStageCreateFlags flags;
        RHIShaderStageFlagBits stage;
        RHIShader* module;
        const char* pName;
        // const RHISpecializationInfo* pSpecializationInfo; // Forward declared
    };

	struct RHISpecializationMapEntry
    {
        uint32_t constantID;
        uint32_t offset;
        size_t size;
    };

	struct RHISpecializationInfo
    {
        uint32_t mapEntryCount;
        const RHISpecializationMapEntry* pMapEntries;
        size_t dataSize;
        const void* pData;
    };

	struct RHIComputePipelineCreateInfo
    {
        RHIPipelineCreateFlags flags;
        RHIPipelineShaderStageCreateInfo* stage;
        RHIPipelineLayout* layout;
        RHIPipelineHandle basePipelineHandle;
        int32_t basePipelineIndex;
    };

	/**
	 * @brief GPU Instancing용 Vertex Input 헬퍼
	 */
	namespace RHIInstanceHelper
	{
		/**
		 * @brief Instance buffer의 Binding Description 반환
		 * @return Binding 1에 대한 설정
		 */
		inline RHIVertexInputBinding getInstanceBinding()
		{
			RHIVertexInputBinding binding;
			binding.binding = 1;  // Instance buffer는 binding 1
			binding.stride = 80;  // sizeof(InstanceData) = 80
			binding.inputRate = RHI_VERTEX_INPUT_RATE_INSTANCE;
			return binding;
		}

		/**
		 * @brief Instance attributes (location 10-14) 반환
		 * @return 5개의 attribute (mat4 = 4개 + materialOffset = 1개)
		 */
		inline std::vector<RHIVertexInputAttribute> getInstanceAttributes()
		{
			std::vector<RHIVertexInputAttribute> attributes(5);

			// mat4 modelMatrix - 4개의 vec4로 분할
			for (uint32_t i = 0; i < 4; i++)
			{
				attributes[i].location = 10 + i;
				attributes[i].binding = 1;
				attributes[i].format = RHI_FORMAT_R32G32B32A32_SFLOAT;
				attributes[i].offset = sizeof(float) * 4 * i;
			}

			// uint32_t materialOffset
			attributes[4].location = 14;
			attributes[4].binding = 1;
			attributes[4].format = RHI_FORMAT_R32_UINT;
			attributes[4].offset = 64;  // sizeof(glm::mat4)

			return attributes;
		}
	}

} // namespace BinRenderer
