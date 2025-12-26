#pragma once

#include "RGPassBase.h"

namespace BinRenderer
{
	// Forward declarations
	class RHIScene;
	class RHIRenderer;

	/**
	 * @brief Forward Pass 데이터
	 */
	struct ForwardPassData
	{
		// 입력
		RGTextureHandle lightingIn;  // HDR from LightingPass
		RGTextureHandle depthIn;  // Depth from GBufferPass

		// 출력
		RGTextureHandle forwardOut;  // HDR + Transparent Objects
	};

	/**
	 * @brief Forward Pass (투명 오브젝트 렌더링)
	 * 
	 * @features
	 * - 투명 오브젝트 렌더링
	 * - Depth Test 사용
	 * - Alpha Blending
	 * 
	 * @inputs
	 * - Lighting HDR (from LightingPass)
	 * - Depth (from GBufferPass)
	 * 
	 * @outputs
	 * - Forward Output (HDR, R16G16B16A16_SFLOAT)
	 */
	class ForwardPassRG : public RGPass<ForwardPassData>
	{
	public:
		ForwardPassRG(RHI* rhi, RHIScene* scene = nullptr, RHIRenderer* renderer = nullptr);
		~ForwardPassRG() override;

		// RGPass 인터페이스
		void setup(ForwardPassData& data, RenderGraphBuilder& builder) override;
		void execute(const ForwardPassData& data, RHI* rhi, uint32_t frameIndex) override;

		// 기존 API 호환
		bool initialize() override;
		void shutdown() override;

		// Scene/Renderer 설정
		void setScene(RHIScene* scene) { scene_ = scene; }
		void setRenderer(RHIRenderer* renderer) { renderer_ = renderer; }

		// 입력 핸들 설정 (setup 전에 호출)
		void setLightingHandle(RGTextureHandle handle) { lightingHandle_ = handle; }
		void setDepthHandle(RGTextureHandle handle) { depthHandle_ = handle; }

		// 출력 핸들
		RGTextureHandle getForwardHandle() const { return getData().forwardOut; }

	private:
		// Scene/Renderer 참조
		RHIScene* scene_ = nullptr;
		RHIRenderer* renderer_ = nullptr;

		// 입력 핸들
		RGTextureHandle lightingHandle_;
		RGTextureHandle depthHandle_;

		// 파이프라인 리소스
		RHIPipelineHandle pipeline_;
		RHIPipelineLayout* pipelineLayout_ = nullptr;
		RHIShaderHandle vertexShader_;
		RHIShaderHandle fragmentShader_;

		//  Descriptor Sets (PBR용)
		RHIDescriptorSetLayoutHandle sceneDescriptorLayout_;     // Set 0: Scene UBO
		RHIDescriptorSetLayoutHandle materialDescriptorLayout_;  // Set 1: Materials
		RHIDescriptorSetLayoutHandle iblDescriptorLayout_;       // Set 2: IBL
		RHIDescriptorSetLayoutHandle shadowDescriptorLayout_;    // Set 3: Shadow
		
		RHIDescriptorPoolHandle descriptorPool_;
		std::vector<RHIDescriptorSetHandle> sceneDescriptorSets_;  // Per-frame
		RHIDescriptorSetHandle materialDescriptorSet_;   // 공유
		RHIDescriptorSetHandle iblDescriptorSet_;        // 공유
		RHIDescriptorSetHandle shadowDescriptorSet_;     // 공유

		//  Dummy Resources (Material/IBL/Shadow용)
		RHIBufferHandle dummyMaterialBuffer_;
		RHIImageHandle dummyTexture_;          // 흰색 1x1 texture
		RHIImageViewHandle dummyTextureView_;
		RHISamplerHandle dummySampler_;
		RHIImageHandle dummyCubemap_;          // 검은색 1x1 cubemap
		RHIImageViewHandle dummyCubemapView_;
		RHIImageHandle dummyShadowMap_;        // 1x1 depth texture
		RHIImageViewHandle dummyShadowMapView_;

		void createPipeline();
		void destroyPipeline();
		void createDescriptorSets();
		void destroyDescriptorSets();
		void updateDescriptorSets(uint32_t frameIndex);
		void createDummyResources();
		void destroyDummyResources();
	};

} // namespace BinRenderer
