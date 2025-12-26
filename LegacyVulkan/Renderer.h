#pragma once

#include "Camera.h"
#include "DescriptorSet.h"
#include "Context.h"
#include "Image2D.h"
#include "StorageBuffer.h"
#include "Sampler.h"
#include "Pipeline.h"
#include "ViewFrustum.h"
#include "Model.h"
#include "MappedBuffer.h"
#include "RenderGraph.h"
#include "ShaderManager.h"
#include "TextureManager.h"
#include "ResourceRegistry.h" // 🆕 Add ResourceRegistry
#include <glm/glm.hpp>
#include <vector>
#include <functional>
#include <memory>
#include <unordered_set>


namespace BinRenderer::Vulkan {

	using namespace std;

	// Forward declaration
	class VulkanResourceManager;


	struct SceneUniform // Layout matches pbrForward.vert
	{
		alignas(16) glm::mat4 projection = glm::mat4(1.0f); // 64 bytes
		alignas(16) glm::mat4 view = glm::mat4(1.0f);       // 64 bytes
		alignas(16) glm::vec3 cameraPos = glm::vec3(0.0f);  // 16 bytes (vec3 + padding)
		alignas(4) float padding1 = 0.0f;                   // 4 bytes padding
		alignas(16) glm::vec3 directionalLightDir = glm::vec3(0.0f, 1.0f, 0.0f); // 16 bytes
		alignas(16) glm::vec3 directionalLightColor = glm::vec3(1.0f);
		alignas(16) glm::mat4 lightSpaceMatrix = glm::mat4(1.0f); // 64 bytes - for shadow mapping
	};

	struct SkyOptionsUBO
	{
		float environmentIntensity = 1.0f;
		float roughnessLevel = 0.5f;
		uint32_t useIrradianceMap = 0;

		uint32_t showMipLevels = 0;
		uint32_t showCubeFaces = 0;
		float padding1;
		float padding2;
		float padding3;
	};

	struct OptionsUniform
	{
		alignas(4) int textureOn = 1;        // Use int instead of bool, 1 = true, 0 = false
		alignas(4) int shadowOn = 1;         // Use int instead of bool, 1 = true, 0 = false
		alignas(4) int discardOn = 1;        // Use int instead of bool, 1 = true, 0 = false
		alignas(4) int animationOn = 1;      // Use int instead of bool, 1 = true, 0 = false
		alignas(4) float specularWeight = 0.05f;  // PBR specular weight - reduced default
		alignas(4) float diffuseWeight = 1.0f;    // PBR diffuse weight
		alignas(4) float emissiveWeight = 1.0f;   // PBR emissive weight
		alignas(4) float shadowOffset = 0.0f;     // Shadow bias offset
		alignas(4) int isInstanced = 0;    // GPU Instancing flag
		alignas(4) float padding1 = 0.0f;     // Padding for alignment
		alignas(4) float padding2 = 0.0f;
		alignas(4) float padding3 = 0.0f;
	};

	// Post-processing options uniform buffer structure
	struct PostOptionsUBO
	{
		// Basic tone mapping
		alignas(4) int toneMappingType = 2; // 0=None, 1=Reinhard, 2=ACES, etc.
		alignas(4) float exposure = 1.0f;   // Exposure adjustment
		alignas(4) float gamma = 2.2f;      // Gamma correction
		alignas(4) float maxWhite = 11.2f;  // For Reinhard extended

		// Color grading
		alignas(4) float contrast = 1.0f;   // Contrast adjustment
		alignas(4) float brightness = 0.0f; // Brightness adjustment
		alignas(4) float saturation = 1.0f; // Saturation adjustment
		alignas(4) float vibrance = 0.0f;   // Vibrance adjustment

		// Effects
		alignas(4) float vignetteStrength = 0.0f;    // Vignette effect strength
		alignas(4) float vignetteRadius = 0.8f;      // Vignette radius
		alignas(4) float filmGrainStrength = 0.0f;   // Film grain effect
		alignas(4) float chromaticAberration = 0.0f; // Chromatic aberration

		// Debug options
		alignas(4) int debugMode = 0;       // Debug visualization mode
		alignas(4) int showOnlyChannel = 0; // Show specific color channel
		alignas(4) float debugSplit = 0.5f; // Split screen position for comparison
		alignas(4) float padding1 = 0.0f;   // Alignment padding
	};

	struct SsaoOptionsUBO
	{
		alignas(4) float ssaoRadius = 0.1f;
		alignas(4) float ssaoBias = 0.025f;
		alignas(4) int ssaoSampleCount = 16;
		alignas(4) float ssaoPower = 2.0f;
	};

	struct BoneDataUniform
	{
		alignas(16) glm::mat4 boneMatrices[65]; // 4,160 bytes (already 16-byte aligned)
		alignas(16) glm::vec4 animationData;    // x = hasAnimation (0.0/1.0), y,z,w = future use
	};

	static_assert(sizeof(BoneDataUniform) % 16 == 0, "BoneDataUniform must be 16-byte aligned");
	static_assert(sizeof(BoneDataUniform) == 65 * 64 + 16, "Unexpected BoneDataUniform size");

	// Push constants structure for PBR forward rendering
	struct PbrPushConstants
	{
		alignas(16) glm::mat4 model = glm::mat4(1.0f); // 64 bytes
		alignas(4) uint32_t materialIndex = 0;         // 4 bytes - Material index for bindless access
		alignas(4) float coeffs[15] = {
			0.0f }; // 60 bytes (reduced from 16 to make room for materialIndex)
	};

	static_assert(sizeof(PbrPushConstants) == 128, "PbrPushConstants must be 128 bytes");

	struct CullingStats
	{
		uint32_t totalMeshes = 0;
		uint32_t culledMeshes = 0;
		uint32_t renderedMeshes = 0;
	};

	class Renderer
	{
	public:
		Renderer(Context& ctx, ShaderManager& shaderManager, const uint32_t& kMaxFramesInFlight,
			const string& kAssetsPathPrefix, const string& kShaderPathPrefix_,
			vector<unique_ptr<Model>>& models, VkFormat outColorFormat, VkFormat depthFormat,
			uint32_t swapChainWidth, uint32_t swapChainHeight,
			VulkanResourceManager* resourceManager = nullptr); //  VulkanResourceManager 추가

		~Renderer() = default;

		void createPipelines(const VkFormat colorFormat, const VkFormat depthFormat);
		void createTextures(uint32_t swapchainWidth, uint32_t swapchainHeight);
		void createUniformBuffers();

		// ========================================
		// Legacy API (unique_ptr 기반)
		// ========================================
		void update(Camera& camera, vector<unique_ptr<Model>>& models, uint32_t currentFrame,
			double time);
		void updateBoneData(const vector<unique_ptr<Model>>& models, uint32_t currentFrame);
		void draw(VkCommandBuffer cmd, uint32_t currentFrame, VkImageView swapchainImageView,
			vector<unique_ptr<Model>>& models, VkViewport viewport, VkRect2D scissor);
		void performFrustumCulling(vector<unique_ptr<Model>>& models);
		void updateWorldBounds(vector<unique_ptr<Model>>& models);

		// ========================================
		//  NEW API (Model* 기반 - Scene 호환)
		// ========================================
		void update(Camera& camera, vector<Model*>& models, uint32_t currentFrame, double time);
		void updateBoneData(const vector<Model*>& models, uint32_t currentFrame);
		void draw(VkCommandBuffer cmd, uint32_t currentFrame, VkImageView swapchainImageView,
			vector<Model*>& models, VkViewport viewport, VkRect2D scissor);
		void performFrustumCulling(vector<Model*>& models);
		void updateWorldBounds(vector<Model*>& models);



		// View frustum culling
		auto getCullingStats() const -> const CullingStats&;
		bool isFrustumCullingEnabled() const;
		void setFrustumCullingEnabled(bool enabled);
		void updateViewFrustum(const glm::mat4& viewProjection);

		// ========================================
		//  NEW: Material 동적 업데이트
		// ========================================
		void updateMaterials(const vector<Model*>& models);
		void updateMaterials(const vector<unique_ptr<Model>>& models);

	private:
		void updateMaterialDescriptorSets(); //  Helper method

	public:
		// Accessors
		Context& getContext()
		{
			return ctx_;
		}

		auto& sceneUBO() { return sceneUBO_; }
		auto& optionsUBO() { return optionsUBO_; }
		auto& skyOptionsUBO() { return skyOptionsUBO_; }
		auto& postOptionsUBO() { return postOptionsUBO_; }
		auto& ssaoOptionsUBO() { return ssaoOptionsUBO_; }

	private:
		const uint32_t& kMaxFramesInFlight_;
		const string& kAssetsPathPrefix_;
		const string& kShaderPathPrefix_;

		Context& ctx_;
		ShaderManager& shaderManager_;
		VulkanResourceManager* resourceManager_ = nullptr;  //  VulkanResourceManager 포인터 추가

		// 🆕 Resource Registry for handle-based resource management
		ResourceRegistry* resourceRegistry_ = nullptr;  //  포인터로 변경 (VulkanResourceManager에서 가져옴)

		// 🆕 Named handles for quick access
		struct ResourceHandles {
			// Textures
			ImageHandle depthStencil;
			ImageHandle floatColor1;
			ImageHandle floatColor2;
			ImageHandle shadowMap;
			ImageHandle prefilteredMap;
			ImageHandle irradianceMap;
			ImageHandle brdfLut;

			// G-Buffer
			ImageHandle gAlbedo;
			ImageHandle gNormal;
			ImageHandle gPosition;
			ImageHandle gMaterial;

			// Per-frame buffers (indexed by frame)
			vector<BufferHandle> sceneData;      // [kMaxFramesInFlight]
			vector<BufferHandle> skyOptions;     // [kMaxFramesInFlight]
			vector<BufferHandle> options;        // [kMaxFramesInFlight]
			vector<BufferHandle> boneData;       // [kMaxFramesInFlight]
			vector<BufferHandle> postOptions;    // [kMaxFramesInFlight]
			vector<BufferHandle> ssaoOptions;    // [kMaxFramesInFlight]
		};

		ResourceHandles resourceHandles_;

		// Per frame uniform buffers
		SceneUniform sceneUBO_{};
		SkyOptionsUBO skyOptionsUBO_{};
		OptionsUniform optionsUBO_{};
		BoneDataUniform boneDataUBO_{};
		PostOptionsUBO postOptionsUBO_{};
		SsaoOptionsUBO ssaoOptionsUBO_{};

		// ❌ REMOVED: OLD SYSTEM - Fully replaced by ResourceRegistry
		// unordered_map<string, vector<unique_ptr<MappedBuffer>>> perFrameUniformBuffers_;
		// unordered_map<string, unique_ptr<Image2D>> imageBuffers_;

		unique_ptr<TextureManager> materialTextures_; // Material textures for bindless rendering
		unique_ptr<StorageBuffer> materialBuffer_;    // Material data storage buffer

		Sampler samplerLinearRepeat_;
		Sampler samplerLinearClamp_;
		Sampler samplerAnisoRepeat_;
		Sampler samplerAnisoClamp_;
		Sampler samplerShadow_;

		unordered_map<string, DescriptorSet> descriptorSets_;
		// Keys: "material", "sky", "post", "shadowMap"

		unordered_map<string, vector<DescriptorSet>> perFrameDescriptorSets_;
		// Keys: "sceneOptions", "skyOptions", "postProcessing", "ssao"

		unordered_map<string, unique_ptr<Pipeline>> pipelines_;

		bool perFrameResources(vector<string> resourceNames)
		{
			// Check if any resource is a per-frame resource
			const static unordered_set<string> perFrameResourceSet = {
				"sceneData", "options", "skyOptions", "postOptions", "ssaoOptions", "boneData"
			};

			for (const auto& name : resourceNames) {
				if (perFrameResourceSet.contains(name)) {
					return true;
				}
			}

			return false;
		}

		void addResource(string resourceName, uint32_t frameNumber,
			vector<reference_wrapper<Resource>>& resources)
		{
			//  Null check 추가
			if (!resourceRegistry_) {
				printLog("❌ ERROR: ResourceRegistry not available!");
				return;
			}

			// 🆕 NEW SYSTEM: Get from ResourceRegistry
			{
				// Check if it's a per-frame buffer
				if (resourceName == "sceneData" && frameNumber != uint32_t(-1)) {
					if (auto* res = resourceRegistry_->getResourceAs<MappedBuffer>(resourceHandles_.sceneData[frameNumber])) {  //  -> 사용
						resources.push_back(*res);
						return;
					}
				}
				else if (resourceName == "options" && frameNumber != uint32_t(-1)) {
					if (auto* res = resourceRegistry_->getResourceAs<MappedBuffer>(resourceHandles_.options[frameNumber])) {  //  -> 사용
						resources.push_back(*res);
						return;
					}
				}
				else if (resourceName == "skyOptions" && frameNumber != uint32_t(-1)) {
					if (auto* res = resourceRegistry_->getResourceAs<MappedBuffer>(resourceHandles_.skyOptions[frameNumber])) {  //  -> 사용
						resources.push_back(*res);
						return;
					}
				}
				else if (resourceName == "postOptions" && frameNumber != uint32_t(-1)) {
					if (auto* res = resourceRegistry_->getResourceAs<MappedBuffer>(resourceHandles_.postOptions[frameNumber])) {  //  -> 사용
						resources.push_back(*res);
						return;
					}
				}
				else if (resourceName == "ssaoOptions" && frameNumber != uint32_t(-1)) {
					if (auto* res = resourceRegistry_->getResourceAs<MappedBuffer>(resourceHandles_.ssaoOptions[frameNumber])) {  //  -> 사용
						resources.push_back(*res);
						return;
					}
				}
				else if (resourceName == "boneData" && frameNumber != uint32_t(-1)) {
					if (auto* res = resourceRegistry_->getResourceAs<MappedBuffer>(resourceHandles_.boneData[frameNumber])) {  //  -> 사용
						resources.push_back(*res);
						return;
					}
				}

				// Check if it's an image resource
				ImageHandle imgHandle = getImageHandleByName(resourceName);
				if (imgHandle.isValid()) {
					if (auto* res = resourceRegistry_->getResourceAs<Image2D>(imgHandle)) {  //  -> 사용
						resources.push_back(*res);
						return;
					}
				}
			}

			// Special cases: materialBuffer and materialTextures (not in Registry)
			if (resourceName == "materialBuffer") {
				resources.push_back(*materialBuffer_);
				return;
			}

			if (resourceName == "materialTextures") {
				resources.push_back(*materialTextures_);
				return;
			}

			// ⚠️ Resource not found - this should not happen!
			printLog("ERROR: Resource '{}' not found in ResourceRegistry!", resourceName);
		}

		// 🆕 Helper: Convert RenderGraph string name to ImageHandle
		ImageHandle getImageHandleByName(const string& name) const
		{
			if (name == "floatColor1") return resourceHandles_.floatColor1;
			if (name == "floatColor2") return resourceHandles_.floatColor2;
			if (name == "depthStencil") return resourceHandles_.depthStencil;
			if (name == "shadowMap") return resourceHandles_.shadowMap;
			if (name == "gAlbedo") return resourceHandles_.gAlbedo;
			if (name == "gNormal") return resourceHandles_.gNormal;
			if (name == "gPosition") return resourceHandles_.gPosition;
			if (name == "gMaterial") return resourceHandles_.gMaterial;
			if (name == "prefilteredMap") return resourceHandles_.prefilteredMap;
			if (name == "irradianceMap") return resourceHandles_.irradianceMap;
			if (name == "brdfLut") return resourceHandles_.brdfLut;

			// Return invalid handle if not found
			return ImageHandle{};
		}

		RenderGraph renderGraph_;

		ViewFrustum viewFrustum_{};
		bool frustumCullingEnabled_{ true };

		// Statistics
		CullingStats cullingStats_;

		// HDR format optimization
		VkFormat selectedHDRFormat_{ VK_FORMAT_R16G16B16A16_SFLOAT }; // Store selected HDR format

		// Format selection functions with proper priority
		VkFormat selectOptimalHDRFormat(bool needsAlpha, bool fullPrecision);

		// Format validation and creation
		bool isFormatSuitableForHDR(VkFormat format);

		// Utility functions
		void logHDRMemoryUsage(uint32_t width, uint32_t height);

		// Helper functions for creating rendering structures
		VkRenderingAttachmentInfo
			createColorAttachment(VkImageView imageView,
				VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				VkClearColorValue clearColor = { 0.0f,0.0f,0.0f,0.0f },
				VkImageView resolveImageView = VK_NULL_HANDLE,
				VkResolveModeFlagBits resolveMode = VK_RESOLVE_MODE_NONE) const;

		VkRenderingAttachmentInfo
			createDepthAttachment(VkImageView imageView,
				VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				float clearDepth = 1.0f, VkImageView resolveImageView = VK_NULL_HANDLE,
				VkResolveModeFlagBits resolveMode = VK_RESOLVE_MODE_NONE) const;

		// NEW: RenderPassManager instance
		RenderPassManager renderPassManager_;
	};

} // namespace BinRenderer::Vulkan