#pragma once

#include "RGPassBase.h"

namespace BinRenderer
{
	/**
	 * @brief GUI Pass 데이터
	 */
	struct GUIPassData
	{
		// 입력
		RGTextureHandle sceneIn;  // LDR from PostProcessPass

		// 출력
		RGTextureHandle guiOut;  // Final Output (Scene + GUI)
	};

	/**
	 * @brief GUI Pass (ImGui UI 오버레이)
	 * 
	 * @features
	 * - ImGui 렌더링
	 * - UI 오버레이
	 * - 최종 출력
	 * 
	 * @inputs
	 * - Scene Image (LDR, R8G8B8A8_UNORM)
	 * 
	 * @outputs
	 * - Final Image (Scene + GUI, R8G8B8A8_UNORM)
	 */
	class GUIPassRG : public RGPass<GUIPassData>
	{
	public:
		GUIPassRG(RHI* rhi);
		~GUIPassRG() override;

		// RGPass 인터페이스
		void setup(GUIPassData& data, RenderGraphBuilder& builder) override;
		void execute(const GUIPassData& data, RHI* rhi, uint32_t frameIndex) override;

		// 기존 API 호환
		bool initialize() override;
		void shutdown() override;

		// 입력 핸들 설정 (setup 전에 호출)
		void setSceneHandle(RGTextureHandle handle) { sceneHandle_ = handle; }

		// 출력 핸들
		RGTextureHandle getGUIHandle() const { return getData().guiOut; }

	private:
		// 입력 핸들
		RGTextureHandle sceneHandle_;

		// 파이프라인 (ImGui용)
		RHIPipelineHandle pipeline_;
		RHIShaderHandle vertexShader_;
		RHIShaderHandle fragmentShader_;

		void createPipeline();
		void destroyPipeline();
	};

} // namespace BinRenderer
