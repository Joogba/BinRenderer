#pragma once

#include "RHIType.h"

namespace BinRenderer
{
	// 전방 선언
	class RHIBuffer;
	class RHIImage;
	class RHIShader;
	class RHIPipeline;
	class RHIPipelineLayout;
	class RHIDescriptorSet;
	class RHIRenderPass;
	class RHIFramebuffer;

	// 버퍼 생성 정보
	struct RHIBufferCreateInfo
	{
		RHIDeviceSize size = 0;
		RHIBufferUsageFlags usage = 0;
		RHIMemoryPropertyFlags memoryProperties = 0;
		const void* initialData = nullptr;
	};

	// 이미지 생성 정보
	struct RHIImageCreateInfo
	{
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t depth = 1;
		uint32_t mipLevels = 1;
		uint32_t arrayLayers = 1;
		RHIFormat format = RHI_FORMAT_UNDEFINED;
		RHIImageUsageFlags usage = 0;
		RHISampleCountFlagBits samples = RHI_SAMPLE_COUNT_1_BIT;
		RHIImageTiling tiling = RHI_IMAGE_TILING_OPTIMAL;
		uint32_t flags = 0;  // ✅ Cubemap flag 등
	};

	// 셰이더 생성 정보
	struct RHIShaderCreateInfo
	{
		RHIShaderStageFlags stage = 0;
		const char* entryPoint = "main";
		std::string name;  // 셰이더 이름 (디버깅용)
		std::vector<uint32_t> code;
	};

	// 버텍스 입력 바인딩
	struct RHIVertexInputBinding
	{
		uint32_t binding = 0;
		uint32_t stride = 0;
		RHIVertexInputRate inputRate = RHI_VERTEX_INPUT_RATE_VERTEX;
	};

	// 버텍스 입력 속성
	struct RHIVertexInputAttribute
	{
		uint32_t location = 0;
		uint32_t binding = 0;
		RHIFormat format = RHI_FORMAT_UNDEFINED;
		uint32_t offset = 0;
	};

	// 버텍스 입력 상태
	struct RHIPipelineVertexInputStateCreateInfo
	{
		std::vector<RHIVertexInputBinding> bindings;
		std::vector<RHIVertexInputAttribute> attributes;
	};

	// 입력 어셈블리 상태
	struct RHIPipelineInputAssemblyStateCreateInfo
	{
		RHIPrimitiveTopology topology = RHI_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		bool primitiveRestartEnable = false;
	};

	// 뷰포트 상태
	struct RHIPipelineViewportStateCreateInfo
	{
		uint32_t viewportCount = 1;
		uint32_t scissorCount = 1;
	};

	// 래스터라이제이션 상태
	struct RHIPipelineRasterizationStateCreateInfo
	{
		RHICullModeFlags cullMode = RHI_CULL_MODE_BACK_BIT;
		RHIFrontFace frontFace = RHI_FRONT_FACE_COUNTER_CLOCKWISE;
		RHIPolygonMode polygonMode = RHI_POLYGON_MODE_FILL;
		float lineWidth = 1.0f;
		bool depthClampEnable = false;
		bool rasterizerDiscardEnable = false;
		bool depthBiasEnable = false;
	};

	// 멀티샘플 상태
	struct RHIPipelineMultisampleStateCreateInfo
	{
		RHISampleCountFlagBits rasterizationSamples = RHI_SAMPLE_COUNT_1_BIT;
		bool sampleShadingEnable = false;
		float minSampleShading = 1.0f;
	};

	// 깊이 스텐실 상태
	struct RHIPipelineDepthStencilStateCreateInfo
	{
		bool depthTestEnable = false;
		bool depthWriteEnable = false;
		RHICompareOp depthCompareOp = RHI_COMPARE_OP_LESS;
		bool stencilTestEnable = false;
	};

	// 컬러 블렌드 어태치먼트
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

	// 컬러 블렌드 상태
	struct RHIPipelineColorBlendStateCreateInfo
	{
		bool logicOpEnable = false;
		RHILogicOp logicOp = RHI_LOGIC_OP_COPY;
		std::vector<RHIPipelineColorBlendAttachment> attachments;
		float blendConstants[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	};

	// 다이나믹 상태
	struct RHIPipelineDynamicStateCreateInfo
	{
		std::vector<RHIPipelineDynamicState> dynamicStates;
	};

	// 파이프라인 생성 정보
	struct RHIPipelineCreateInfo
	{
		std::vector<RHIShader*> shaderStages;
		RHIPipelineVertexInputStateCreateInfo vertexInputState;
		RHIPipelineInputAssemblyStateCreateInfo inputAssemblyState;
		RHIPipelineViewportStateCreateInfo viewportState;
		RHIPipelineRasterizationStateCreateInfo rasterizationState;
		RHIPipelineMultisampleStateCreateInfo multisampleState;
		RHIPipelineDepthStencilStateCreateInfo depthStencilState;
		RHIPipelineColorBlendStateCreateInfo colorBlendState;
		std::vector<RHIPipelineDynamicState> dynamicStates;
		RHIRenderPass* renderPass = nullptr;
		uint32_t subpass = 0;
	};

} // namespace BinRenderer
