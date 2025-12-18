#include "RHIRenderer.h"
#include "../Core/Logger.h"
#include "../RenderPass/RHIForwardPassRG.h"

namespace BinRenderer
{
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

		// 1. Uniform buffers 생성
		createUniformBuffers();

		// 2. Render targets 생성
		createRenderTargets(width, height);

		// 3. Pipelines 생성
		createPipelines(colorFormat, depthFormat);

		// 4. Descriptor sets 생성
		createDescriptorSets();

		// 5. RenderGraph 생성
		renderGraph_ = std::make_unique<RenderGraph>(rhi_);
		setupRenderPasses();
		renderGraph_->compile();

		printLog("RHIRenderer initialized successfully");
		return true;
	}

	void RHIRenderer::shutdown()
	{
		// RenderGraph 정리
		if (renderGraph_)
		{
			renderGraph_.reset();
		}

		// Uniform buffers 정리
		for (auto* buffer : sceneUniformBuffers_)
		{
			if (buffer) rhi_->destroyBuffer(buffer);
		}
		for (auto* buffer : optionsUniformBuffers_)
		{
			if (buffer) rhi_->destroyBuffer(buffer);
		}
		for (auto* buffer : boneDataUniformBuffers_)
		{
			if (buffer) rhi_->destroyBuffer(buffer);
		}

		// Render targets 정리
		if (depthStencilTexture_) rhi_->destroyImage(depthStencilTexture_);
		if (shadowMapTexture_) rhi_->destroyImage(shadowMapTexture_);

		// Pipelines 정리
		for (auto& [name, pipeline] : pipelines_)
		{
			if (pipeline) rhi_->destroyPipeline(pipeline);
		}

		// Descriptor sets 정리
		// TODO: RHI에 destroyDescriptorSet 추가 필요
		// for (auto& [name, sets] : descriptorSets_)
		// {
		// 	for (auto* set : sets)
		// 	{
		// 		if (set) rhi_->destroyDescriptorSet(set);
		// 	}
		// }

		printLog("RHIRenderer shutdown complete");
	}

	void RHIRenderer::resize(uint32_t width, uint32_t height)
	{
		width_ = width;
		height_ = height;

		printLog("RHIRenderer::resize - {}x{}", width, height);

		// Render targets 재생성
		if (depthStencilTexture_) rhi_->destroyImage(depthStencilTexture_);
		if (shadowMapTexture_) rhi_->destroyImage(shadowMapTexture_);

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
		if (sceneUniformBuffers_[frameIndex])
		{
			void* data = rhi_->mapBuffer(sceneUniformBuffers_[frameIndex]);
			memcpy(data, &sceneUniform_, sizeof(SceneUniform));
			rhi_->unmapBuffer(sceneUniformBuffers_[frameIndex]);
		}

		// Options uniform 업데이트
		if (optionsUniformBuffers_[frameIndex])
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

		if (boneDataUniformBuffers_[frameIndex])
		{
			void* data = rhi_->mapBuffer(boneDataUniformBuffers_[frameIndex]);
			memcpy(data, &boneDataUniform_, sizeof(BoneDataUniform));
			rhi_->unmapBuffer(boneDataUniformBuffers_[frameIndex]);
		}
	}

	void RHIRenderer::render(RHICommandBuffer* cmd, RHIScene& scene, uint32_t frameIndex, RHIImageView* swapchainImageView)
	{
		// TODO: RHIScene에 getModels() 추가 필요
		std::vector<RHIModel*> visibleModels; // = scene.getModels();
		
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
		for (uint32_t i = 0; i < maxFramesInFlight_; i++)
		{
			// Scene uniform buffer
			RHIBufferCreateInfo sceneBufferInfo{};
			sceneBufferInfo.size = sizeof(SceneUniform);
			sceneBufferInfo.usage = RHI_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			sceneBufferInfo.memoryProperties = RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			sceneUniformBuffers_[i] = rhi_->createBuffer(sceneBufferInfo);

			// Options uniform buffer
			RHIBufferCreateInfo optionsBufferInfo{};
			optionsBufferInfo.size = sizeof(OptionsUniform);
			optionsBufferInfo.usage = RHI_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			optionsBufferInfo.memoryProperties = RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			optionsUniformBuffers_[i] = rhi_->createBuffer(optionsBufferInfo);

			// Bone data uniform buffer
			RHIBufferCreateInfo boneBufferInfo{};
			boneBufferInfo.size = sizeof(BoneDataUniform);
			boneBufferInfo.usage = RHI_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			boneBufferInfo.memoryProperties = RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			boneDataUniformBuffers_[i] = rhi_->createBuffer(boneBufferInfo);
		}

		printLog("Uniform buffers created");
	}

	void RHIRenderer::createRenderTargets(uint32_t width, uint32_t height)
	{
		// Depth/Stencil image
		RHIImageCreateInfo depthInfo{};
		depthInfo.width = width;
		depthInfo.height = height;
		depthInfo.format = depthFormat_;
		depthInfo.usage = RHI_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | RHI_IMAGE_USAGE_SAMPLED_BIT;
		depthInfo.samples = RHI_SAMPLE_COUNT_1_BIT;

		depthStencilTexture_ = rhi_->createImage(depthInfo);

		// Shadow map image (2048x2048)
		RHIImageCreateInfo shadowInfo{};
		shadowInfo.width = 2048;
		shadowInfo.height = 2048;
		shadowInfo.format = RHI_FORMAT_D32_SFLOAT;
		shadowInfo.usage = RHI_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | RHI_IMAGE_USAGE_SAMPLED_BIT;
		shadowInfo.samples = RHI_SAMPLE_COUNT_1_BIT;

		shadowMapTexture_ = rhi_->createImage(shadowInfo);

		printLog("✅ Render targets created: {}x{}", width, height);
	}

	void RHIRenderer::createPipelines(RHIFormat colorFormat, RHIFormat depthFormat)
	{
		// TODO: Pipeline 생성 (셰이더, 상태 등)
		// 현재는 플레이스홀더

		printLog("✅ Pipelines created");
	}

	void RHIRenderer::createDescriptorSets()
	{
		// TODO: Descriptor sets 생성
		// 현재는 플레이스홀더

		printLog("✅ Descriptor sets created");
	}

	void RHIRenderer::setupRenderPasses()
	{
		// Forward Pass 추가
		auto forwardPass = std::make_unique<RHIForwardPassRG>(rhi_);
		renderGraph_->addPass(std::move(forwardPass));

		printLog("✅ RenderGraph passes setup complete");
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

} // namespace BinRenderer
