#include "Renderer.h"
#include "Logger.h"
#include "TracyProfiler.h"
#include "VulkanResourceManager.h"  //  VulkanResourceManager 헤더 추가

#include <optional>
#include <stb_image.h>

namespace BinRenderer::Vulkan {

	Renderer::Renderer(Context& ctx, ShaderManager& shaderManager, const uint32_t& kMaxFramesInFlight,
		const string& kAssetsPathPrefix, const string& kShaderPathPrefix_,
		vector<unique_ptr<Model>>& models, VkFormat outColorFormat, VkFormat depthFormat,
		uint32_t swapChainWidth, uint32_t swapChainHeight,
		VulkanResourceManager* resourceManager)  //  파라미터 추가
		: ctx_(ctx), shaderManager_(shaderManager),
		resourceManager_(resourceManager),  //  초기화
		kMaxFramesInFlight_(kMaxFramesInFlight),
		kAssetsPathPrefix_(kAssetsPathPrefix), kShaderPathPrefix_(kShaderPathPrefix_),
		samplerShadow_(ctx), samplerLinearRepeat_(ctx), samplerLinearClamp_(ctx),
		samplerAnisoRepeat_(ctx), samplerAnisoClamp_(ctx),
		materialTextures_(std::make_unique<TextureManager>(ctx))
	{
		TRACY_CPU_SCOPE("Renderer::Constructor");

		//  ResourceRegistry 설정
		if (resourceManager_) {
			resourceRegistry_ = &resourceManager_->GetGpuResources();
			printLog(" Renderer using VulkanResourceManager's ResourceRegistry");
		}
		else {
			printLog("⚠️ Renderer created without VulkanResourceManager");
		}

		{
			TRACY_CPU_SCOPE("Create Pipelines");
			createPipelines(outColorFormat, depthFormat);
		}

		{
			TRACY_CPU_SCOPE("Create Textures");
			createTextures(swapChainWidth, swapChainHeight);
		}

		{
			TRACY_CPU_SCOPE("Create Uniform Buffers");
			createUniformBuffers();
		}

		{
			TRACY_CPU_SCOPE("Setup Material Buffers");
			vector<MaterialUBO> allMaterials;

			for (auto& m : models) {
				m->prepareForBindlessRendering(samplerLinearRepeat_, allMaterials, *materialTextures_);
			}

			// ========================================
			// FIX: 빈 모델일 때 더미 버퍼 생성
			// ========================================
			if (allMaterials.empty()) {
				printLog("WARNING: No models provided, creating dummy material buffer");

				// 더미 Material 추가 (최소 1개 필요)
				MaterialUBO dummyMaterial{};
				allMaterials.push_back(dummyMaterial);
			}

			materialBuffer_ = std::make_unique<StorageBuffer>(ctx_, allMaterials.data(),
				sizeof(MaterialUBO) * allMaterials.size());
		}

		{
			TRACY_CPU_SCOPE("Setup Descriptor Sets");
			// ... existing descriptor set creation code ...
			unordered_map<string, vector<string>> descriptorSetNames; // TODO: move to script
			descriptorSetNames["shadowMap"] = { "sceneOptions" };
			descriptorSetNames["pbrDeferred"] = { "sceneOptions", "material" };
			descriptorSetNames["sky"] = { "skyOptions", "sky" };
			descriptorSetNames["deferredLighting"] = { "deferredLightingData" };
			descriptorSetNames["post"] = { "postProcessing" };

			unordered_map<string, vector<vector<BindingInfo>>> bindingInfos =
				shaderManager_.bindingInfos();

			for (auto i : descriptorSetNames) {
				auto pipelineName = i.first;

				auto& bindings = bindingInfos.at(pipelineName);

				assert(bindings.size() == descriptorSetNames[pipelineName].size());

				for (int s = 0; s < bindings.size(); s++) {

					string setName = descriptorSetNames[pipelineName][s];

					if (perFrameDescriptorSets_.find(setName) != perFrameDescriptorSets_.end())
						continue;
					if (descriptorSets_.find(setName) != descriptorSets_.end())
						continue;

					vector<string> bindingNames;
					for (int b = 0; b < bindings[s].size(); b++) {
						bindingNames.push_back(bindings[s][b].resourceName);
					}

					bool perFramesSet = this->perFrameResources(bindingNames);

					if (perFramesSet) {
						perFrameDescriptorSets_[setName].resize(kMaxFramesInFlight_);
						for (uint32_t i = 0; i < kMaxFramesInFlight_; i++) {
							// Collect resources for this descriptor set
							vector<reference_wrapper<Resource>> resources;

							for (const string& resourceName : bindingNames) {
								addResource(resourceName, i, resources);
							}

							// Create the descriptor set with collected resources
							perFrameDescriptorSets_[setName][i].create(
								ctx_, pipelines_[pipelineName]->layouts()[s], resources);
						}
					}
					else {
						// Collect resources for non-per-frame descriptor set
						vector<reference_wrapper<Resource>> resources;

						for (const string& resourceName : bindingNames) {
							addResource(resourceName, uint32_t(-1), resources);
						}

						// Create the descriptor set with collected resources
						descriptorSets_[setName].create(ctx_, pipelines_[pipelineName]->layouts()[s],
							resources);
					}
				}

				// Update pipeline's descriptor sets with the created descriptor sets for this pipeline
				vector<vector<reference_wrapper<DescriptorSet>>> pipelineDescriptorSets;
				pipelineDescriptorSets.resize(kMaxFramesInFlight_);

				for (uint32_t frameIndex = 0; frameIndex < kMaxFramesInFlight_; ++frameIndex) {
					pipelineDescriptorSets[frameIndex].reserve(descriptorSetNames[pipelineName].size());

					for (size_t setIndex = 0; setIndex < descriptorSetNames[pipelineName].size();
						++setIndex) {
						const string& setName = descriptorSetNames[pipelineName][setIndex];

						// Check if this is a per-frame descriptor set
						if (perFrameDescriptorSets_.find(setName) != perFrameDescriptorSets_.end()) {
							// For per-frame sets, use the specific frame
							pipelineDescriptorSets[frameIndex].emplace_back(
								std::ref(perFrameDescriptorSets_[setName][frameIndex]));
						}
						else if (descriptorSets_.find(setName) != descriptorSets_.end()) {
							// For non-per-frame sets, use the same descriptor set for all frames
							pipelineDescriptorSets[frameIndex].emplace_back(
								std::ref(descriptorSets_[setName]));
						}
					}
				}

				// Set the descriptor sets on the pipeline
				pipelines_[pipelineName]->setDescriptorSets(pipelineDescriptorSets);
			}
		}
	}

	void Renderer::createUniformBuffers()
	{
		TRACY_CPU_SCOPE("Renderer::createUniformBuffers");

		//  Null check
		if (!resourceRegistry_) {
			printLog("❌ ERROR: ResourceRegistry not available!");
			return;
		}

		// 🆕 NEW SYSTEM: Register all uniform buffers with ResourceRegistry
		for (uint32_t i = 0; i < kMaxFramesInFlight_; ++i) {
			// Scene data
			{
				auto sceneBuffer = std::make_unique<MappedBuffer>(ctx_);
				sceneBuffer->createUniformBuffer(sceneUBO_);
				resourceHandles_.sceneData[i] = resourceRegistry_->registerBuffer(  //  -> 사용
					std::format("sceneData_{}", i),
					std::move(sceneBuffer)
				);
			}

			// Options
			{
				auto optionsBuffer = std::make_unique<MappedBuffer>(ctx_);
				optionsBuffer->createUniformBuffer(optionsUBO_);
				resourceHandles_.options[i] = resourceRegistry_->registerBuffer(  //  -> 사용
					std::format("options_{}", i),
					std::move(optionsBuffer)
				);
			}

			// Sky options
			{
				auto skyOptionsBuffer = std::make_unique<MappedBuffer>(ctx_);
				skyOptionsBuffer->createUniformBuffer(skyOptionsUBO_);
				resourceHandles_.skyOptions[i] = resourceRegistry_->registerBuffer(  //  -> 사용
					std::format("skyOptions_{}", i),
					std::move(skyOptionsBuffer)
				);
			}

			// Post options
			{
				auto postOptionsBuffer = std::make_unique<MappedBuffer>(ctx_);
				postOptionsBuffer->createUniformBuffer(postOptionsUBO_);
				resourceHandles_.postOptions[i] = resourceRegistry_->registerBuffer(  //  -> 사용
					std::format("postOptions_{}", i),
					std::move(postOptionsBuffer)
				);
			}

			// SSAO options
			{
				auto ssaoOptionsBuffer = std::make_unique<MappedBuffer>(ctx_);
				ssaoOptionsBuffer->createUniformBuffer(ssaoOptionsUBO_);
				resourceHandles_.ssaoOptions[i] = resourceRegistry_->registerBuffer(  //  -> 사용
					std::format("ssaoOptions_{}", i),
					std::move(ssaoOptionsBuffer)
				);
			}

			// Bone data
			{
				auto boneDataBuffer = std::make_unique<MappedBuffer>(ctx_);
				boneDataBuffer->createUniformBuffer(boneDataUBO_);
				resourceHandles_.boneData[i] = resourceRegistry_->registerBuffer(  //  -> 사용
					std::format("boneData_{}", i),
					std::move(boneDataBuffer)
				);
			}
		}

		printLog(" Created {} uniform buffer types × {} frames = {} total buffers",
			6, kMaxFramesInFlight_, 6 * kMaxFramesInFlight_);
	}

	void Renderer::update(Camera& camera, vector<unique_ptr<Model>>& models, uint32_t currentFrame,
		double time)
	{
		TRACY_CPU_SCOPE("Renderer::update");

		// ========================================
		//  GPU Instancing: Detect if any model uses instancing
		// ========================================
		{
			TRACY_CPU_SCOPE("Detect GPU Instancing");
			bool anyInstancedModel = false;
			for (const auto& model : models) {
				if (model && model->getInstanceCount() > 1) {
					anyInstancedModel = true;
					break;
				}
			}
			optionsUBO_.isInstanced = anyInstancedModel ? 1 : 0;
		}

		{
			TRACY_CPU_SCOPE("Update View Frustum");
			// Update view frustum based on current camera view-projection matrix
			updateViewFrustum(camera.matrices.perspective * camera.matrices.view);
		}

		{
			TRACY_CPU_SCOPE("Update World Bounds");
			updateWorldBounds(models);
		}

		{
			TRACY_CPU_SCOPE("Update Bone Data");
			updateBoneData(models, currentFrame);
		}

		{
			TRACY_CPU_SCOPE("Perform Frustum Culling");
			performFrustumCulling(models);
		}

		{
			TRACY_CPU_SCOPE("Update Uniform Buffers");

			// 🆕 NEW SYSTEM: Use ResourceRegistry with Handles
			if (auto* buffer = resourceRegistry_->getResourceAs<MappedBuffer>(resourceHandles_.sceneData[currentFrame])) {  //  -> 사용
				buffer->updateFromCpuData();
			}

			if (auto* buffer = resourceRegistry_->getResourceAs<MappedBuffer>(resourceHandles_.options[currentFrame])) {  //  -> 사용
				buffer->updateFromCpuData();
			}

			if (auto* buffer = resourceRegistry_->getResourceAs<MappedBuffer>(resourceHandles_.skyOptions[currentFrame])) {  //  -> 사용
				buffer->updateFromCpuData();
			}

			if (auto* buffer = resourceRegistry_->getResourceAs<MappedBuffer>(resourceHandles_.postOptions[currentFrame])) {  //  -> 사용
				buffer->updateFromCpuData();
			}

			if (auto* buffer = resourceRegistry_->getResourceAs<MappedBuffer>(resourceHandles_.ssaoOptions[currentFrame])) {  //  -> 사용
				buffer->updateFromCpuData();
			}

			if (auto* buffer = resourceRegistry_->getResourceAs<MappedBuffer>(resourceHandles_.boneData[currentFrame])) {  //  -> 사용
				buffer->updateFromCpuData();
			}
		}
	}

	void Renderer::updateBoneData(const vector<unique_ptr<Model>>& models, uint32_t currentFrame)
	{
		TRACY_CPU_SCOPE("Renderer::updateBoneData");

		// Reset bone data
		boneDataUBO_.animationData.x = 0.0f;
		for (int i = 0; i < 65; ++i) {
			boneDataUBO_.boneMatrices[i] = glm::mat4(1.0f);
		}

		// Check if any model has animation data
		bool hasAnyAnimation = false;
		for (const auto& model : models) {
			if (model->hasAnimations() && model->hasBones()) {
				hasAnyAnimation = true;

				// Get bone matrices from the first animated model
				const auto& boneMatrices = model->getBoneMatrices();

				// Copy bone matrices (up to 65 bones)
				const size_t maxBones = 65;
				size_t bonesToCopy = (boneMatrices.size() < maxBones) ? boneMatrices.size() : maxBones;
				for (size_t i = 0; i < bonesToCopy; ++i) {
					boneDataUBO_.boneMatrices[i] = boneMatrices[i];
				}

				break; // For now, use the first animated model
			}
		}

		boneDataUBO_.animationData.x = float(hasAnyAnimation);

		// DEBUG: Log hasAnimation state
		static bool lastHasAnimation = false;
		if (lastHasAnimation != hasAnyAnimation) {
			printLog("hasAnimation changed to: {}", hasAnyAnimation);
			lastHasAnimation = hasAnyAnimation;
		}

		// 🆕 NEW SYSTEM: Update via Handle
		if (auto* buffer = resourceRegistry_->getResourceAs<MappedBuffer>(resourceHandles_.boneData[currentFrame])) {  //  이미 -> 사용 중
			buffer->updateFromCpuData();
		}
	}

	void Renderer::draw(VkCommandBuffer cmd, uint32_t currentFrame, VkImageView swapchainImageView,
		vector<unique_ptr<Model>>& models, VkViewport viewport, VkRect2D scissor)
	{
		TRACY_CPU_SCOPE("Renderer::draw");

		// ❌ 디버그 로그 제거 (테스트용)
		// static bool firstFrame = true;
		// if (firstFrame) { ... }

		for (auto& renderNode : renderGraph_.renderNodes_) {
			if (renderNode.pipelineNames[0] == "deferredLighting") {
				TRACY_CPU_SCOPE("deferredLighting");
				pipelines_.at("deferredLighting")->dispatch(cmd, currentFrame);
				continue;
			}

			string mainTarget;
			vector<VkRenderingAttachmentInfo> colorAttachments{};
			VkRenderingAttachmentInfo depthAttachment{};
			VkRenderingInfo renderingInfo{};
			renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;

			// Handle color attachments
			{
				TRACY_CPU_SCOPE("Setup Color Attachments");
				for (const auto& colorTarget : renderNode.colorAttachments) {
					if (colorTarget == "swapchain") {
						colorAttachments.push_back(createColorAttachment(
							swapchainImageView, VK_ATTACHMENT_LOAD_OP_CLEAR, { 0.0f, 0.0f, 1.0f, 0.0f }));
						continue;
					}

					if (mainTarget.empty()) {
						mainTarget = colorTarget;
					}

					// 🆕 NEW SYSTEM: Use Handle
					ImageHandle handle = getImageHandleByName(colorTarget);
					Image2D* image = resourceRegistry_->getResourceAs<Image2D>(handle);

					if (image) {
						if (renderNode.pipelineNames[0] == "sky") {
							colorAttachments.push_back(createColorAttachment(
								image->view(), VK_ATTACHMENT_LOAD_OP_LOAD, { 0.0f, 0.0f, 0.5f, 0.0f }));
						}
						else {
							colorAttachments.push_back(createColorAttachment(
								image->view(), VK_ATTACHMENT_LOAD_OP_CLEAR,
								{ 0.0f, 0.0f, 0.5f, 0.0f }));
						}
					}
					else {
						// ⚠️ Resource not found - should not happen!
						printLog("ERROR: Color target '{}' not found in ResourceRegistry!", colorTarget);
					}
				}
			}

			// Handle depth attachment
			{
				TRACY_CPU_SCOPE("Setup Depth Attachment");
				if (!renderNode.depthAttachment.empty()) {
					if (mainTarget.empty()) {
						mainTarget = renderNode.depthAttachment;
					}

					// 🆕 NEW SYSTEM: Use Handle (now safe!)
					ImageHandle handle = getImageHandleByName(renderNode.depthAttachment);
					Image2D* image = resourceRegistry_->getResourceAs<Image2D>(handle);

					if (image) {
						image->transitionToDepthStencilAttachment(cmd);

						if (renderNode.pipelineNames[0] == "sky") {
							depthAttachment = createDepthAttachment(
								image->attachmentView(),
								VK_ATTACHMENT_LOAD_OP_LOAD, 1.0f);
						}
						else {
							depthAttachment = createDepthAttachment(
								image->attachmentView(),
								VK_ATTACHMENT_LOAD_OP_CLEAR, 1.0f);
						}

						renderingInfo.pDepthAttachment = &depthAttachment;
					}
					else {
						// ⚠️ Resource not found - should not happen!
						printLog("ERROR: Depth attachment '{}' not found in ResourceRegistry!", renderNode.depthAttachment);
					}
				}
			}

			{
				TRACY_CPU_SCOPE("Submit Pipeline Barriers");
				for (auto& pipelineName : renderNode.pipelineNames) {
					pipelines_.at(pipelineName)->submitBarriers(cmd, currentFrame);
				}
			}

			uint32_t width = uint32_t(viewport.width);
			uint32_t height = uint32_t(viewport.height);
			if (!mainTarget.empty()) {
				// 🆕 NEW SYSTEM: Use Handle for width/height
				ImageHandle handle = getImageHandleByName(mainTarget);
				Image2D* image = resourceRegistry_->getResourceAs<Image2D>(handle);

				if (image) {
					width = image->width();
					height = image->height();
				}
				else {
					// ⚠️ Resource not found - use default viewport dimensions
					printLog("WARNING: Main target '{}' not found, using default viewport dimensions", mainTarget);
				}
			}

			// Setup rendering info
			VkRect2D renderArea = { 0, 0, width, height };
			renderingInfo.renderArea = renderArea;
			renderingInfo.layerCount = 1;
			if (colorAttachments.size() > 0) {
				renderingInfo.colorAttachmentCount = uint32_t(colorAttachments.size());
				renderingInfo.pColorAttachments = colorAttachments.data();
			}

			VkViewport vp{ 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
			VkRect2D sc{ 0, 0, width, height };

			{
				TRACY_CPU_SCOPE("Begin Rendering");
				vkCmdBeginRendering(cmd, &renderingInfo);
				vkCmdSetViewport(cmd, 0, 1, &vp);
				vkCmdSetScissor(cmd, 0, 1, &sc);
			}

			// Process all pipelines for this render node
			{
				TRACY_CPU_SCOPE("ProcessPipelines");

				for (auto& pipelineName : renderNode.pipelineNames) {
					// Use a scoped block for each pipeline instead of dynamic scope
					{
						TRACY_CPU_SCOPE("Pipeline Processing");

						vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
							pipelines_.at(pipelineName)->pipeline());

						pipelines_.at(pipelineName)->bindDescriptorSets(cmd, currentFrame);

						if (pipelineName == "sky") {
							TRACY_CPU_SCOPE("drawSky");
							vkCmdDraw(cmd, 36, 1, 0, 0);
							continue;
						}

						if (pipelineName == "post") {
							TRACY_CPU_SCOPE("drawPost");
							vkCmdDraw(cmd, 6, 1, 0, 0);
							continue;
						}

						if (pipelineName == "shadowMap") {
							TRACY_CPU_SCOPE("shadowMapSetup");
							vkCmdSetDepthBias(cmd, 1.1f, 0.0f, 2.0f);
						}

						// Render all visible models for this pipeline
						{
							TRACY_CPU_SCOPE("DrawModels");

							VkDeviceSize offsets[1]{ 0 };
							size_t visibleMeshCount = 0;
							size_t totalMeshCount = 0;

							for (size_t j = 0; j < models.size(); j++) {
								if (!models[j]->visible()) {
									continue;
								}

								// Render all meshes in this model
								for (size_t i = 0; i < models[j]->meshes().size(); i++) {
									auto& mesh = models[j]->meshes()[i];
									totalMeshCount++;

									// Skip culled meshes
									if (mesh.isCulled) {
										continue;
									}
									visibleMeshCount++;

									PbrPushConstants pushConstants;
									pushConstants.model = models[j]->modelMatrix();
									pushConstants.materialIndex = mesh.materialIndex_;
									memcpy(pushConstants.coeffs, models[j]->coeffs(),
										sizeof(pushConstants.coeffs));
									vkCmdPushConstants(cmd, pipelines_.at(pipelineName)->pipelineLayout(),
										VK_SHADER_STAGE_VERTEX_BIT |
										VK_SHADER_STAGE_FRAGMENT_BIT,
										0, sizeof(PbrPushConstants), &pushConstants);

									// Bind vertex and index buffers
									vkCmdBindVertexBuffers(cmd, 0, 1, &mesh.vertexBuffer_, offsets);
									vkCmdBindIndexBuffer(cmd, mesh.indexBuffer_, 0, VK_INDEX_TYPE_UINT32);

									// Draw the mesh
									vkCmdDrawIndexed(cmd, static_cast<uint32_t>(mesh.indices_.size()), 1, 0,
										0, 0);
								}
							}

							// Track rendering statistics
							TRACY_PLOT("VisibleMeshes", static_cast<int64_t>(visibleMeshCount));
							TRACY_PLOT("TotalMeshes", static_cast<int64_t>(totalMeshCount));
							TRACY_PLOT("CulledMeshes",
								static_cast<int64_t>(totalMeshCount - visibleMeshCount));
						}
					}
				}
			}

			{
				TRACY_CPU_SCOPE("End Rendering");
				vkCmdEndRendering(cmd);
			}
		}
	}

	void Renderer::createPipelines(const VkFormat swapChainColorFormat, const VkFormat depthFormat)
	{
		TRACY_CPU_SCOPE("Renderer::createPipelines");

		{
			TRACY_CPU_SCOPE("Read Render Graph");
			renderGraph_.readFromFile("RenderGraph.json");
		}

		// Select optimal HDR format with proper priority (float formats first)
		VkFormat selectedHDRFormat =
			selectOptimalHDRFormat(false, false); // No alpha, moderate precision

		printLog("HDR Format Selection:");
		printLog("  Selected format: {} ({} bytes/pixel)", vkFormatToString(selectedHDRFormat),
			getFormatSize(selectedHDRFormat));

		{
			TRACY_CPU_SCOPE("Create Graphics Pipelines");

			// Create PBR deferred pipeline with multiple render targets (G-buffer)
			std::vector<VkFormat> pbrDeferredFormats = {
				VK_FORMAT_R8G8B8A8_UNORM,
				VK_FORMAT_R16G16B16A16_SFLOAT,
				VK_FORMAT_R32G32B32A32_SFLOAT,
				VK_FORMAT_R8G8B8A8_UNORM
			};

			pipelines_["pbrDeferred"] = std::make_unique<Pipeline>(
				ctx_, shaderManager_, PipelineConfig::createPbrDeferred(),
				pbrDeferredFormats, depthFormat, VK_SAMPLE_COUNT_1_BIT
			);

			// Sky pipeline (renders to HDR target)
			pipelines_["sky"] = std::make_unique<Pipeline>(
				ctx_, shaderManager_, PipelineConfig::createSky(),
				std::vector<VkFormat>{ selectedHDRFormat }, depthFormat, VK_SAMPLE_COUNT_1_BIT
			);

			// Post-processing pipeline (renders to swapchain color format)
			pipelines_["post"] = std::make_unique<Pipeline>(
				ctx_, shaderManager_, PipelineConfig::createPost(),
				std::vector<VkFormat>{ swapChainColorFormat }, depthFormat, VK_SAMPLE_COUNT_1_BIT
			);

			// Shadow map pipeline: depth-only
			pipelines_["shadowMap"] = std::make_unique<Pipeline>(
				ctx_, shaderManager_, PipelineConfig::createShadowMap(),
				std::vector<VkFormat>{}, VK_FORMAT_D16_UNORM, VK_SAMPLE_COUNT_1_BIT
			);
		}

		{
			TRACY_CPU_SCOPE("Create Compute Pipelines");
			// Deferred lighting is a compute pipeline - no color/depth formats required
			pipelines_["deferredLighting"] = std::make_unique<Pipeline>(
				ctx_, shaderManager_, PipelineConfig::createDeferredLighting(),
				std::vector<VkFormat>{}, std::optional<VkFormat>(std::nullopt), VK_SAMPLE_COUNT_1_BIT
			);
		}

		// Store the selected format for texture creation
		selectedHDRFormat_ = selectedHDRFormat;
	}

	void Renderer::createTextures(uint32_t swapchainWidth, uint32_t swapchainHeight)
	{
		TRACY_CPU_SCOPE("Renderer::createTextures");

		{
			TRACY_CPU_SCOPE("createSamplers");
			samplerLinearRepeat_.createLinearRepeat();
			samplerLinearClamp_.createLinearClamp();
			samplerAnisoRepeat_.createAnisoRepeat();
			samplerAnisoClamp_.createAnisoClamp();
			samplerShadow_.createShadow();
		}

		// 🆕 Initialize per-frame buffer vectors
		resourceHandles_.sceneData.resize(kMaxFramesInFlight_);
		resourceHandles_.skyOptions.resize(kMaxFramesInFlight_);
		resourceHandles_.options.resize(kMaxFramesInFlight_);
		resourceHandles_.boneData.resize(kMaxFramesInFlight_);
		resourceHandles_.postOptions.resize(kMaxFramesInFlight_);
		resourceHandles_.ssaoOptions.resize(kMaxFramesInFlight_);

		{
			TRACY_CPU_SCOPE("loadIBLTextures");
			// Load IBL textures for PBR rendering
			string path = kAssetsPathPrefix_ + "textures/golden_gate_hills_4k/";

			printLog("Loading IBL textures...");
			printLog("  Prefiltered: {}", path + "specularGGX.ktx2");
			printLog("  Irradiance: {}", path + "diffuseLambertian.ktx2");
			printLog("  BRDF LUT: {}", path + "outputLUT.png");

			// 🆕 Load prefiltered environment map using ResourceRegistry
			{
				auto prefilteredMap = std::make_unique<Image2D>(ctx_);
				prefilteredMap->createTextureFromKtx2(path + "specularGGX.ktx2", true);
				prefilteredMap->setSampler(samplerLinearRepeat_.handle());
				resourceHandles_.prefilteredMap = resourceRegistry_->registerImage(  //  -> 사용
					"prefilteredMap", std::move(prefilteredMap)
				);
			}

			// 🆕 Load irradiance map
			{
				auto irradianceMap = std::make_unique<Image2D>(ctx_);
				irradianceMap->createTextureFromKtx2(path + "diffuseLambertian.ktx2", true);
				irradianceMap->setSampler(samplerLinearRepeat_.handle());
				resourceHandles_.irradianceMap = resourceRegistry_->registerImage(  //  -> 사용
					"irradianceMap", std::move(irradianceMap)
				);
			}

			// 🆕 Load BRDF lookup table
			{
				auto brdfLut = std::make_unique<Image2D>(ctx_);
				brdfLut->createTextureFromImage(path + "outputLUT.png", false, false);
				brdfLut->setSampler(samplerLinearClamp_.handle());
				resourceHandles_.brdfLut = resourceRegistry_->registerImage(  //  -> 사용
					"brdfLut", std::move(brdfLut)
				);
			}

			printLog(" IBL textures loaded successfully");
		}

		{
			TRACY_CPU_SCOPE("createHDRRenderTargets");
			// Create HDR render targets with selected format
			printLog("Creating HDR render targets:");
			printLog("  Format: {} ({} bytes/pixel)", vkFormatToString(selectedHDRFormat_),
				getFormatSize(selectedHDRFormat_));

			// Log memory usage analysis
			logHDRMemoryUsage(swapchainWidth, swapchainHeight);

			// Storage color buffers for compute shaders and post-processing
			VkImageUsageFlags storageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
				VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT |
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
				VK_IMAGE_USAGE_TRANSFER_DST_BIT;

			// 🆕 Create floatColor1 using ResourceRegistry
			{
				auto floatColor1 = std::make_unique<Image2D>(ctx_);
				floatColor1->createImage(
					selectedHDRFormat_, swapchainWidth, swapchainHeight, VK_SAMPLE_COUNT_1_BIT,
					storageUsage, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 0, VK_IMAGE_VIEW_TYPE_2D);
				floatColor1->setSampler(samplerLinearClamp_.handle());
				resourceHandles_.floatColor1 = resourceRegistry_->registerImage(  //  -> 사용
					"floatColor1", std::move(floatColor1)
				);
			}

			// 🆕 Create floatColor2
			{
				auto floatColor2 = std::make_unique<Image2D>(ctx_);
				floatColor2->createImage(
					selectedHDRFormat_, swapchainWidth, swapchainHeight, VK_SAMPLE_COUNT_1_BIT,
					storageUsage, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 0, VK_IMAGE_VIEW_TYPE_2D);
				floatColor2->setSampler(samplerLinearClamp_.handle());
				resourceHandles_.floatColor2 = resourceRegistry_->registerImage(  //  -> 사용
					"floatColor2", std::move(floatColor2)
				);
			}

			printLog(" HDR render targets created successfully");
		}

		{
			TRACY_CPU_SCOPE("createGBuffer");
			// Create G-buffer textures for deferred rendering
			printLog("Creating G-buffer textures for deferred rendering:");

			// G-Buffer format selection for optimal memory usage and precision
			VkFormat albedoFormat = VK_FORMAT_R8G8B8A8_UNORM; // Albedo + Metallic (4 bytes)
			VkFormat normalFormat =
				VK_FORMAT_R16G16B16A16_SFLOAT; // Normal + Roughness (8 bytes, needs precision)
			VkFormat positionFormat =
				VK_FORMAT_R32G32B32A32_SFLOAT; // Position + Depth (16 bytes, needs high precision)
			VkFormat materialFormat = VK_FORMAT_R8G8B8A8_UNORM; // AO + Emissive + Material ID (4 bytes)

			printLog("  gAlbedo: {} ({} bytes/pixel)", vkFormatToString(albedoFormat),
				getFormatSize(albedoFormat));
			printLog("  gNormal: {} ({} bytes/pixel)", vkFormatToString(normalFormat),
				getFormatSize(normalFormat));
			printLog("  gPosition: {} ({} bytes/pixel)", vkFormatToString(positionFormat),
				getFormatSize(positionFormat));
			printLog("  gMaterial: {} ({} bytes/pixel)", vkFormatToString(materialFormat),
				getFormatSize(materialFormat));

			// G-buffer usage flags
			VkImageUsageFlags gBufferUsage =
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

			// 🆕 Create G-Buffer using ResourceRegistry
			{
				auto gAlbedo = std::make_unique<Image2D>(ctx_);
				gAlbedo->createImage(albedoFormat, swapchainWidth, swapchainHeight,
					VK_SAMPLE_COUNT_1_BIT, gBufferUsage,
					VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 0, VK_IMAGE_VIEW_TYPE_2D);
				gAlbedo->setSampler(samplerLinearClamp_.handle());
				resourceHandles_.gAlbedo = resourceRegistry_->registerImage(  //  -> 사용
					"gAlbedo", std::move(gAlbedo)
				);
			}

			{
				auto gNormal = std::make_unique<Image2D>(ctx_);
				gNormal->createImage(normalFormat, swapchainWidth, swapchainHeight,
					VK_SAMPLE_COUNT_1_BIT, gBufferUsage,
					VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 0, VK_IMAGE_VIEW_TYPE_2D);
				gNormal->setSampler(samplerLinearClamp_.handle());
				resourceHandles_.gNormal = resourceRegistry_->registerImage(  //  -> 사용
					"gNormal", std::move(gNormal)
				);
			}

			{
				auto gPosition = std::make_unique<Image2D>(ctx_);
				gPosition->createImage(positionFormat, swapchainWidth, swapchainHeight,
					VK_SAMPLE_COUNT_1_BIT, gBufferUsage,
					VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 0, VK_IMAGE_VIEW_TYPE_2D);
				gPosition->setSampler(samplerLinearClamp_.handle());
				resourceHandles_.gPosition = resourceRegistry_->registerImage(  //  -> 사용
					"gPosition", std::move(gPosition)
				);
			}

			{
				auto gMaterial = std::make_unique<Image2D>(ctx_);
				gMaterial->createImage(materialFormat, swapchainWidth, swapchainHeight,
					VK_SAMPLE_COUNT_1_BIT, gBufferUsage,
					VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 0, VK_IMAGE_VIEW_TYPE_2D);
				gMaterial->setSampler(samplerLinearClamp_.handle());
				resourceHandles_.gMaterial = resourceRegistry_->registerImage(  //  -> 사용
					"gMaterial", std::move(gMaterial)
				);
			}

			printLog(" G-buffer creation complete");
		}

		{
			TRACY_CPU_SCOPE("createDepthAndShadowBuffers");

			// 🆕 Create Depth/Shadow using ResourceRegistry
			{
				auto depthStencil = std::make_unique<Image2D>(ctx_);
				depthStencil->createDepthBuffer(swapchainWidth, swapchainHeight);
				depthStencil->setSampler(samplerLinearClamp_.handle());
				resourceHandles_.depthStencil = resourceRegistry_->registerImage(  //  -> 사용
					"depthStencil", std::move(depthStencil)
				);
			}

			{
				uint32_t shadowMapSize = 2048 * 2;
				auto shadowMap = std::make_unique<Image2D>(ctx_);
				shadowMap->createShadow(shadowMapSize, shadowMapSize);
				shadowMap->setSampler(samplerShadow_.handle());
				resourceHandles_.shadowMap = resourceRegistry_->registerImage(  //  -> 사용
					"shadowMap", std::move(shadowMap)
				);
			}

			printLog(" Depth and shadow buffers created successfully");
		}

		printLog(" All textures and render targets created successfully");
	}

	// Format selection function with proper priority: float formats first, R8G8B8A8 last
	VkFormat Renderer::selectOptimalHDRFormat(bool needsAlpha, bool fullPrecision)
	{
		TRACY_CPU_SCOPE("Renderer::selectOptimalHDRFormat");

		vector<VkFormat> candidateFormats;

		if (!needsAlpha && !fullPrecision) {
			// Memory-efficient HDR formats (no alpha, moderate precision)
			candidateFormats = {
				VK_FORMAT_B10G11R11_UFLOAT_PACK32,
				VK_FORMAT_R16G16B16_SFLOAT,
				VK_FORMAT_R16G16B16A16_SFLOAT,
				VK_FORMAT_R32G32B32A32_SFLOAT,
				VK_FORMAT_R8G8B8A8_UNORM
			};
		}
		else if (!fullPrecision) {
			// Standard HDR with alpha channel
			candidateFormats = {
				VK_FORMAT_R16G16B16A16_SFLOAT,
				VK_FORMAT_R32G32B32A32_SFLOAT,
				VK_FORMAT_R8G8B8A8_UNORM
			};
		}
		else {
			// Full precision required
			candidateFormats = {
				VK_FORMAT_R32G32B32A32_SFLOAT,
				VK_FORMAT_R32G32B32_SFLOAT,
				VK_FORMAT_R16G16B16A16_SFLOAT,
				VK_FORMAT_R8G8B8A8_UNORM
			};
		}

		// Test each format for compatibility (float formats first, R8G8B8A8 last)
		for (size_t i = 0; i < candidateFormats.size(); ++i) {
			VkFormat format = candidateFormats[i];

			if (isFormatSuitableForHDR(format)) {
				string formatType = (format == VK_FORMAT_R8G8B8A8_UNORM) ? "NON-FLOAT" : "FLOAT";
				float memoryRatio = static_cast<float>(getFormatSize(format)) / 8.0f; // vs RGBA16F

				printLog("✓ Selected HDR format: {} ({} bytes/pixel, {}, {:.0f}% memory vs RGBA16F)",
					vkFormatToString(format), getFormatSize(format), formatType,
					memoryRatio * 100.0f);

				// Warn if we fell back to non-float format
				if (format == VK_FORMAT_R8G8B8A8_UNORM) {
					printLog("⚠️ WARNING: Using R8G8B8A8_UNORM for HDR - limited dynamic range!");
					printLog("  Consider using float formats for better HDR quality");
				}

				return format;
			}
			else {
				string formatType = (format == VK_FORMAT_R8G8B8A8_UNORM) ? "NON-FLOAT" : "FLOAT";
				printLog("✗ Format {} ({}) not supported, trying next...", vkFormatToString(format),
					formatType);
			}
		}

		// Emergency fallback - this should rarely happen
		printLog(
			"⚠ All candidate formats failed, using emergency fallback: VK_FORMAT_R16G16B16A16_SFLOAT");
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	}

	// Enhanced format validation function
	bool Renderer::isFormatSuitableForHDR(VkFormat format)
	{
		TRACY_CPU_SCOPE("Renderer::isFormatSuitableForHDR");

		// Check if format supports required features for HDR rendering
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(ctx_.physicalDevice(), format, &props);

		// Required features for HDR color attachments
		VkFormatFeatureFlags requiredFeatures =
			VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | // Can render to it
			VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;     // Can sample from it

		// Optional but preferred for HDR
		VkFormatFeatureFlags preferredFeatures =
			VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT; // Can blend (for transparency)

		bool hasRequired = (props.optimalTilingFeatures & requiredFeatures) == requiredFeatures;
		bool hasPreferred = (props.optimalTilingFeatures & preferredFeatures) == preferredFeatures;

		if (hasRequired && !hasPreferred && format != VK_FORMAT_R8G8B8A8_UNORM) {
			printLog("  Note: {} missing blend support but acceptable for HDR",
				vkFormatToString(format));
		}

		return hasRequired;
	}

	void Renderer::logHDRMemoryUsage(uint32_t width, uint32_t height)
	{
		TRACY_CPU_SCOPE("Renderer::logHDRMemoryUsage");

		uint64_t totalPixels = static_cast<uint64_t>(width) * height;

		uint32_t hdrBytes = getFormatSize(selectedHDRFormat_);
		uint32_t standardBytes = getFormatSize(VK_FORMAT_R16G16B16A16_SFLOAT);

		// Calculate memory usage for current format (no MSAA, so 1x samples)
		uint64_t hdrMemoryMB = (totalPixels * hdrBytes + totalPixels * hdrBytes * 2) / (1024 * 1024);
		uint64_t standardMemoryMB =
			(totalPixels * standardBytes + totalPixels * standardBytes * 2) / (1024 * 1024);

		float savings =
			(1.0f - static_cast<float>(hdrMemoryMB) / static_cast<float>(standardMemoryMB)) * 100.0f;

		printLog("HDR Memory Analysis:");
		printLog("  Resolution: {}x{} (no MSAA)", width, height);
		printLog("  Current format memory: {} MB", hdrMemoryMB);
		printLog("  Standard RGBA16F memory: {} MB", standardMemoryMB);
		if (savings > 0) {
			printLog("  Memory savings: {:.1f}%", savings);
		}
		else {
			printLog("  Memory overhead: {:.1f}%", -savings);
		}

		// Quality assessment
		if (selectedHDRFormat_ == VK_FORMAT_R8G8B8A8_UNORM) {
			printLog("  Quality: ⚠️ LIMITED - R8G8B8A8 has restricted HDR range");
		}
		else if (selectedHDRFormat_ == VK_FORMAT_B10G11R11_UFLOAT_PACK32) {
			printLog("  Quality: ✓ GOOD - B10G11R11 excellent for HDR with memory savings");
		}
		else if (selectedHDRFormat_ == VK_FORMAT_R16G16B16A16_SFLOAT) {
			printLog("  Quality: ✓ EXCELLENT - Standard HDR format");
		}
		else {
			printLog("  Quality: ✓ HIGH - Float format suitable for HDR");
		}
	}

	void Renderer::updateViewFrustum(const glm::mat4& viewProjection)
	{
		TRACY_CPU_SCOPE("Renderer::updateViewFrustum");

		if (frustumCullingEnabled_) {
			viewFrustum_.extractFromViewProjection(viewProjection);
		}
	}

	void Renderer::performFrustumCulling(vector<unique_ptr<Model>>& models)
	{
		TRACY_CPU_SCOPE("Renderer::performFrustumCulling");

		cullingStats_.totalMeshes = 0;
		cullingStats_.culledMeshes = 0;
		cullingStats_.renderedMeshes = 0;

		if (!frustumCullingEnabled_) {
			TRACY_CPU_SCOPE("Frustum Culling Disabled");
			for (auto& model : models) {
				for (auto& mesh : model->meshes()) {
					mesh.isCulled = false;
					cullingStats_.totalMeshes++;
					cullingStats_.renderedMeshes++;
				}
			}
			return;
		}

		{
			TRACY_CPU_SCOPE("frustumCullingLoop");
			for (auto& model : models) {
				for (auto& mesh : model->meshes()) {
					cullingStats_.totalMeshes++;

					bool isVisible = viewFrustum_.intersects(mesh.worldBounds);

					mesh.isCulled = !isVisible;

					if (isVisible) {
						cullingStats_.renderedMeshes++;
					}
					else {
						cullingStats_.culledMeshes++;
					}
				}
			}
		}

		// Track culling statistics in Tracy
		TRACY_PLOT("FrustumCulling_TotalMeshes", static_cast<int64_t>(cullingStats_.totalMeshes));
		TRACY_PLOT("FrustumCulling_RenderedMeshes", static_cast<int64_t>(cullingStats_.renderedMeshes));
		TRACY_PLOT("FrustumCulling_CulledMeshes", static_cast<int64_t>(cullingStats_.culledMeshes));

		if (cullingStats_.totalMeshes > 0) {
			float cullingEfficiency =
				(float(cullingStats_.culledMeshes) / float(cullingStats_.totalMeshes)) * 100.0f;
			TRACY_PLOT("FrustumCulling_EfficiencyPercent", static_cast<int64_t>(cullingEfficiency));
		}
	}

	void Renderer::performFrustumCulling(vector<Model*>& models)
	{
		TRACY_CPU_SCOPE("Renderer::performFrustumCulling (Model*)");

		cullingStats_.totalMeshes = 0;
		cullingStats_.culledMeshes = 0;
		cullingStats_.renderedMeshes = 0;

		if (!frustumCullingEnabled_) {
			for (auto* model : models) {
				if (!model) continue;
				for (auto& mesh : model->meshes()) {
					mesh.isCulled = false;
					cullingStats_.totalMeshes++;
					cullingStats_.renderedMeshes++;
				}
			}
			return;
		}

		for (auto* model : models) {
			if (!model) continue;
			for (auto& mesh : model->meshes()) {
				cullingStats_.totalMeshes++;
				bool isVisible = viewFrustum_.intersects(mesh.worldBounds);
				mesh.isCulled = !isVisible;
				if (isVisible) cullingStats_.renderedMeshes++; else cullingStats_.culledMeshes++;
			}
		}

		// Update Tracy plots
		TRACY_PLOT("FrustumCulling_TotalMeshes", static_cast<int64_t>(cullingStats_.totalMeshes));
		TRACY_PLOT("FrustumCulling_RenderedMeshes", static_cast<int64_t>(cullingStats_.renderedMeshes));
		TRACY_PLOT("FrustumCulling_CulledMeshes", static_cast<int64_t>(cullingStats_.culledMeshes));
	}

	void Renderer::updateWorldBounds(vector<unique_ptr<Model>>& models)
	{
		TRACY_CPU_SCOPE("Renderer::updateWorldBounds");

		for (auto& model : models) {
			for (auto& mesh : model->meshes()) {
				mesh.updateWorldBounds(model->modelMatrix());
			}
		}
	}

	void Renderer::updateWorldBounds(vector<Model*>& models)
	{
		TRACY_CPU_SCOPE("Renderer::updateWorldBounds (Model*)");

		for (auto* model : models) {
			if (!model) continue;
			for (auto& mesh : model->meshes()) {
				mesh.updateWorldBounds(model->modelMatrix());
			}
		}
	}

	void Renderer::setFrustumCullingEnabled(bool enabled)
	{
		frustumCullingEnabled_ = enabled;
	}

	bool Renderer::isFrustumCullingEnabled() const
	{
		return frustumCullingEnabled_;
	}

	const CullingStats& Renderer::getCullingStats() const
	{
		return cullingStats_;
	}

	VkRenderingAttachmentInfo Renderer::createColorAttachment(VkImageView imageView,
		VkAttachmentLoadOp loadOp,
		VkClearColorValue clearColor,
		VkImageView resolveImageView,
		VkResolveModeFlagBits resolveMode) const
	{
		VkRenderingAttachmentInfo attachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
		attachment.imageView = imageView;
		attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachment.loadOp = loadOp;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.clearValue.color = clearColor;
		attachment.resolveMode = resolveMode;
		attachment.resolveImageView = resolveImageView;
		attachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		return attachment;
	}

	VkRenderingAttachmentInfo Renderer::createDepthAttachment(VkImageView imageView,
		VkAttachmentLoadOp loadOp,
		float clearDepth,
		VkImageView resolveImageView,
		VkResolveModeFlagBits resolveMode) const
	{
		VkRenderingAttachmentInfo attachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
		attachment.imageView = imageView;
		attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachment.loadOp = loadOp;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.clearValue.depthStencil = { clearDepth, 0 };
		attachment.resolveMode = resolveMode;
		attachment.resolveImageView = resolveImageView;
		attachment.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		return attachment;
	}

	// ========================================
	//  NEW API: Model* 기반 오버로드 구현
	// ========================================

	void Renderer::update(Camera& camera, vector<Model*>& models, uint32_t currentFrame, double time)
	{
		TRACY_CPU_SCOPE("Renderer::update (Model*)");

		// ========================================
		//  GPU Instancing: Detect if any model uses instancing
		// ========================================
		{
			TRACY_CPU_SCOPE("Detect GPU Instancing");
			bool anyInstancedModel = false;
			for (auto* model : models) {
				if (model && model->getInstanceCount() > 1) {
					anyInstancedModel = true;
					break;
				}
			}
			optionsUBO_.isInstanced = anyInstancedModel ? 1 : 0;
		}

		{
			TRACY_CPU_SCOPE("Update View Frustum");
			updateViewFrustum(camera.matrices.perspective * camera.matrices.view);
		}

		{
			TRACY_CPU_SCOPE("Update World Bounds");
			updateWorldBounds(models);
		}

		{
			TRACY_CPU_SCOPE("Update Bone Data");
			updateBoneData(models, currentFrame);
		}

		{
			TRACY_CPU_SCOPE("Perform Frustum Culling");
			if (frustumCullingEnabled_) {
				performFrustumCulling(models);
			}
		}

		{
			TRACY_CPU_SCOPE("Update Uniform Buffers");

			// 🆕 NEW SYSTEM: Update scene UBO
			if (auto* buffer = resourceRegistry_->getResourceAs<MappedBuffer>(resourceHandles_.sceneData[currentFrame])) {
				sceneUBO_.projection = camera.matrices.perspective;
				sceneUBO_.view = camera.matrices.view;
				sceneUBO_.cameraPos = camera.position;
				buffer->updateFromCpuData();
			}

			if (auto* buffer = resourceRegistry_->getResourceAs<MappedBuffer>(resourceHandles_.options[currentFrame])) {
				buffer->updateFromCpuData();
			}

			if (auto* buffer = resourceRegistry_->getResourceAs<MappedBuffer>(resourceHandles_.skyOptions[currentFrame])) {
				buffer->updateFromCpuData();
			}

			if (auto* buffer = resourceRegistry_->getResourceAs<MappedBuffer>(resourceHandles_.postOptions[currentFrame])) {
				buffer->updateFromCpuData();
			}

			if (auto* buffer = resourceRegistry_->getResourceAs<MappedBuffer>(resourceHandles_.ssaoOptions[currentFrame])) {
				buffer->updateFromCpuData();
			}
		}
	}

	void Renderer::updateBoneData(const vector<Model*>& models, uint32_t currentFrame)
	{
		TRACY_CPU_SCOPE("Renderer::updateBoneData (Model*)");

		// Reset bone data
		boneDataUBO_.animationData.x = 0.0f;
		for (int i = 0; i < 65; ++i) {
			boneDataUBO_.boneMatrices[i] = glm::mat4(1.0f);
		}

		// Check if any model has animation data
		bool hasAnyAnimation = false;
		for (auto* model : models) {
			if (model && model->hasAnimations() && model->hasBones()) {
				hasAnyAnimation = true;

				// Get bone matrices from the first animated model
				const auto& boneMatrices = model->getBoneMatrices();

				// Copy bone matrices (up to 65 bones)
				const size_t maxBones = 65;
				size_t bonesToCopy = (boneMatrices.size() < maxBones) ? boneMatrices.size() : maxBones;
				for (size_t i = 0; i < bonesToCopy; ++i) {
					boneDataUBO_.boneMatrices[i] = boneMatrices[i];
				}

				break; // For now, use the first animated model
			}
		}

		boneDataUBO_.animationData.x = float(hasAnyAnimation);

		// DEBUG: Log hasAnimation state
		static bool lastHasAnimation = false;
		if (lastHasAnimation != hasAnyAnimation) {
			printLog("hasAnimation changed to: {}", hasAnyAnimation);
			lastHasAnimation = hasAnyAnimation;
		}

		// 🆕 NEW SYSTEM: Update via Handle
		if (auto* buffer = resourceRegistry_->getResourceAs<MappedBuffer>(resourceHandles_.boneData[currentFrame])) {
			buffer->updateFromCpuData();
		}
	}

	void Renderer::draw(VkCommandBuffer cmd, uint32_t currentFrame, VkImageView swapchainImageView,
		vector<Model*>& models, VkViewport viewport, VkRect2D scissor)
	{
		TRACY_CPU_SCOPE("Renderer::draw (Model*)");

		for (auto& renderNode : renderGraph_.renderNodes_) {
			if (renderNode.pipelineNames[0] == "deferredLighting") {
				TRACY_CPU_SCOPE("deferredLighting");
				pipelines_.at("deferredLighting")->dispatch(cmd, currentFrame);
				continue;
			}

			string mainTarget;
			vector<VkRenderingAttachmentInfo> colorAttachments{};
			VkRenderingAttachmentInfo depthAttachment{};
			VkRenderingInfo renderingInfo{};
			renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;

			// Handle color attachments
			{
				TRACY_CPU_SCOPE("Setup Color Attachments");
				for (const auto& colorTarget : renderNode.colorAttachments) {
					if (colorTarget == "swapchain") {
						colorAttachments.push_back(createColorAttachment(
							swapchainImageView, VK_ATTACHMENT_LOAD_OP_CLEAR, { 0.0f, 0.0f, 1.0f, 0.0f }));
						continue;
					}

					if (mainTarget.empty()) {
						mainTarget = colorTarget;
					}

					// 🆕 NEW SYSTEM: Use Handle
					ImageHandle handle = getImageHandleByName(colorTarget);
					Image2D* image = resourceRegistry_->getResourceAs<Image2D>(handle);

					if (image) {
						if (renderNode.pipelineNames[0] == "sky") {
							colorAttachments.push_back(createColorAttachment(
								image->view(), VK_ATTACHMENT_LOAD_OP_LOAD, { 0.0f, 0.0f, 0.5f, 0.0f }));
						}
						else {
							colorAttachments.push_back(createColorAttachment(
								image->view(), VK_ATTACHMENT_LOAD_OP_CLEAR, { 0.0f, 0.0f, 0.5f, 0.0f }));
						}
					}
					else {
						printLog("ERROR: Color target '{}' not found in ResourceRegistry!", colorTarget);
					}
				}
			}

			// Handle depth attachment
			{
				TRACY_CPU_SCOPE("Setup Depth Attachment");
				if (!renderNode.depthAttachment.empty()) {
					if (mainTarget.empty()) {
						mainTarget = renderNode.depthAttachment;
					}

					// 🆕 NEW SYSTEM: Use Handle (now safe!)
					ImageHandle handle = getImageHandleByName(renderNode.depthAttachment);
					Image2D* image = resourceRegistry_->getResourceAs<Image2D>(handle);

					if (image) {
						image->transitionToDepthStencilAttachment(cmd);

						if (renderNode.pipelineNames[0] == "sky") {
							depthAttachment = createDepthAttachment(
								image->attachmentView(),
								VK_ATTACHMENT_LOAD_OP_LOAD, 1.0f);
						}
						else {
							depthAttachment = createDepthAttachment(
								image->attachmentView(),
								VK_ATTACHMENT_LOAD_OP_CLEAR, 1.0f);
						}

						renderingInfo.pDepthAttachment = &depthAttachment;
					}
					else {
						printLog("ERROR: Depth attachment '{}' not found in ResourceRegistry!", renderNode.depthAttachment);
					}
				}
			}

			{
				TRACY_CPU_SCOPE("Submit Pipeline Barriers");
				for (auto& pipelineName : renderNode.pipelineNames) {
					pipelines_.at(pipelineName)->submitBarriers(cmd, currentFrame);
				}
			}

			//  FIX: Get dimensions properly
			uint32_t width = uint32_t(viewport.width);
			uint32_t height = uint32_t(viewport.height);

			if (!mainTarget.empty()) {
				ImageHandle handle = getImageHandleByName(mainTarget);
				Image2D* image = resourceRegistry_->getResourceAs<Image2D>(handle);

				if (image) {
					width = image->width();
					height = image->height();
				}
				else {
					printLog("WARNING: Main target '{}' not found, using viewport dimensions", mainTarget);
				}
			}

			// Setup rendering info
			VkRect2D renderArea = { 0, 0, width, height };
			renderingInfo.renderArea = renderArea;
			renderingInfo.layerCount = 1;

			if (!colorAttachments.empty()) {
				renderingInfo.colorAttachmentCount = uint32_t(colorAttachments.size());
				renderingInfo.pColorAttachments = colorAttachments.data();
			}

			VkViewport vp{ 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
			VkRect2D sc{ 0, 0, width, height };

			{
				TRACY_CPU_SCOPE("Begin Rendering");
				vkCmdBeginRendering(cmd, &renderingInfo);
				vkCmdSetViewport(cmd, 0, 1, &vp);
				vkCmdSetScissor(cmd, 0, 1, &sc);
			}

			// Process pipelines
			{
				TRACY_CPU_SCOPE("ProcessPipelines");

				for (auto& pipelineName : renderNode.pipelineNames) {
					{
						TRACY_CPU_SCOPE("Pipeline Processing");

						vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
							pipelines_.at(pipelineName)->pipeline());
						pipelines_.at(pipelineName)->bindDescriptorSets(cmd, currentFrame);

						if (pipelineName == "sky") {
							TRACY_CPU_SCOPE("drawSky");
							vkCmdDraw(cmd, 36, 1, 0, 0);
							continue;
						}

						if (pipelineName == "post") {
							TRACY_CPU_SCOPE("drawPost");
							vkCmdDraw(cmd, 6, 1, 0, 0);
							continue;
						}

						if (pipelineName == "shadowMap") {
							TRACY_CPU_SCOPE("shadowMapSetup");
							vkCmdSetDepthBias(cmd, 1.1f, 0.0f, 2.0f);
						}

						//  Render models (Model* version)
						{
							TRACY_CPU_SCOPE("DrawModels");

							VkDeviceSize offsets[1]{ 0 };
							size_t visibleMeshCount = 0;
							size_t totalMeshCount = 0;

							for (auto* model : models) {
								if (!model || !model->visible()) continue;

								// ========================================
								//  GPU Instancing: Get instance count & buffer
								// ========================================
								uint32_t instanceCount = model->getInstanceCount();
								VkBuffer instanceBuffer = model->getInstanceBuffer();

								if (instanceCount == 0) {
									instanceCount = 1; // Fallback for non-instanced models
								}

								// ========================================
								//  GPU Instancing: Set shader flag
								// ========================================
								bool isInstanced = (instanceCount > 1 && instanceBuffer != VK_NULL_HANDLE);

								for (auto& mesh : model->meshes()) {
									totalMeshCount++;

									if (mesh.isCulled) {
										continue;
									}
									visibleMeshCount++;

									PbrPushConstants pushConstants;
									pushConstants.model = model->modelMatrix();
									pushConstants.materialIndex = mesh.materialIndex_;
									memcpy(pushConstants.coeffs, model->coeffs(), sizeof(pushConstants.coeffs));

									vkCmdPushConstants(cmd, pipelines_.at(pipelineName)->pipelineLayout(),
										VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
										0, sizeof(PbrPushConstants), &pushConstants);

									// ========================================
									//  GPU Instancing: Bind vertex + instance buffers
									// ========================================
									vkCmdBindVertexBuffers(cmd, 0, 1, &mesh.vertexBuffer_, offsets);

									// Bind instance buffer if available
									if (isInstanced) {
										vkCmdBindVertexBuffers(cmd, 1, 1, &instanceBuffer, offsets);
									}

									vkCmdBindIndexBuffer(cmd, mesh.indexBuffer_, 0, VK_INDEX_TYPE_UINT32);

									// ========================================
									//  GPU Instancing: Draw with instanceCount
									// ========================================
									vkCmdDrawIndexed(cmd, static_cast<uint32_t>(mesh.indices_.size()),
										instanceCount, 0, 0, 0);
								}
							}

							// Track rendering statistics
							TRACY_PLOT("VisibleMeshes", static_cast<int64_t>(visibleMeshCount));
							TRACY_PLOT("TotalMeshes", static_cast<int64_t>(totalMeshCount));
							TRACY_PLOT("CulledMeshes", static_cast<int64_t>(totalMeshCount - visibleMeshCount));
						}
					}
				}
			}

			{
				TRACY_CPU_SCOPE("End Rendering");
				vkCmdEndRendering(cmd);
			}
		}
	}

	// ========================================
	//  NEW: Material 동적 업데이트
	//  NEW: RenderPass system helper methods
	// ========================================

	void Renderer::updateMaterials(const vector<unique_ptr<Model>>& models)
	{
		TRACY_CPU_SCOPE("Renderer::updateMaterials (unique_ptr)");

		vector<MaterialUBO> allMaterials;

		for (const auto& model : models) {
			if (model) {
				model->prepareForBindlessRendering(samplerLinearRepeat_, allMaterials, *materialTextures_);
			}
		}

		if (allMaterials.empty()) {
			printLog("WARNING: No materials to update, keeping dummy material");
			return;
		}

		// Recreate material buffer with new data
		materialBuffer_ = std::make_unique<StorageBuffer>(ctx_, allMaterials.data(),
			sizeof(MaterialUBO) * allMaterials.size());

		printLog("Updated material buffer with {} materials", allMaterials.size());
	}

	void Renderer::updateMaterials(const vector<Model*>& models)
	{
		TRACY_CPU_SCOPE("Renderer::updateMaterials (Model*)");

		vector<MaterialUBO> allMaterials;

		for (auto* model : models) {
			if (model) {
				model->prepareForBindlessRendering(samplerLinearRepeat_, allMaterials, *materialTextures_);
			}
		}

		if (allMaterials.empty()) {
			printLog("WARNING: No materials to update, keeping dummy material");
			return;
		}

		// Recreate material buffer with new data
		materialBuffer_ = std::make_unique<StorageBuffer>(ctx_, allMaterials.data(),
			sizeof(MaterialUBO) * allMaterials.size());

		printLog("Updated material buffer with {} materials", allMaterials.size());

		// Recreate descriptor sets that reference materialBuffer_
		updateMaterialDescriptorSets();
	}

	void Renderer::updateMaterialDescriptorSets()
	{
		TRACY_CPU_SCOPE("Renderer::updateMaterialDescriptorSets");

		printLog("Recreating material descriptor sets...");

		// "material" descriptor set을 다시 생성
		vector<string> bindingNames = { "materialBuffer", "materialTextures" };

		vector<reference_wrapper<Resource>> resources;
		for (const string& resourceName : bindingNames) {
			addResource(resourceName, uint32_t(-1), resources);
		}

		// Recreate descriptor set
		if (pipelines_.find("pbrDeferred") != pipelines_.end()) {
			descriptorSets_["material"].create(
				ctx_,
				pipelines_["pbrDeferred"]->layouts()[1],
				resources
			);

			printLog(" Recreated 'material' descriptor set");

			// Rebind to relevant pipelines
			for (const string& pipelineName : { string("pbrDeferred"), string("pbrForward") }) {
				if (pipelines_.find(pipelineName) == pipelines_.end()) continue;

				vector<string> setNames;
				if (pipelineName == "pbrDeferred") {
					setNames = { "sceneOptions", "material" };
				}
				else if (pipelineName == "pbrForward") {
					setNames = { "sceneOptions", "material", "sky", "shadowMap" };
				}

				vector<vector<reference_wrapper<DescriptorSet>>> pipelineDescriptorSets;
				pipelineDescriptorSets.resize(kMaxFramesInFlight_);

				for (uint32_t frameIndex = 0; frameIndex < kMaxFramesInFlight_; ++frameIndex) {
					pipelineDescriptorSets[frameIndex].reserve(setNames.size());
					for (const string& setName : setNames) {
						if (perFrameDescriptorSets_.find(setName) != perFrameDescriptorSets_.end()) {
							pipelineDescriptorSets[frameIndex].emplace_back(
								std::ref(perFrameDescriptorSets_[setName][frameIndex]));
						}
						else if (descriptorSets_.find(setName) != descriptorSets_.end()) {
							pipelineDescriptorSets[frameIndex].emplace_back(
								std::ref(descriptorSets_[setName]));
						}
					}
				}

				pipelines_[pipelineName]->setDescriptorSets(pipelineDescriptorSets);
				printLog(" Rebound descriptor sets to pipeline '{}'", pipelineName);
			}
		}
	}
} // namespace BinRenderer::Vulkan