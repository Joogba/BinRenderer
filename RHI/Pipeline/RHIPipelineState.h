#pragma once

#include "../Core/RHIType.h"
#include "RHIPipeline.h"
#include "RHIPipelineLayout.h"
#include "RHIRenderPass.h"
#include "RHIFramebuffer.h"
#include "RHIDescriptor.h"

namespace BinRenderer
{
	/**
	 * @brief 파이프라인 상태 정보
	 */
	struct RHIPipelineState
	{
		// 버텍스 입력
		struct VertexInputState
		{
			std::vector<struct RHIVertexInputBinding> bindings;
			std::vector<struct RHIVertexInputAttribute> attributes;
		} vertexInput;

		// 입력 어셈블리
		struct InputAssemblyState
		{
			RHIPrimitiveTopology topology = RHI_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			bool primitiveRestartEnable = false;
		} inputAssembly;

		// 래스터라이제이션
		struct RasterizationState
		{
			RHICullModeFlags cullMode = RHI_CULL_MODE_BACK_BIT;
			RHIFrontFace frontFace = RHI_FRONT_FACE_COUNTER_CLOCKWISE;
			RHIPolygonMode polygonMode = RHI_POLYGON_MODE_FILL;
			float lineWidth = 1.0f;
			bool depthClampEnable = false;
			bool rasterizerDiscardEnable = false;
		} rasterization;

		// 멀티샘플
		struct MultisampleState
		{
			RHISampleCountFlagBits samples = RHI_SAMPLE_COUNT_1_BIT;
			bool sampleShadingEnable = false;
		} multisample;

		// 깊이 스텐실
		struct DepthStencilState
		{
			bool depthTestEnable = true;
			bool depthWriteEnable = true;
			RHICompareOp depthCompareOp = RHI_COMPARE_OP_LESS;
			bool stencilTestEnable = false;
		} depthStencil;

		// 컬러 블렌드
		struct ColorBlendState
		{
			bool logicOpEnable = false;
			RHILogicOp logicOp = RHI_LOGIC_OP_COPY;
			struct Attachment
			{
				bool blendEnable = false;
				RHIColorComponentFlags colorWriteMask = 0xF;
			} attachment;
		} colorBlend;

		// 다이나믹 스테이트
		std::vector<RHIPipelineDynamicState> dynamicStates;
	};

} // namespace BinRenderer
