#pragma once

#include "../RHI/Core/RHI.h"
#include "../RHI/Commands/RHICommandBuffer.h"
#include "../RenderPass/RenderGraph/RGGraph.h"
#include "../Core/RHIScene.h"
#include "../Scene/RHICamera.h"
#include "../Scene/Animation.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <unordered_map>

namespace BinRenderer
{
	// ========================================
	// Uniform 구조체들 (레거시에서 이식)
	// ========================================

	/**
	 * @brief 씬 전역 Uniform (Camera, Lighting)
	 */
	struct SceneUniform
	{
		alignas(16) glm::mat4 projection = glm::mat4(1.0f);
		alignas(16) glm::mat4 view = glm::mat4(1.0f);
		alignas(16) glm::vec3 cameraPos = glm::vec3(0.0f);
		alignas(4) float padding1 = 0.0f;
		alignas(16) glm::vec3 directionalLightDir = glm::vec3(0.0f, 1.0f, 0.0f);
		alignas(16) glm::vec3 directionalLightColor = glm::vec3(1.0f);
		alignas(16) glm::mat4 lightSpaceMatrix = glm::mat4(1.0f); // Shadow mapping
	};

	/**
	 * @brief 렌더링 옵션 Uniform
	 */
	struct OptionsUniform
	{
		alignas(4) int textureOn = 1;
		alignas(4) int shadowOn = 1;
		alignas(4) int discardOn = 1;
		alignas(4) int animationOn = 1;
		alignas(4) float specularWeight = 0.05f;
		alignas(4) float diffuseWeight = 1.0f;
		alignas(4) float emissiveWeight = 1.0f;
		alignas(4) float shadowOffset = 0.0f;
		alignas(4) int isInstanced = 0;
		alignas(4) float padding1 = 0.0f;
		alignas(4) float padding2 = 0.0f;
		alignas(4) float padding3 = 0.0f;
	};

	/**
	 * @brief 애니메이션 Bone 데이터
	 */
	struct BoneDataUniform
	{
		alignas(16) glm::mat4 boneMatrices[65];
		alignas(16) glm::vec4 animationData; // x = hasAnimation (0.0/1.0)
	};

	/**
	 * @brief PBR Push Constants
	 */
	struct PbrPushConstants
	{
		alignas(16) glm::mat4 model = glm::mat4(1.0f);
		alignas(4) uint32_t materialIndex = 0;
		alignas(4) float coeffs[15] = { 0.0f };
	};

	static_assert(sizeof(PbrPushConstants) == 128, "PbrPushConstants must be 128 bytes");

	/**
	 * @brief Frustum Culling 통계
	 */
	struct CullingStats
	{
		uint32_t totalMeshes = 0;
		uint32_t culledMeshes = 0;
		uint32_t renderedMeshes = 0;
	};

	/**
	 * @brief 플랫폼 독립적 Renderer (RHI 기반)
	 */
	class RHIRenderer
	{
	public:
		RHIRenderer(RHI* rhi, uint32_t maxFramesInFlight);
		~RHIRenderer();

		// ========================================
		// 초기화 및 종료
		// ========================================
		bool initialize(uint32_t width, uint32_t height, RHIFormat colorFormat, RHIFormat depthFormat);
		void shutdown();
		void resize(uint32_t width, uint32_t height);

		// ========================================
		// 프레임 렌더링
		// ========================================
		void beginFrame(uint32_t frameIndex);
		void updateUniforms(const RHICamera& camera, RHIScene& scene, uint32_t frameIndex, double time);
		void updateBoneData(const std::vector<RHIModel*>& models, uint32_t frameIndex);
		void render(RHICommandBuffer* cmd, RHIScene& scene, uint32_t frameIndex, RHIImageView* swapchainImageView);
		void endFrame(uint32_t frameIndex);

		// ========================================
		// Frustum Culling
		// ========================================
		void performFrustumCulling(std::vector<RHIModel*>& models);
		void updateViewFrustum(const glm::mat4& viewProjection);
		void setFrustumCullingEnabled(bool enabled) { frustumCullingEnabled_ = enabled; }
		bool isFrustumCullingEnabled() const { return frustumCullingEnabled_; }
		const CullingStats& getCullingStats() const { return cullingStats_; }

		// ========================================
		// Uniform 접근자
		// ========================================
		SceneUniform& getSceneUniform() { return sceneUniform_; }
		OptionsUniform& getOptionsUniform() { return optionsUniform_; }
		BoneDataUniform& getBoneDataUniform() { return boneDataUniform_; }

		// ✅ Uniform Buffer 접근자 (Descriptor Set 바인딩용)
		RHIBuffer* getSceneUniformBuffer(uint32_t frameIndex) const 
		{ 
			return frameIndex < sceneUniformBuffers_.size() ? sceneUniformBuffers_[frameIndex] : nullptr; 
		}
		RHIBuffer* getOptionsUniformBuffer(uint32_t frameIndex) const 
		{ 
			return frameIndex < optionsUniformBuffers_.size() ? optionsUniformBuffers_[frameIndex] : nullptr; 
		}
		RHIBuffer* getBoneDataUniformBuffer(uint32_t frameIndex) const 
		{ 
			return frameIndex < boneDataUniformBuffers_.size() ? boneDataUniformBuffers_[frameIndex] : nullptr; 
		}

		// ========================================
		// Forward Rendering 헬퍼 (ForwardPass에서 사용)
		// ========================================
		void renderForwardModels(RHI* rhi, RHIScene& scene, RHIPipeline* pipeline, uint32_t frameIndex);

		// ========================================
		// Getters
		// ========================================
		uint32_t getWidth() const { return width_; }
		uint32_t getHeight() const { return height_; }

	private:
		// ========================================
		// 초기화 헬퍼
		// ========================================
		void createPipelines(RHIFormat colorFormat, RHIFormat depthFormat);
		void createRenderTargets(uint32_t width, uint32_t height);
		void createUniformBuffers();
		void createDescriptorSets();

		// ========================================
		// 렌더링 헬퍼
		// ========================================
		void renderForward(RHICommandBuffer* cmd, const std::vector<RHIModel*>& models, uint32_t frameIndex);
		void renderShadowMap(RHICommandBuffer* cmd, const std::vector<RHIModel*>& models, uint32_t frameIndex);
		void updateMaterialDescriptorSets(const std::vector<RHIModel*>& models);

		// ========================================
		// 멤버 변수
		// ========================================
		RHI* rhi_;
		uint32_t maxFramesInFlight_;

		uint32_t width_ = 0;
		uint32_t height_ = 0;
		RHIFormat colorFormat_ = RHI_FORMAT_UNDEFINED;
		RHIFormat depthFormat_ = RHI_FORMAT_UNDEFINED;

		// Uniform 데이터
		SceneUniform sceneUniform_;
		OptionsUniform optionsUniform_;
		BoneDataUniform boneDataUniform_;

		// Uniform Buffers (per-frame)
		std::vector<RHIBuffer*> sceneUniformBuffers_;      // [maxFramesInFlight]
		std::vector<RHIBuffer*> optionsUniformBuffers_;    // [maxFramesInFlight]
		std::vector<RHIBuffer*> boneDataUniformBuffers_;   // [maxFramesInFlight]

		// Render Targets
		RHIImage* depthStencilTexture_ = nullptr;
		RHIImage* shadowMapTexture_ = nullptr;

		// Pipelines
		std::unordered_map<std::string, RHIPipeline*> pipelines_;

		// Descriptor Sets
		std::unordered_map<std::string, std::vector<RHIDescriptorSet*>> descriptorSets_;

		// Frustum Culling
		bool frustumCullingEnabled_ = true;
		CullingStats cullingStats_;
		// ViewFrustum 클래스는 나중에 추가
	};

} // namespace BinRenderer
