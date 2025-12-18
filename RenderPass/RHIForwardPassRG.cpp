#include "RHIForwardPassRG.h"
#include "../Core/Logger.h"

namespace BinRenderer
{
	RHIForwardPassRG::RHIForwardPassRG(RHI* rhi)
		: RGPass<RHIForwardPassData>(rhi, "RHIForwardPass")
	{
	}

	RHIForwardPassRG::~RHIForwardPassRG()
	{
		shutdown();
	}

	void RHIForwardPassRG::setup(RHIForwardPassData& data, RenderGraphBuilder& builder)
	{
		// Color output
		RGTextureDesc colorDesc{};
		colorDesc.width = 1280;
		colorDesc.height = 720;
		colorDesc.format = RHI_FORMAT_R8G8B8A8_UNORM;
		colorDesc.usage = RHI_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | RHI_IMAGE_USAGE_SAMPLED_BIT;
		data.colorOutput = builder.createTexture(colorDesc);

		// Depth output
		RGTextureDesc depthDesc{};
		depthDesc.width = 1280;
		depthDesc.height = 720;
		depthDesc.format = RHI_FORMAT_D32_SFLOAT;
		depthDesc.usage = RHI_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		data.depthOutput = builder.createTexture(depthDesc);

		// Write outputs
		builder.writeTexture(data.colorOutput);
		builder.writeTexture(data.depthOutput);
	}

	void RHIForwardPassRG::execute(const RHIForwardPassData& data, RHI* rhi, uint32_t frameIndex)
	{
		if (!scene_)
			return;

		// TODO: 실제 렌더링 구현
		// 1. RenderPass 시작
		// 2. Pipeline 바인딩
		// 3. Scene의 모든 모델 그리기
		// 4. RenderPass 종료

		// Scene의 모든 노드 렌더링
		const auto& nodes = scene_->getNodes();
		for (const auto& node : nodes)
		{
			if (!node.visible || !node.model)
				continue;

			// TODO: Transform 설정
			// TODO: Draw
			// node.model->draw(rhi, 1);
		}
	}

	bool RHIForwardPassRG::initialize()
	{
		createPipeline();
		return true;
	}

	void RHIForwardPassRG::shutdown()
	{
		destroyPipeline();
	}

	void RHIForwardPassRG::createPipeline()
	{
		// TODO: 파이프라인 생성
		// vertexShader_ = rhi_->createShader(...);
		// fragmentShader_ = rhi_->createShader(...);
		// pipeline_ = rhi_->createPipeline(...);
	}

	void RHIForwardPassRG::destroyPipeline()
	{
		if (vertexShader_)
		{
			rhi_->destroyShader(vertexShader_);
			vertexShader_ = nullptr;
		}

		if (fragmentShader_)
		{
			rhi_->destroyShader(fragmentShader_);
			fragmentShader_ = nullptr;
		}

		if (pipeline_)
		{
			rhi_->destroyPipeline(pipeline_);
			pipeline_ = nullptr;
		}
	}

} // namespace BinRenderer
