#include "ForwardPassRG.h"
#include "../Core/Logger.h"

namespace BinRenderer
{
	ForwardPassRG::ForwardPassRG(RHI* rhi)
		: RGPass<ForwardPassData>(rhi, "ForwardPass")
	{
	}

	ForwardPassRG::~ForwardPassRG()
	{
		shutdown();
	}

	bool ForwardPassRG::initialize()
	{
		printLog("[ForwardPassRG] Initializing...");
		createPipeline();
		printLog("[ForwardPassRG] Initialized successfully");
		return true;
	}

	void ForwardPassRG::shutdown()
	{
		destroyPipeline();
	}

	void ForwardPassRG::setup(ForwardPassData& data, RenderGraphBuilder& builder)
	{
		printLog("[ForwardPassRG] Setup - Declaring inputs and outputs");

		// 입력: Lighting HDR + Depth (자동 의존성!)
		data.lightingIn = builder.readTexture(lightingHandle_);
		data.depthIn = builder.readTexture(depthHandle_);

		// 출력: Forward Output (투명 오브젝트 추가)
		RGTextureDesc forwardDesc;
		forwardDesc.name = "Forward_Output";
		forwardDesc.width = width_;
		forwardDesc.height = height_;
		forwardDesc.format = RHI_FORMAT_R16G16B16A16_SFLOAT;
		forwardDesc.usage = RHI_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | 
		  RHI_IMAGE_USAGE_SAMPLED_BIT;

		data.forwardOut = builder.writeTexture(builder.createTexture(forwardDesc));
	}

	void ForwardPassRG::execute(const ForwardPassData& data, RHI* rhi, uint32_t frameIndex)
	{
		// TODO: 실제 Forward Rendering
		// - 투명 오브젝트 렌더링
		// - Alpha Blending
		// - Depth Test
	}

	void ForwardPassRG::createPipeline()
	{
		// TODO: Forward 파이프라인 생성
		// - Alpha Blending 활성화
		// - Depth Test 활성화 (Write 비활성화)
	}

	void ForwardPassRG::destroyPipeline()
	{
		if (pipeline_) {
			rhi_->destroyPipeline(pipeline_);
			pipeline_ = nullptr;
		}

		if (vertexShader_) {
			rhi_->destroyShader(vertexShader_);
			vertexShader_ = nullptr;
		}

		if (fragmentShader_) {
			rhi_->destroyShader(fragmentShader_);
			fragmentShader_ = nullptr;
		}
	}

} // namespace BinRenderer
