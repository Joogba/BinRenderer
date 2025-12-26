#include "LightingPassRG.h"
#include "../Core/Logger.h"

namespace BinRenderer
{
	LightingPassRG::LightingPassRG(RHI* rhi)
		: RGPass<LightingPassData>(rhi, "LightingPass")
	{
	}

	LightingPassRG::~LightingPassRG()
	{
		shutdown();
	}

	bool LightingPassRG::initialize()
	{
		printLog("[LightingPassRG] Initializing...");
		createPipeline();
		printLog("[LightingPassRG] Initialized successfully");
		return true;
	}

	void LightingPassRG::shutdown()
	{
		destroyPipeline();
	}

	void LightingPassRG::setup(LightingPassData& data, RenderGraphBuilder& builder)
	{
		printLog("[LightingPassRG] Setup - Declaring inputs and outputs");

		// 입력: G-Buffer와 Shadow Map (자동 의존성!)
		data.shadowMapIn = builder.readTexture(shadowMapHandle_);
		data.albedoIn = builder.readTexture(albedoHandle_);
		data.normalIn = builder.readTexture(normalHandle_);
		data.positionIn = builder.readTexture(positionHandle_);
		data.metallicRoughnessIn = builder.readTexture(metallicRoughnessHandle_);
		data.depthIn = builder.readTexture(depthHandle_);

		// 출력: HDR Lighting
		RGTextureDesc lightingDesc;
		lightingDesc.name = "Lighting_HDR";
		lightingDesc.width = width_;
		lightingDesc.height = height_;
		lightingDesc.format = RHI_FORMAT_R16G16B16A16_SFLOAT;
		lightingDesc.usage = RHI_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | 
		         RHI_IMAGE_USAGE_SAMPLED_BIT;

		data.lightingOut = builder.writeTexture(builder.createTexture(lightingDesc));
	}

	void LightingPassRG::execute(const LightingPassData& data, RHI* rhi, uint32_t frameIndex)
	{
		// TODO: 실제 Deferred Lighting
		// - PBR 계산
		// - Shadow Map 샘플링
		// - IBL
	}

	void LightingPassRG::createPipeline()
	{
		// TODO: Lighting 파이프라인 생성
	}

	void LightingPassRG::destroyPipeline()
	{
		if (pipelineHandle_.isValid()) {
			rhi_->destroyPipeline(pipelineHandle_);
		}

		if (vertexShaderHandle_.isValid()) {
			rhi_->destroyShader(vertexShaderHandle_);
		}

		if (fragmentShaderHandle_.isValid()) {
			rhi_->destroyShader(fragmentShaderHandle_);
		}
	}

} // namespace BinRenderer
