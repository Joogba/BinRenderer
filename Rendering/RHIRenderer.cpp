#include "RHIRenderer.h"
#include "../Core/Logger.h"
#include "../RenderPass/RHIForwardPassRG.h"

namespace BinRenderer
{
	// ========================================
	//  Material UBO Definition (matching shader)
	// ========================================
	struct MaterialUBO
	{
		glm::vec4 emissiveFactor;          // offset 0
		glm::vec4 baseColorFactor;         // offset 16
		float roughnessFactor;             // offset 32
		float transparencyFactor;          // offset 36
		float discardAlpha;                // offset 40
		float metallicFactor;              // offset 44
		int32_t baseColorTextureIndex;    // offset 48
		int32_t emissiveTextureIndex;     // offset 52
		int32_t normalTextureIndex;       // offset 56
		int32_t opacityTextureIndex;      // offset 60
		int32_t metallicRoughnessTextureIndex; // offset 64
		int32_t occlusionTextureIndex;    // offset 68
		// Total size: 72 bytes (aligned to 16 bytes = 80 bytes in std140)
	};

	RHIRenderer::RHIRenderer(RHI* rhi, uint32_t maxFramesInFlight)
		: rhi_(rhi)
		, maxFramesInFlight_(maxFramesInFlight)
	{
		sceneUniformBuffers_.resize(maxFramesInFlight);
		optionsUniformBuffers_.resize(maxFramesInFlight);
		boneDataUniformBuffers_.resize(maxFramesInFlight);
	}

	RHIRenderer::~RHIRenderer()
	{
		shutdown();
	}

	bool RHIRenderer::initialize(uint32_t width, uint32_t height, RHIFormat colorFormat, RHIFormat depthFormat)
	{
		width_ = width;
		height_ = height;
		colorFormat_ = colorFormat;
		depthFormat_ = depthFormat;

		printLog("RHIRenderer::initialize - {}x{}, color: {}, depth: {}", 
			width, height, static_cast<int>(colorFormat), static_cast<int>(depthFormat));

		try
		{
			// 1. Uniform buffers 생성
			createUniformBuffers();

			// 2. Render targets 생성
			createRenderTargets(width, height);

			// 3. Pipelines 생성
			createPipelines(colorFormat, depthFormat);

			// 4. Descriptor sets 생성
			createDescriptorSets();

			// RenderGraph는 RHIApplication에서 관리
			// renderGraph_ = std::make_unique<RenderGraph>(rhi_);
			// setupRenderPasses();
			// renderGraph_->compile();

			printLog(" RHIRenderer initialized successfully");
			return true;
		}
		catch (const std::exception& e)
		{
			printLog("❌ ERROR: RHIRenderer initialization failed: {}", e.what());
			
			// 부분적으로 생성된 리소스 정리
			shutdown();
			
			return false;
		}
	}

	void RHIRenderer::shutdown()
	{
		printLog("RHIRenderer::shutdown - Starting cleanup");

		// RHI가 없으면 종료
		if (!rhi_)
		{
			printLog("⚠️  RHIRenderer::shutdown - RHI is null, skipping cleanup");
			return;
		}

		// GPU 작업이 완료될 때까지 대기
		try
		{
			rhi_->waitIdle();
		}
		catch (const std::exception& e)
		{
			printLog("⚠️  Warning: waitIdle failed during shutdown: {}", e.what());
		}

		// Uniform buffers 정리
		printLog("   Cleaning up uniform buffers...");
		for (auto& buffer : sceneUniformBuffers_)
		{
			if (buffer.isValid())
			{
				try { rhi_->destroyBuffer(buffer); }
				catch (...) { printLog("⚠️  Warning: Failed to destroy scene uniform buffer"); }
			}
		}
		sceneUniformBuffers_.clear();

		for (auto& buffer : optionsUniformBuffers_)
		{
			if (buffer.isValid())
			{
				try { rhi_->destroyBuffer(buffer); }
				catch (...) { printLog("⚠️  Warning: Failed to destroy options uniform buffer"); }
			}
		}
		optionsUniformBuffers_.clear();

		for (auto& buffer : boneDataUniformBuffers_)
		{
			if (buffer.isValid())
			{
				try { rhi_->destroyBuffer(buffer); }
				catch (...) { printLog("⚠️  Warning: Failed to destroy bone data uniform buffer"); }
			}
		}
		boneDataUniformBuffers_.clear();

		//  Material buffer 정리
		if (materialBuffer_.isValid())
		{
			try { rhi_->destroyBuffer(materialBuffer_); }
			catch (...) { printLog("⚠️  Warning: Failed to destroy material buffer"); }
			materialBuffer_ = {};
		}
		materialTextures_.clear();
		materialCount_ = 0;

		// Render targets 정리
		printLog("   Cleaning up render targets...");
		if (depthStencilTexture_.isValid())
		{
			try { rhi_->destroyImage(depthStencilTexture_); }
			catch (...) { printLog("⚠️  Warning: Failed to destroy depth/stencil texture"); }
			depthStencilTexture_ = {};
		}

		if (shadowMapTexture_.isValid())
		{
			try { rhi_->destroyImage(shadowMapTexture_); }
			catch (...) { printLog("⚠️  Warning: Failed to destroy shadow map texture"); }
			shadowMapTexture_ = {};
		}

		// Pipelines 정리
		printLog("   Cleaning up pipelines...");
		for (auto& [name, pipeline] : pipelines_)
		{
			if (pipeline.isValid())
			{
				try { rhi_->destroyPipeline(pipeline); }
				catch (...) { printLog("⚠️  Warning: Failed to destroy pipeline: {}", name); }
			}
		}
		pipelines_.clear();

		printLog(" RHIRenderer shutdown complete");
	}

	void RHIRenderer::resize(uint32_t width, uint32_t height)
	{
		width_ = width;
		height_ = height;

		printLog("RHIRenderer::resize - {}x{}", width, height);

		// Render targets 재생성
		if (depthStencilTexture_.isValid()) rhi_->destroyImage(depthStencilTexture_);
		if (shadowMapTexture_.isValid()) rhi_->destroyImage(shadowMapTexture_);

		createRenderTargets(width, height);

		// RenderGraph 업데이트
		// TODO: RenderGraph 리소스 재생성
	}

	// ========================================
	// 프레임 렌더링
	// ========================================

	void RHIRenderer::beginFrame(uint32_t frameIndex)
	{
		// 프레임 시작 준비
		cullingStats_ = {}; // Reset culling stats
	}

	void RHIRenderer::updateUniforms(const RHICamera& camera, RHIScene& scene, uint32_t frameIndex, double time)
	{
		// Scene uniform 업데이트
		sceneUniform_.projection = camera.getProjectionMatrix();
		sceneUniform_.view = camera.getViewMatrix();
		sceneUniform_.cameraPos = camera.getPosition();

		// TODO: Directional light 업데이트
		// sceneUniform_.directionalLightDir = scene.getDirectionalLight().direction;
		// sceneUniform_.directionalLightColor = scene.getDirectionalLight().color;

		// Uniform buffer 업데이트
		if (sceneUniformBuffers_[frameIndex].isValid())
		{
			void* data = rhi_->mapBuffer(sceneUniformBuffers_[frameIndex]);
			memcpy(data, &sceneUniform_, sizeof(SceneUniform));
			rhi_->unmapBuffer(sceneUniformBuffers_[frameIndex]);
		}

		// Options uniform 업데이트
		if (optionsUniformBuffers_[frameIndex].isValid())
		{
			void* data = rhi_->mapBuffer(optionsUniformBuffers_[frameIndex]);
			memcpy(data, &optionsUniform_, sizeof(OptionsUniform));
			rhi_->unmapBuffer(optionsUniformBuffers_[frameIndex]);
		}
	}

	void RHIRenderer::updateBoneData(const std::vector<RHIModel*>& models, uint32_t frameIndex)
	{
		// TODO: 애니메이션 bone matrices 업데이트
		// 현재는 identity matrices

		if (boneDataUniformBuffers_[frameIndex].isValid())
		{
			void* data = rhi_->mapBuffer(boneDataUniformBuffers_[frameIndex]);
			memcpy(data, &boneDataUniform_, sizeof(BoneDataUniform));
			rhi_->unmapBuffer(boneDataUniformBuffers_[frameIndex]);
		}
	}

	void RHIRenderer::render(RHICommandBuffer* cmd, RHIScene& scene, uint32_t frameIndex, RHIImageView* swapchainImageView)
	{
		//  RHIScene에서 모델 가져오기
		auto visibleModels = scene.getModels();
		
		// Frustum culling
		if (frustumCullingEnabled_)
		{
			performFrustumCulling(visibleModels);
		}

		// Shadow map pass (optional)
		// renderShadowMap(cmd, visibleModels, frameIndex);

		// Forward rendering pass
		renderForward(cmd, visibleModels, frameIndex);
	}

	void RHIRenderer::endFrame(uint32_t frameIndex)
	{
		// 프레임 종료 처리
	}

	// ========================================
	// Frustum Culling
	// ========================================

	void RHIRenderer::performFrustumCulling(std::vector<RHIModel*>& models)
	{
		if (!frustumCullingEnabled_)
		{
			return;
		}

		// TODO: ViewFrustum 클래스 구현 필요
		// 현재는 모든 모델 렌더링

		cullingStats_.totalMeshes = static_cast<uint32_t>(models.size());
		cullingStats_.renderedMeshes = static_cast<uint32_t>(models.size());
		cullingStats_.culledMeshes = 0;
	}

	void RHIRenderer::updateViewFrustum(const glm::mat4& viewProjection)
	{
		// TODO: ViewFrustum 업데이트
	}

	// ========================================
	// 초기화 헬퍼
	// ========================================

	void RHIRenderer::createUniformBuffers()
	{
		printLog("Creating uniform buffers (maxFramesInFlight: {})...", maxFramesInFlight_);

		for (uint32_t i = 0; i < maxFramesInFlight_; i++)
		{
			// Scene uniform buffer
			RHIBufferCreateInfo sceneBufferInfo{};
			sceneBufferInfo.size = sizeof(SceneUniform);
			sceneBufferInfo.usage = RHI_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			sceneBufferInfo.memoryProperties = RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			
			sceneUniformBuffers_[i] = rhi_->createBuffer(sceneBufferInfo);
			if (!sceneUniformBuffers_[i].isValid())
			{
				printLog("❌ ERROR: Failed to create scene uniform buffer {}", i);
				throw std::runtime_error("Failed to create scene uniform buffer");
			}

			// Options uniform buffer
			RHIBufferCreateInfo optionsBufferInfo{};
			optionsBufferInfo.size = sizeof(OptionsUniform);
			optionsBufferInfo.usage = RHI_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			optionsBufferInfo.memoryProperties = RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			
			optionsUniformBuffers_[i] = rhi_->createBuffer(optionsBufferInfo);
			if (!optionsUniformBuffers_[i].isValid())
			{
				printLog("❌ ERROR: Failed to create options uniform buffer {}", i);
				throw std::runtime_error("Failed to create options uniform buffer");
			}

			// Bone data uniform buffer
			RHIBufferCreateInfo boneBufferInfo{};
			boneBufferInfo.size = sizeof(BoneDataUniform);
			boneBufferInfo.usage = RHI_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			boneBufferInfo.memoryProperties = RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			
			boneDataUniformBuffers_[i] = rhi_->createBuffer(boneBufferInfo);
			if (!boneDataUniformBuffers_[i].isValid())
			{
				printLog("❌ ERROR: Failed to create bone data uniform buffer {}", i);
				throw std::runtime_error("Failed to create bone data uniform buffer");
			}

			printLog("    Frame {} uniform buffers created", i);
		}

		printLog(" All uniform buffers created successfully");
	}

	void RHIRenderer::createRenderTargets(uint32_t width, uint32_t height)
	{
		printLog("Creating render targets ({}x{})...", width, height);

		// Depth/Stencil image
		RHIImageCreateInfo depthInfo{};
		depthInfo.width = width;
		depthInfo.height = height;
		depthInfo.format = depthFormat_;
		depthInfo.usage = RHI_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | RHI_IMAGE_USAGE_SAMPLED_BIT;
		depthInfo.samples = RHI_SAMPLE_COUNT_1_BIT;

		depthStencilTexture_ = rhi_->createImage(depthInfo);
		if (!depthStencilTexture_.isValid())
		{
			printLog("❌ ERROR: Failed to create depth/stencil texture");
			throw std::runtime_error("Failed to create depth/stencil texture");
		}
		printLog("    Depth/stencil texture created");

		// Shadow map image (2048x2048)
		RHIImageCreateInfo shadowInfo{};
	shadowInfo.width = 2048;
	shadowInfo.height = 2048;
	shadowInfo.format = RHI_FORMAT_D32_SFLOAT;
	shadowInfo.usage = RHI_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | RHI_IMAGE_USAGE_SAMPLED_BIT;
	shadowInfo.samples = RHI_SAMPLE_COUNT_1_BIT;

		shadowMapTexture_ = rhi_->createImage(shadowInfo);
		if (!shadowMapTexture_.isValid())
		{
			printLog("❌ ERROR: Failed to create shadow map texture");
			throw std::runtime_error("Failed to create shadow map texture");
		}
		printLog("    Shadow map texture created");

		printLog(" Render targets created successfully: {}x{}", width, height);
	}

	void RHIRenderer::createPipelines(RHIFormat colorFormat, RHIFormat depthFormat)
	{
		// TODO: Pipeline 생성 (셰이더, 상태 등)
		// 현재는 플레이스홀더

		printLog(" Pipelines created");
	}

	void RHIRenderer::createDescriptorSets()
	{
		// TODO: Descriptor sets 생성
		// 현재는 플레이스홀더

		printLog(" Descriptor sets created");
	}

	// ========================================
	// 렌더링 헬퍼
	// ========================================

	void RHIRenderer::renderForward(RHICommandBuffer* cmd, const std::vector<RHIModel*>& models, uint32_t frameIndex)
	{
		// TODO: Forward rendering 구현
		// 1. Pipeline 바인딩
		// 2. Descriptor sets 바인딩
		// 3. Draw calls
	}

	void RHIRenderer::renderShadowMap(RHICommandBuffer* cmd, const std::vector<RHIModel*>& models, uint32_t frameIndex)
	{
		// TODO: Shadow map rendering 구현
	}

	void RHIRenderer::updateMaterialDescriptorSets(const std::vector<RHIModel*>& models)
	{
		// TODO: Material descriptor sets 업데이트
	}

	void RHIRenderer::renderForwardModels(RHI* rhi, RHIScene& scene, RHIPipelineHandle pipeline, uint32_t frameIndex)
	{
		if (!pipeline.isValid())
		{
			printLog("❌ ERROR: Pipeline is null in renderForwardModels");
			return;
		}

		auto models = scene.getModels();
		
		if (frameIndex % 60 == 0)
		{
			printLog("[RHIRenderer] Rendering {} models", models.size());
		}

		// 모델 렌더링
		for (auto* model : models)
		{
			if (!model)
				continue;

			// ❌ Simple 셰이더는 Push constants 불필요 - 주석 처리
			// TODO: PBR 셰이더로 전환 시 다시 활성화
			/*
			//  Push constants (Model matrix)
			PbrPushConstants pushConstants{};
			pushConstants.model = model->getTransform();
			pushConstants.materialIndex = 0;  // TODO: Material index 전달
			
			//  Push constants 전송 (Pipeline 사용)
			rhi->cmdPushConstants(
				pipeline, 
				RHI_SHADER_STAGE_VERTEX_BIT | RHI_SHADER_STAGE_FRAGMENT_BIT,
				0, 
				sizeof(PbrPushConstants), 
				&pushConstants
			);
			*/

			//  Draw
			model->draw(rhi, 1);
			
			if (frameIndex % 60 == 0)
			{
				const auto& meshes = model->getMeshes();
				printLog("[RHIRenderer]   - Drew model '{}': {} meshes", 
					model->getName().empty() ? "<unnamed>" : model->getName(), 
					meshes.size());
			}
		}
	}

	// ========================================
	//  Material System
	// ========================================

	void RHIRenderer::buildMaterialBuffer(RHIScene& scene)
	{
		printLog("[RHIRenderer] Building material buffer from scene...");

		// 기존 material buffer 정리
		if (materialBuffer_.isValid())
		{
			rhi_->destroyBuffer(materialBuffer_);
			materialBuffer_ = {};
		}
		materialTextures_.clear();
		materialCount_ = 0;

		// Scene에서 모든 모델의 materials 수집
		std::vector<MaterialUBO> materials;
		auto models = scene.getModels();

		for (auto* model : models)
		{
			if (!model) continue;

			const auto& modelMaterials = model->getMaterials();
			for (const auto& mat : modelMaterials)
			{
				MaterialUBO materialUBO{};
				const auto& data = mat.getData();

				// Material 데이터 복사
				materialUBO.emissiveFactor = data.emissiveFactor;
				materialUBO.baseColorFactor = data.baseColorFactor;
				materialUBO.roughnessFactor = data.roughness;
				materialUBO.transparencyFactor = data.transparency;
				materialUBO.discardAlpha = data.discardAlpha;
				materialUBO.metallicFactor = data.metallic;

				// Texture indices
				materialUBO.baseColorTextureIndex = data.baseColorTextureIndex;
				materialUBO.emissiveTextureIndex = data.emissiveTextureIndex;
				materialUBO.normalTextureIndex = data.normalTextureIndex;
				materialUBO.opacityTextureIndex = data.opacityTextureIndex;
				materialUBO.metallicRoughnessTextureIndex = data.metallicRoughnessTextureIndex;
				materialUBO.occlusionTextureIndex = data.occlusionTextureIndex;

				materials.push_back(materialUBO);
			}
		}

		if (materials.empty())
		{
			printLog("[RHIRenderer]   ⚠️  No materials found in scene - using dummy");
			
			// 기본 material 하나 추가
			MaterialUBO defaultMat{};
			defaultMat.baseColorFactor = glm::vec4(1.0f); // 흰색
			defaultMat.roughnessFactor = 1.0f;
			defaultMat.metallicFactor = 0.0f;
			materials.push_back(defaultMat);
		}

		materialCount_ = static_cast<uint32_t>(materials.size());

		// GPU 버퍼 생성
		RHIBufferCreateInfo bufferInfo{};
		bufferInfo.size = sizeof(MaterialUBO) * materials.size();
		bufferInfo.usage = RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		bufferInfo.memoryProperties = RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		materialBuffer_ = rhi_->createBuffer(bufferInfo);
		if (!materialBuffer_.isValid())
		{
			printLog("[RHIRenderer] ❌ Failed to create material buffer!");
			return;
		}

		// 데이터 업로드
		void* data = rhi_->mapBuffer(materialBuffer_);
		memcpy(data, materials.data(), bufferInfo.size);
		rhi_->unmapBuffer(materialBuffer_);

		printLog("[RHIRenderer]    Material buffer created: {} materials, {} bytes", 
			materialCount_, bufferInfo.size);

		// TODO: Texture 수집 및 bindless array 구성
		// 현재는 placeholder
		printLog("[RHIRenderer]   ⏳ Material textures collection - TODO");
	}

} // namespace BinRenderer
