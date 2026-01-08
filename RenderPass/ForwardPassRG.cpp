#include "ForwardPassRG.h"
#include "../Core/Logger.h"
#include "../Core/RHIScene.h"
#include "../Rendering/RHIRenderer.h"
#include "../Rendering/RHIVertex.h"
#include "../RHI/Vulkan/VulkanRHI.h"
#include "../RHI/Vulkan/Pipeline/VulkanPipeline.h"
#include <fstream>
#include <vector>

using namespace BinRenderer::Vulkan;

namespace BinRenderer
{
	// =========================================================================================
	//  ForwardPassRG Implementation
	//
	//  이 클래스는 사용자 정의 렌더 패스를 구현하는 예시입니다.
	//  RenderGraph (RG) 시스템 내에서 동작하며, 초기화 -> 설정 -> 실행 단계로 구성됩니다.
	// =========================================================================================

	// Helper: 셰이더 파일 로드 (유틸리티로 분리 가능)
	static std::vector<uint32_t> loadShaderCode(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::binary | std::ios::ate);
		if (!file.is_open())
		{
			printLog("❌ [ForwardPassRG] Failed to open shader file: {}", filename);
			return {};
		}

		size_t fileSize = static_cast<size_t>(file.tellg());
		file.seekg(0);
		std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
		file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
		return buffer;
	}

	ForwardPassRG::ForwardPassRG(RHI* rhi, RHIScene* scene, RHIRenderer* renderer)
		: RGPass<ForwardPassData>(rhi, "ForwardPass")
		, scene_(scene)
		, renderer_(renderer)
	{
	}

	ForwardPassRG::~ForwardPassRG()
	{
		shutdown();
	}

	// -----------------------------------------------------------------------------------------
	//  1. Initialize (초기화)
	//  - 렌더링에 필요한 고정 리소스(Pipeline, Shader, Constant Buffers)를 생성합니다.
	//  - 앱 실행 시 한 번만 호출됩니다.
	// -----------------------------------------------------------------------------------------
	bool ForwardPassRG::initialize()
	{
		printLog("[ForwardPassRG] Initializing...");
		
		// 1. 테스트용 더미 리소스 생성 (실제 엔진에서는 Scene/AssetManager 등에서 가져옴)
		createDummyResources();
		
		// 2. 디스크립터 세트 레이아웃 및 풀 생성
		createDescriptorSets();
		
		// 3. 그래픽스 파이프라인 생성 (Shaders, States)
		createPipeline();
		
		printLog("[ForwardPassRG] Initialized successfully");
		return true;
	}

	// -----------------------------------------------------------------------------------------
	//  4. Shutdown (종료)
	//  - 생성된 모든 RHI 리소스를 정리합니다.
	// -----------------------------------------------------------------------------------------
	void ForwardPassRG::shutdown()
	{
		destroyDescriptorSets();
		destroyPipeline();
		destroyDummyResources();
	}

	// -----------------------------------------------------------------------------------------
	//  2. Setup (설정)
	//  - Render Graph 빌더를 통해 이 패스가 사용하는 입력(Read)과 출력(Write) 리소스를 정의합니다.
	//  - 텍스처 생성 및 의존성 관리가 여기서 수행됩니다.
	// -----------------------------------------------------------------------------------------
	void ForwardPassRG::setup(ForwardPassData& data, RenderGraphBuilder& builder)
	{
		// 출력 텍스처 정의 (HDR Color Buffer)
		RGTextureDesc forwardDesc;
		forwardDesc.name = "Forward_Output";
		forwardDesc.width = 1280;  // TODO: Window 크기에 동기화
		forwardDesc.height = 720;
		forwardDesc.format = RHI_FORMAT_R8G8B8A8_UNORM; // 나중에 HDR 포맷(R16G16B16A16)으로 변경 권장
		forwardDesc.usage = RHI_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | RHI_IMAGE_USAGE_TRANSFER_SRC_BIT;

		// 리소스 생성 및 쓰기 권한 요청
		data.forwardOut = builder.createTexture(forwardDesc);
		builder.writeTexture(data.forwardOut);
		
		printLog("[ForwardPassRG] Setup complete - Output: Forward_Output");
	}

	// -----------------------------------------------------------------------------------------
	//  3. Execute (실행)
	//  - 실제 커맨드 버퍼에 렌더링 명령을 기록합니다.
	//  - 매 프레임 호출됩니다.
	// -----------------------------------------------------------------------------------------
	void ForwardPassRG::execute(const ForwardPassData& data, RHI* rhi, uint32_t frameIndex)
	{
		// RHI 기록 시작
		rhi->beginCommandRecording();
		
		// -------------------------------------------------------
		// A. 렌더 타겟 준비 (Swapchain View 가져오기)
		// -------------------------------------------------------
		auto* swapchain = rhi->getSwapchain();
		if (!swapchain)
		{
			rhi->endCommandRecording();
			return;
		}

		uint32_t imageIndex = rhi->getCurrentImageIndex();
		RHIImageViewHandle targetImageView = swapchain->getImageView(imageIndex);

		uint32_t renderWidth = renderer_ ? renderer_->getWidth() : 1280;
		uint32_t renderHeight = renderer_ ? renderer_->getHeight() : 720;

		// -------------------------------------------------------
		// B. Dynamic Rendering 시작
		// - RenderPass 객체 없이 색상/깊이 첨부물을 직접 지정
		// -------------------------------------------------------
		rhi->cmdBeginRendering(renderWidth, renderHeight, targetImageView, {}); // Depth attachment 생략 (현재)
		
		// -------------------------------------------------------
		// C. 뷰포트 및 가위(Scissor) 설정
		// -------------------------------------------------------
		RHIViewport viewport{ 0.0f, 0.0f, (float)renderWidth, (float)renderHeight, 0.0f, 1.0f };
		rhi->cmdSetViewport(viewport);

		RHIRect2D scissor{ {0, 0}, {renderWidth, renderHeight} };
		rhi->cmdSetScissor(scissor);

		// -------------------------------------------------------
		// D. 파이프라인 바인딩
		// -------------------------------------------------------
		if (pipeline_.isValid())
		{
			rhi->cmdBindPipeline(pipeline_);
		}

		// -------------------------------------------------------
		// E. 디스크립터 세트(리소스) 바인딩
		// -------------------------------------------------------
		if (!sceneDescriptorSets_.empty() && pipeline_.isValid())
		{
			uint32_t currentFrame = frameIndex % sceneDescriptorSets_.size();
			
			std::vector<RHIDescriptorSetHandle> setsToBind;
			setsToBind.push_back(sceneDescriptorSets_[currentFrame]); // Set 0: Scene Data
			if (materialDescriptorSet_.isValid()) setsToBind.push_back(materialDescriptorSet_); // Set 1: Material
			if (iblDescriptorSet_.isValid()) setsToBind.push_back(iblDescriptorSet_);           // Set 2: IBL
			if (shadowDescriptorSet_.isValid()) setsToBind.push_back(shadowDescriptorSet_);     // Set 3: Shadow

			if (!setsToBind.empty())
			{
				rhi->cmdBindDescriptorSets(pipeline_, 0, setsToBind.data(), (uint32_t)setsToBind.size());
			}
		}

		// -------------------------------------------------------
		// F. 씬 렌더링 (Draw Calls)
		// -------------------------------------------------------
		if (scene_ && pipeline_.isValid())
		{
			const auto& nodes = scene_->getNodes();
			
			for (const auto& node : nodes)
			{
				if (!node.model || !node.visible) continue;

				// Push Constants: Model Matrix 등 전달
				struct PushConstants {
					glm::mat4 model;
					uint32_t materialIndex;
					float coeffs[15]; // Padding/Extra
				} pc;
				
				pc.model = node.transform * node.model->getTransform();
				pc.materialIndex = 0; // Temp

				rhi->cmdPushConstants(pipeline_, RHI_SHADER_STAGE_VERTEX_BIT | RHI_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);

				// Mesh Draw
				for (const auto& mesh : node.model->getMeshes())
				{
					if (mesh)
					{
						mesh->bind(rhi);
						mesh->draw(rhi);
					}
				}
			}
		}

		// -------------------------------------------------------
		// G. 렌더링 종료 및 제출
		// -------------------------------------------------------
		rhi->cmdEndRendering();
		rhi->endCommandRecording();
		rhi->submitCommands();
	}

	// =========================================================================================
	//  Private Methods: Resource Creation
	// =========================================================================================

	void ForwardPassRG::createPipeline()
	{
		// 1. 셰이더 로드
		auto vertCode = loadShaderCode("../../assets/shaders/pbrForward.vert.spv");
		auto fragCode = loadShaderCode("../../assets/shaders/pbrForward.frag.spv");

		if (vertCode.empty() || fragCode.empty()) return;

		RHIShaderCreateInfo vInfo{ RHI_SHADER_STAGE_VERTEX_BIT, "pbrForward.vert", "main", std::move(vertCode) };
		RHIShaderCreateInfo fInfo{ RHI_SHADER_STAGE_FRAGMENT_BIT, "pbrForward.frag", "main", std::move(fragCode) };

		vertexShader_ = rhi_->createShader(vInfo);
		fragmentShader_ = rhi_->createShader(fInfo);

		// 2. 파이프라인 상태 설정 (Desc)
		RHIPipelineCreateInfo info{};
		info.useDynamicRendering = true;
		info.colorAttachmentFormats = { RHI_FORMAT_B8G8R8A8_SRGB }; // Swapchain format
		info.depthAttachmentFormat = RHI_FORMAT_D32_SFLOAT;
		info.shaderStages = { vertexShader_, fragmentShader_ };
		
		// Descriptor Layouts
		if (sceneDescriptorLayout_.isValid()) info.descriptorSetLayouts.push_back(sceneDescriptorLayout_);
		if (materialDescriptorLayout_.isValid()) info.descriptorSetLayouts.push_back(materialDescriptorLayout_);
		if (iblDescriptorLayout_.isValid()) info.descriptorSetLayouts.push_back(iblDescriptorLayout_);
		if (shadowDescriptorLayout_.isValid()) info.descriptorSetLayouts.push_back(shadowDescriptorLayout_);

		// Vertex Input (From Helper)
		info.vertexInputState.bindings = { RHIVertexHelper::getVertexBinding() };
		auto attrs = RHIVertexHelper::getVertexAttributesAnimated();
		info.vertexInputState.attributes.assign(attrs.begin(), attrs.end());

		// Push Constants (128 bytes)
		info.pushConstantRanges.push_back({ RHI_SHADER_STAGE_VERTEX_BIT | RHI_SHADER_STAGE_FRAGMENT_BIT, 0, 128 });

		// Rasterizer / Depth / Blend (Default similar settings)
		info.rasterizationState.cullMode = RHI_CULL_MODE_BACK_BIT;
		info.depthStencilState.depthTestEnable = false; // TODO: true for real depth
		info.depthStencilState.depthWriteEnable = true;
		info.colorBlendState.attachments.push_back({ false, 0xF }); // No blending

		// Dynamic States
		info.dynamicStates = { RHI_DYNAMIC_STATE_VIEWPORT, RHI_DYNAMIC_STATE_SCISSOR };

		// 3. 파이프라인 생성
		pipeline_ = rhi_->createPipeline(info);
		printLog("[ForwardPassRG] Pipeline created");
	}

	void ForwardPassRG::destroyPipeline()
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

	void ForwardPassRG::createDescriptorSets()
	{
		printLog("[ForwardPassRG] Creating descriptor layouts and sets...");
		
		if (!renderer_) return;

		// -------------------------------------------------------
		// 1. Descriptor Set Layouts 정의
		// - 셰이더 리소스(UBO, Texture 등)의 바인딩 슬롯을 정의합니다.
		// -------------------------------------------------------
		
		// Set 0: Scene Global Data (UBOs)
		{
			RHIDescriptorSetLayoutCreateInfo info;
			info.bindings = {
				{ 0, RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, RHI_SHADER_STAGE_VERTEX_BIT | RHI_SHADER_STAGE_FRAGMENT_BIT }, // SceneData
				{ 1, RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, RHI_SHADER_STAGE_VERTEX_BIT | RHI_SHADER_STAGE_FRAGMENT_BIT }, // Options
				{ 2, RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, RHI_SHADER_STAGE_VERTEX_BIT }  // BoneData
			};
			sceneDescriptorLayout_ = rhi_->createDescriptorSetLayout(info);
		}

		// Set 1: Materials (StorageBuffer + Textures)
		{
			RHIDescriptorSetLayoutCreateInfo info;
			info.bindings = {
				{ 0, RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, RHI_SHADER_STAGE_FRAGMENT_BIT }, // Material Data
				{ 1, RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 512, RHI_SHADER_STAGE_FRAGMENT_BIT } // Bindless Textures
			};
			materialDescriptorLayout_ = rhi_->createDescriptorSetLayout(info);
		}

		// Set 2: IBL (Environment Maps)
		{
			RHIDescriptorSetLayoutCreateInfo info;
			info.bindings = {
				{ 0, RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, RHI_SHADER_STAGE_FRAGMENT_BIT }, // Prefiltered
				{ 1, RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, RHI_SHADER_STAGE_FRAGMENT_BIT }, // Irradiance
				{ 2, RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, RHI_SHADER_STAGE_FRAGMENT_BIT }  // BRDF LUT
			};
			iblDescriptorLayout_ = rhi_->createDescriptorSetLayout(info);
		}

		// Set 3: Shadow Maps
		{
			RHIDescriptorSetLayoutCreateInfo info;
			info.bindings = {
				{ 0, RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, RHI_SHADER_STAGE_FRAGMENT_BIT } // Shadow map
			};
			shadowDescriptorLayout_ = rhi_->createDescriptorSetLayout(info);
		}

		// -------------------------------------------------------
		// 2. Descriptor Pool 생성
		// - 필요한 총 디스크립터 개수를 추산하여 풀을 생성합니다.
		// -------------------------------------------------------
		RHIDescriptorPoolCreateInfo poolInfo{};
		poolInfo.maxSets = 100; // 넉넉하게
		poolInfo.poolSizes = {
			{ RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
			{ RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 },
			{ RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 }
		};
		descriptorPool_ = rhi_->createDescriptorPool(poolInfo);

		// -------------------------------------------------------
		// 3. Descriptor Sets 할당 및 업데이트 
		// - 실제 리소스(Buffer, ImageView)를 바인딩합니다.
		// -------------------------------------------------------
		
		// Set 0: Per-Frame Scene Data
		uint32_t maxFrames = 2; // Double Buffering
		sceneDescriptorSets_.resize(maxFrames);
		for (uint32_t i = 0; i < maxFrames; i++)
		{
			sceneDescriptorSets_[i] = rhi_->allocateDescriptorSet(descriptorPool_, sceneDescriptorLayout_);
			if (renderer_)
			{
				rhi_->updateDescriptorSet(sceneDescriptorSets_[i], 0, renderer_->getSceneUniformBuffer(i), 0, sizeof(SceneUniform));
				rhi_->updateDescriptorSet(sceneDescriptorSets_[i], 1, renderer_->getOptionsUniformBuffer(i), 0, sizeof(OptionsUniform));
				rhi_->updateDescriptorSet(sceneDescriptorSets_[i], 2, renderer_->getBoneDataUniformBuffer(i), 0, sizeof(BoneDataUniform));
			}
		}

		// Set 1: Material (Global)
		materialDescriptorSet_ = rhi_->allocateDescriptorSet(descriptorPool_, materialDescriptorLayout_);
		if (dummyMaterialBuffer_.isValid())
		{
			rhi_->updateDescriptorSet(materialDescriptorSet_, 0, dummyMaterialBuffer_, 0, 0); // Full range
			// Textures are bound dynamically or bindless (TODO implementation)
			if (dummyTextureView_.isValid())
			{
				rhi_->updateDescriptorSet(materialDescriptorSet_, 1, dummyTextureView_, dummySampler_);
			}
		}

		// Set 2: IBL (Global)
		iblDescriptorSet_ = rhi_->allocateDescriptorSet(descriptorPool_, iblDescriptorLayout_);
		if (dummyCubemapView_.isValid())
		{
			rhi_->updateDescriptorSet(iblDescriptorSet_, 0, dummyCubemapView_, dummySampler_);
			rhi_->updateDescriptorSet(iblDescriptorSet_, 1, dummyCubemapView_, dummySampler_);
			rhi_->updateDescriptorSet(iblDescriptorSet_, 2, dummyTextureView_, dummySampler_); // BRDF (Use 2D for now)
		}

		// Set 3: Shadows (Global)
		shadowDescriptorSet_ = rhi_->allocateDescriptorSet(descriptorPool_, shadowDescriptorLayout_);
		if (dummyShadowMapView_.isValid())
		{
			rhi_->updateDescriptorSet(shadowDescriptorSet_, 0, dummyShadowMapView_, dummySampler_);
		}

		printLog("[ForwardPassRG] Descriptor sets prepared.");
	}

	void ForwardPassRG::destroyDescriptorSets()
	{
		printLog("[ForwardPassRG] Cleaning up descriptor sets...");
		
		sceneDescriptorSets_.clear();
		materialDescriptorSet_ = {};
		iblDescriptorSet_ = {};
		shadowDescriptorSet_ = {};
		
		if (descriptorPool_.isValid()) rhi_->destroyDescriptorPool(descriptorPool_);
		if (sceneDescriptorLayout_.isValid()) rhi_->destroyDescriptorSetLayout(sceneDescriptorLayout_);
		if (materialDescriptorLayout_.isValid()) rhi_->destroyDescriptorSetLayout(materialDescriptorLayout_);
		if (iblDescriptorLayout_.isValid()) rhi_->destroyDescriptorSetLayout(iblDescriptorLayout_);
		if (shadowDescriptorLayout_.isValid()) rhi_->destroyDescriptorSetLayout(shadowDescriptorLayout_);
		
		descriptorPool_ = {};
		sceneDescriptorLayout_ = {};
		materialDescriptorLayout_ = {};
		iblDescriptorLayout_ = {};
		shadowDescriptorLayout_ = {};
	}

	void ForwardPassRG::updateDescriptorSets(uint32_t frameIndex)
	{
		// 이 예제에서는 정적 바인딩을 주로 사용하거나 execute() 시점에 동적 바인딩을 처리합니다.
		// 만약 매 프레임 갱신이 필요한 디스크립터가 있다면 여기서 처리합니다.
	}

	void ForwardPassRG::createDummyResources()
	{
		printLog("[ForwardPassRG] Creating dummy resources (Fallback for missing assets)...");

		// 1. Dummy Sampler
		RHISamplerCreateInfo samplerInfo{}; 
		dummySampler_ = rhi_->createSampler(samplerInfo);

		// 2. Dummy Material Buffer (Default White Material)
		{
			struct DummyMaterialData {
				glm::vec4 emissiveFactor = glm::vec4(0.0f);
				glm::vec4 baseColorFactor = glm::vec4(1.0f);
				float roughness = 1.0f;
				float transparency = 1.0f;
				float discardAlpha = 0.0f;
				float metallic = 0.0f;
				int32_t textureIndices[6] = { -1, -1, -1, -1, -1, -1 }; // Base, Emissive, Normal, Opacity, MetallicRoughness, Occlusion
			};

			RHIBufferCreateInfo bufferInfo{};
			bufferInfo.size = sizeof(DummyMaterialData);
			bufferInfo.usage = RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			bufferInfo.memoryProperties = RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT;

			dummyMaterialBuffer_ = rhi_->createBuffer(bufferInfo);
			if (dummyMaterialBuffer_.isValid())
			{
				DummyMaterialData data;
				void* mapped = rhi_->mapBuffer(dummyMaterialBuffer_);
				memcpy(mapped, &data, sizeof(DummyMaterialData));
				rhi_->unmapBuffer(dummyMaterialBuffer_);
			}
		}

		// Helper to create valid 4x4 dummy textures
		auto createDummyImage = [&](RHIFormat format, RHIImageUsageFlags usage, RHIImageAspectFlags aspect) -> std::pair<RHIImageHandle, RHIImageViewHandle> {
			RHIImageCreateInfo imgInfo{};
			imgInfo.width = 4; imgInfo.height = 4; imgInfo.depth = 1;
			imgInfo.format = format;
			imgInfo.usage = usage | RHI_IMAGE_USAGE_TRANSFER_DST_BIT; // Transfer dst for potential clearing/upload
			
			RHIImageHandle img = rhi_->createImage(imgInfo);
			RHIImageViewHandle view = {};
			
			if (img.isValid()) {
				RHIImageViewCreateInfo viewInfo{};
				viewInfo.viewType = RHI_IMAGE_VIEW_TYPE_2D;
				viewInfo.aspectMask = aspect;
				view = rhi_->createImageView(img, viewInfo);
			}
			return { img, view };
		};

		// 3. Create Textures
		// White Texture
		auto [tex, texView] = createDummyImage(RHI_FORMAT_R8G8B8A8_UNORM, RHI_IMAGE_USAGE_SAMPLED_BIT, RHI_IMAGE_ASPECT_COLOR_BIT);
		dummyTexture_ = tex; dummyTextureView_ = texView;

		// Cubemap (Using 2D fallback for now as requested)
		auto [cube, cubeView] = createDummyImage(RHI_FORMAT_R8G8B8A8_UNORM, RHI_IMAGE_USAGE_SAMPLED_BIT, RHI_IMAGE_ASPECT_COLOR_BIT);
		dummyCubemap_ = cube; dummyCubemapView_ = cubeView;

		// Shadow Map (Depth)
		auto [shadow, shadowView] = createDummyImage(RHI_FORMAT_D32_SFLOAT, RHI_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | RHI_IMAGE_USAGE_SAMPLED_BIT, RHI_IMAGE_ASPECT_DEPTH_BIT);
		dummyShadowMap_ = shadow; dummyShadowMapView_ = shadowView;

		// 4. Transition Layouts
		// 리소스를 사용 가능한 상태(Shader Read Only)로 전이합니다.
		rhi_->beginCommandRecording();
		
		if (dummyTexture_.isValid())
			rhi_->cmdTransitionImageLayout(dummyTexture_, RHI_IMAGE_LAYOUT_UNDEFINED, RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, RHI_IMAGE_ASPECT_COLOR_BIT);
		
		if (dummyCubemap_.isValid())
			rhi_->cmdTransitionImageLayout(dummyCubemap_, RHI_IMAGE_LAYOUT_UNDEFINED, RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, RHI_IMAGE_ASPECT_COLOR_BIT);
			
		if (dummyShadowMap_.isValid())
			rhi_->cmdTransitionImageLayout(dummyShadowMap_, RHI_IMAGE_LAYOUT_UNDEFINED, RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, RHI_IMAGE_ASPECT_DEPTH_BIT);

		rhi_->endCommandRecording();
		rhi_->submitCommands(); // Immediate submit for initialization
		rhi_->waitIdle();

		printLog("[ForwardPassRG] Dummy resources created.");
	}

	void ForwardPassRG::destroyDummyResources()
	{
		if (dummyShadowMapView_.isValid()) rhi_->destroyImageView(dummyShadowMapView_);
		if (dummyShadowMap_.isValid()) rhi_->destroyImage(dummyShadowMap_);
		if (dummyCubemapView_.isValid()) rhi_->destroyImageView(dummyCubemapView_);
		if (dummyCubemap_.isValid()) rhi_->destroyImage(dummyCubemap_);
		if (dummyTextureView_.isValid()) rhi_->destroyImageView(dummyTextureView_);
		if (dummyTexture_.isValid()) rhi_->destroyImage(dummyTexture_);
		if (dummySampler_.isValid()) rhi_->destroySampler(dummySampler_);
		if (dummyMaterialBuffer_.isValid()) rhi_->destroyBuffer(dummyMaterialBuffer_);

		dummyShadowMapView_ = {}; dummyShadowMap_ = {};
		dummyCubemapView_ = {}; dummyCubemap_ = {};
		dummyTextureView_ = {}; dummyTexture_ = {};
		dummySampler_ = {}; dummyMaterialBuffer_ = {};
	}

} // namespace BinRenderer
