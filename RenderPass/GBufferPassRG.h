#pragma once

#include "RGPassBase.h"

namespace BinRenderer
{
	/**
	 * @brief G-Buffer 렌더 패스 데이터
	 * 
	 * RenderGraph에서 자동으로 관리되는 G-Buffer 리소스
	 */
	struct GBufferPassData
	{
		RGTextureHandle albedo;      // RGB: Albedo, A: AO
		RGTextureHandle normal;           // RGB: Normal (World Space)
		RGTextureHandle position;// RGB: Position (World Space)
		RGTextureHandle metallicRoughness;  // R: Metallic, G: Roughness
		RGTextureHandle depth;     // Depth/Stencil
	};

	/**
	 * @brief G-Buffer 렌더 패스 (Deferred Rendering 1단계)
	 * 
	 * RenderGraph 기반으로 재구현된 G-Buffer Pass
	 * 
	 * @features
	 * - 자동 리소스 관리 (RenderGraph)
	 * - 자동 의존성 해결
	 * - MRT (Multiple Render Targets) 렌더링
	 * 
	 * @outputs
	 * - Albedo (R8G8B8A8_UNORM)
	 * - Normal (R16G16B16A16_SFLOAT, World Space)
	 * - Position (R16G16B16A16_SFLOAT, World Space)
	 * - Metallic-Roughness (R8G8_UNORM)
	 * - Depth (D32_SFLOAT)
	 */
	class GBufferPassRG : public RGPass<GBufferPassData>
	{
	public:
		GBufferPassRG(RHI* rhi);
		~GBufferPassRG() override;

		// RGPass 인터페이스 구현
		void setup(GBufferPassData& data, RenderGraphBuilder& builder) override;
		void execute(const GBufferPassData& data, RHI* rhi, uint32_t frameIndex) override;

		// 기존 API 호환 (옵션)
		bool initialize() override;
		void shutdown() override;

		// G-Buffer 텍스처 핸들 접근
		RGTextureHandle getAlbedoHandle() const { return getData().albedo; }
		RGTextureHandle getNormalHandle() const { return getData().normal; }
		RGTextureHandle getPositionHandle() const { return getData().position; }
		RGTextureHandle getMetallicRoughnessHandle() const { return getData().metallicRoughness; }
		RGTextureHandle getDepthHandle() const { return getData().depth; }

	private:
		// 파이프라인 및 셰이더 (RenderGraph와 독립적)
		RHIPipelineHandle pipeline_;
		RHIShaderHandle vertexShader_;
		RHIShaderHandle fragmentShader_;

		void createPipeline();
		void destroyPipeline();
	};

} // namespace BinRenderer
