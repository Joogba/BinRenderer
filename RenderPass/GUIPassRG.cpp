#include "GUIPassRG.h"
#include "../Core/Logger.h"

namespace BinRenderer
{
	GUIPassRG::GUIPassRG(RHI* rhi)
		: RGPass<GUIPassData>(rhi, "GUIPass")
	{
	}

	GUIPassRG::~GUIPassRG()
	{
		shutdown();
	}

	bool GUIPassRG::initialize()
	{
		printLog("[GUIPassRG] Initializing...");
		createPipeline();
		printLog("[GUIPassRG] Initialized successfully");
		return true;
	}

	void GUIPassRG::shutdown()
	{
		destroyPipeline();
	}

	void GUIPassRG::setup(GUIPassData& data, RenderGraphBuilder& builder)
	{
		printLog("[GUIPassRG] Setup - Declaring inputs and outputs");

		// 입력: Scene (LDR) (자동 의존성!)
		data.sceneIn = builder.readTexture(sceneHandle_);

		// 출력: Final Output (Scene + GUI)
		RGTextureDesc guiDesc;
		guiDesc.name = "GUI_Final";
		guiDesc.width = width_;
		guiDesc.height = height_;
		guiDesc.format = RHI_FORMAT_R8G8B8A8_UNORM;
		guiDesc.usage = RHI_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | 
		  RHI_IMAGE_USAGE_SAMPLED_BIT;

		data.guiOut = builder.writeTexture(builder.createTexture(guiDesc));

		//  최종 출력 설정
		builder.setFinalOutput(data.guiOut);
	}

	void GUIPassRG::execute(const GUIPassData& data, RHI* rhi, uint32_t frameIndex)
	{
		// TODO: 실제 GUI 렌더링
		// 1. Scene 이미지를 먼저 그리기 (Copy 또는 Fullscreen Quad)
		// 2. ImGui 렌더링 (UI 오버레이)
	}

	void GUIPassRG::createPipeline()
	{
		// TODO: GUI 파이프라인 생성
		// - ImGui용 파이프라인
		// - Alpha Blending 활성화
	}

	void GUIPassRG::destroyPipeline()
	{
		if (pipeline_.isValid()) {
			rhi_->destroyPipeline(pipeline_);
			pipeline_ = {};
		}

		if (vertexShader_.isValid()) {
			rhi_->destroyShader(vertexShader_);
			vertexShader_ = {};
		}

		if (fragmentShader_.isValid()) {
			rhi_->destroyShader(fragmentShader_);
			fragmentShader_ = {};
		}
	}

} // namespace BinRenderer
