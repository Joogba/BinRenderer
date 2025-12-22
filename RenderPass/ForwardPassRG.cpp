#include "ForwardPassRG.h"
#include "../Core/Logger.h"
#include "../Core/RHIScene.h"
#include "../Rendering/RHIRenderer.h"
#include "../Rendering/RHIVertex.h"
#include "../RHI/Vulkan/VulkanRHI.h"
#include "../RHI/Vulkan/Pipeline/VulkanPipeline.h"
#include "../RHI/Vulkan/Pipeline/VulkanDescriptor.h"
#include "../RHI/Vulkan/Commands/VulkanCommandBuffer.h"
#include <vulkan/vulkan.h>
#include <fstream>
#include <vector>

using namespace BinRenderer::Vulkan;

namespace BinRenderer
{
	// 셰이더 파일 읽기 헬퍼 함수
	static std::vector<uint32_t> readShaderFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::binary | std::ios::ate);
		if (!file.is_open())
		{
			printLog("❌ Failed to open shader file: {}", filename);
			return {};
		}

		size_t fileSize = static_cast<size_t>(file.tellg());
		if (fileSize == 0 || fileSize % 4 != 0)
		{
			printLog("❌ Invalid shader file size: {}", filename);
			return {};
		}

		file.seekg(0);
		std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
		file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
		file.close();

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

	bool ForwardPassRG::initialize()
	{
		printLog("[ForwardPassRG] Initializing...");
		
		// 1. Dummy Resources 생성 (Descriptor Sets보다 먼저)
		createDummyResources();
		
		// 2. Descriptor Sets 생성
		createDescriptorSets();
		
		// 3. 파이프라인 생성 (PBR 셰이더 사용)
		createPipeline();
		
		printLog("[ForwardPassRG] Initialized successfully");
		return true;
	}

	void ForwardPassRG::shutdown()
	{
		destroyDescriptorSets();
		destroyPipeline();
		destroyDummyResources();
	}

	void ForwardPassRG::setup(ForwardPassData& data, RenderGraphBuilder& builder)
	{
		printLog("[ForwardPassRG] Setup - Creating output texture");

		// 출력: Forward Output
		RGTextureDesc forwardDesc;
		forwardDesc.name = "Forward_Output";
		forwardDesc.width = 1280;  // TODO: 동적으로 가져오기
		forwardDesc.height = 720;
		forwardDesc.format = RHI_FORMAT_R8G8B8A8_UNORM;
		forwardDesc.usage = RHI_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | 
		                    RHI_IMAGE_USAGE_TRANSFER_SRC_BIT;

		data.forwardOut = builder.createTexture(forwardDesc);
		builder.writeTexture(data.forwardOut);
		
		printLog("[ForwardPassRG] Setup complete - Output texture created");
	}

	void ForwardPassRG::execute(const ForwardPassData& data, RHI* rhi, uint32_t frameIndex)
	{
		if (frameIndex % 60 == 0)
		{
			printLog("[ForwardPassRG] Execute - Frame {}", frameIndex);
			
			// Scene 정보 출력
			if (scene_)
			{
				auto models = scene_->getModels();
				printLog("[ForwardPassRG]   - {} models in scene", models.size());
			}
			else
			{
				printLog("[ForwardPassRG]   - ⚠️  Scene is null!");
			}
		}

		// 커맨드 버퍼 기록 시작
		rhi->beginCommandRecording();
		
		// ========================================
		// Pass 책임: Render Target 및 상태 설정
		// ========================================
		
		// ✅ Swapchain 검증
		auto* swapchain = rhi->getSwapchain();
		if (!swapchain)
		{
			printLog("[ForwardPassRG] ❌ Swapchain is null!");
			rhi->endCommandRecording();
			rhi->submitCommands();
			return;
		}

		// ✅ Swapchain image index 가져오기 (프레임 인덱스가 아님!)
		uint32_t imageIndex = rhi->getCurrentImageIndex();
		
		// ✅ RHIImageView 가져오기 (이제 제대로 구현됨!)
		auto* swapchainImageView = swapchain->getImageView(imageIndex);
		if (!swapchainImageView)
		{
			printLog("[ForwardPassRG] ❌ Swapchain image view is null! (index: {})", imageIndex);
			rhi->endCommandRecording();
			rhi->submitCommands();
			return;
		}

		if (frameIndex % 60 == 0)
		{
			printLog("[ForwardPassRG]   - Using swapchain image {} (frame: {})", imageIndex, frameIndex);
		}

		// ✅ 실제 렌더러 크기 사용 (하드코딩 제거)
		uint32_t renderWidth = renderer_ ? renderer_->getWidth() : 1280;
		uint32_t renderHeight = renderer_ ? renderer_->getHeight() : 720;

		// ✅ Dynamic Rendering 시작
		rhi->cmdBeginRendering(renderWidth, renderHeight, swapchainImageView, nullptr);
		
		// Viewport 및 Scissor 설정
		RHIViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(renderWidth);
		viewport.height = static_cast<float>(renderHeight);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		rhi->cmdSetViewport(viewport);

		RHIRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = {renderWidth, renderHeight};
		rhi->cmdSetScissor(scissor);

		// Pipeline 바인딩
		if (pipeline_)
		{
			rhi->cmdBindPipeline(pipeline_);
			
			if (frameIndex % 60 == 0)
			{
				printLog("[ForwardPassRG]   - Pipeline bound");
			}
		}

		// ❌ Simple 셰이더는 Descriptor Sets 불필요 - 주석 처리
		/*
		// ✅ Descriptor Sets 바인딩 (Set 0: Scene UBO)
		if (!sceneDescriptorSets_.empty() && pipeline_)
		{
			uint32_t currentFrame = frameIndex % sceneDescriptorSets_.size();
			
			// ✅ 모든 Descriptor Sets 바인딩 (Set 0, 1, 2, 3)
			std::vector<RHIDescriptorSet*> allSets;
			allSets.push_back(sceneDescriptorSets_[currentFrame]); // Set 0
			if (materialDescriptorSet_) allSets.push_back(materialDescriptorSet_); // Set 1
			if (iblDescriptorSet_) allSets.push_back(iblDescriptorSet_);           // Set 2
			if (shadowDescriptorSet_) allSets.push_back(shadowDescriptorSet_);     // Set 3
			
			if (!allSets.empty())
			{
				rhi->cmdBindDescriptorSets(pipeline_, 0, allSets.data(), static_cast<uint32_t>(allSets.size()));
				
				if (frameIndex % 60 == 0)
				{
					printLog("[ForwardPassRG]   ✅ All descriptor sets bound ({} sets, frame: {})", 
						allSets.size(), currentFrame);
				}
			}
		}
		*/

		// ========================================
		// Renderer 책임: 실제 렌더링 로직
		// ========================================
		
		if (scene_ && renderer_ && pipeline_)
		{
			// ✅ View와 Projection 행렬 가져오기
			auto& camera = scene_->getCamera();
			glm::mat4 view = camera.getMatrices().view;
			glm::mat4 projection = camera.getMatrices().perspective;

			// ✅ DEBUG: 첫 프레임에 행렬 출력
			if (frameIndex == 0)
			{
				printLog("[ForwardPassRG] Camera Matrices:");
				printLog("  Camera Position: ({:.2f}, {:.2f}, {:.2f})",
					camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);
				printLog("  View[0]: ({:.2f}, {:.2f}, {:.2f}, {:.2f})", 
					view[0][0], view[0][1], view[0][2], view[0][3]);
				printLog("  View[1]: ({:.2f}, {:.2f}, {:.2f}, {:.2f})", 
					view[1][0], view[1][1], view[1][2], view[1][3]);
				printLog("  View[2]: ({:.2f}, {:.2f}, {:.2f}, {:.2f})", 
					view[2][0], view[2][1], view[2][2], view[2][3]);
				printLog("  View[3]: ({:.2f}, {:.2f}, {:.2f}, {:.2f})", 
					view[3][0], view[3][1], view[3][2], view[3][3]);
				printLog("  Proj[1][1]: {:.2f}", projection[1][1]);
			}

			// ✅ Scene Nodes 순회 (transform 포함)
			const auto& nodes = scene_->getNodes();
			
			for (const auto& node : nodes)
			{
				if (!node.model || !node.visible)
					continue;

				// ✅ Model matrix 계산: NodeTransform * ModelTransform
				glm::mat4 modelMatrix = node.transform * node.model->getTransform();
				
				// ✅ MVP 계산: Projection * View * Model (올바른 순서!)
				glm::mat4 mvp = projection * view * modelMatrix;

				// ✅ Push constants로 MVP 전달
				rhi->cmdPushConstants(
					pipeline_,
					RHI_SHADER_STAGE_VERTEX_BIT,
					0,
					sizeof(glm::mat4),
					&mvp
				);

				// ✅ 각 메시 렌더링
				for (const auto& meshPtr : node.model->getMeshes())
				{
					if (!meshPtr)
						continue;

					// RHIMesh의 bind와 draw 메서드 사용
					meshPtr->bind(rhi);
					meshPtr->draw(rhi, 1);
				}
			}
			
			if (frameIndex % 60 == 0)
			{
				printLog("[ForwardPassRG]   - {} scene nodes rendered with MVP", nodes.size());
			}
		}

		// ✅ Dynamic Rendering 종료
		rhi->cmdEndRendering();
		
		if (frameIndex % 60 == 0)
		{
			printLog("[ForwardPassRG]   - Rendering commands recorded");
		}
		
		// 커맨드 버퍼 기록 종료 및 제출
		rhi->endCommandRecording();
		rhi->submitCommands();
	}

	void ForwardPassRG::createPipeline()
	{
		printLog("[ForwardPassRG] Creating PBR pipeline...");

		// ✅ 임시로 Simple 셰이더 사용 (PBR은 나중에)
		auto vertCode = readShaderFile("../../assets/shaders/simple.vert.spv");
		if (vertCode.empty())
		{
			printLog("[ForwardPassRG] ❌ Failed to read vertex shader file");
			return;
		}

		RHIShaderCreateInfo vertShaderInfo{};
		vertShaderInfo.stage = RHI_SHADER_STAGE_VERTEX_BIT;
		vertShaderInfo.name = "simple.vert";
		vertShaderInfo.entryPoint = "main";
		vertShaderInfo.code = std::move(vertCode);

		vertexShader_ = rhi_->createShader(vertShaderInfo);
		if (!vertexShader_)
		{
			printLog("[ForwardPassRG] ❌ Failed to create vertex shader");
			return;
		}
		printLog("[ForwardPassRG]   - Simple Vertex shader created");

		// Fragment Shader 로드
		auto fragCode = readShaderFile("../../assets/shaders/simple.frag.spv");
		if (fragCode.empty())
		{
			printLog("[ForwardPassRG] ❌ Failed to read fragment shader file");
			return;
		}

		RHIShaderCreateInfo fragShaderInfo{};
		fragShaderInfo.stage = RHI_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderInfo.name = "simple.frag";
		fragShaderInfo.entryPoint = "main";
		fragShaderInfo.code = std::move(fragCode);

		fragmentShader_ = rhi_->createShader(fragShaderInfo);
		if (!fragmentShader_)
		{
			printLog("[ForwardPassRG] ❌ Failed to create fragment shader");
			return;
		}
		printLog("[ForwardPassRG]   - Simple Fragment shader created");

		// Pipeline 생성
		RHIPipelineCreateInfo pipelineInfo{};
		
		// ✅ Dynamic Rendering 설정
		pipelineInfo.useDynamicRendering = true;
		pipelineInfo.colorAttachmentFormats.push_back(RHI_FORMAT_B8G8R8A8_SRGB); // Swapchain format
		pipelineInfo.depthAttachmentFormat = RHI_FORMAT_D32_SFLOAT;
		
		// Shader stages 설정
		pipelineInfo.shaderStages.push_back(vertexShader_);
		pipelineInfo.shaderStages.push_back(fragmentShader_);
		
		// ❌ Simple 셰이더는 descriptor sets 불필요
		// (PBR 셰이더 사용 시 다시 활성화)
		/*
		if (sceneDescriptorLayout_)
		{
			pipelineInfo.descriptorSetLayouts.push_back(sceneDescriptorLayout_);
		}
		if (materialDescriptorLayout_)
		{
			pipelineInfo.descriptorSetLayouts.push_back(materialDescriptorLayout_);
		}
		if (iblDescriptorLayout_)
		{
			pipelineInfo.descriptorSetLayouts.push_back(iblDescriptorLayout_);
		}
		if (shadowDescriptorLayout_)
		{
			pipelineInfo.descriptorSetLayouts.push_back(shadowDescriptorLayout_);
		}
		*/
		
		printLog("[ForwardPassRG]   - Pipeline will use {} descriptor set layouts", 
			pipelineInfo.descriptorSetLayouts.size());

		// ✅ Simple 셰이더용 Push constants (MVP만)
		RHIPipelineCreateInfo::PushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = RHI_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(glm::mat4);  // MVP matrix only
		pipelineInfo.pushConstantRanges.push_back(pushConstantRange);

		// ✅ Vertex Input State
		// Binding 0: Vertex data
		RHIVertexInputBinding vertexBinding{};
		vertexBinding.binding = 0;
		vertexBinding.stride = sizeof(RHIVertex);
		vertexBinding.inputRate = RHI_VERTEX_INPUT_RATE_VERTEX;
		pipelineInfo.vertexInputState.bindings.push_back(vertexBinding);

		// Attributes
		uint32_t offset = 0;
		
		// Location 0: position (vec3) - vec4로 패ding
		RHIVertexInputAttribute posAttr{};
		posAttr.location = 0;
		posAttr.binding = 0;
		posAttr.format = RHI_FORMAT_R32G32B32A32_SFLOAT;  // vec3를 vec4로
		posAttr.offset = offset;
		pipelineInfo.vertexInputState.attributes.push_back(posAttr);
		offset += sizeof(glm::vec3);

		// Location 1: normal (vec3) - vec4로 패딩
		RHIVertexInputAttribute normalAttr{};
		normalAttr.location = 1;
		normalAttr.binding = 0;
		normalAttr.format = RHI_FORMAT_R32G32B32A32_SFLOAT;  // vec3를 vec4로
		normalAttr.offset = offset;
		pipelineInfo.vertexInputState.attributes.push_back(normalAttr);
		offset += sizeof(glm::vec3);

		// Location 2: texCoord (vec2) - vec4로 패딩
		RHIVertexInputAttribute texCoordAttr{};
		texCoordAttr.location = 2;
		texCoordAttr.binding = 0;
		texCoordAttr.format = RHI_FORMAT_R32G32B32A32_SFLOAT;  // vec2를 vec4로
		texCoordAttr.offset = offset;
		pipelineInfo.vertexInputState.attributes.push_back(texCoordAttr);
		offset += sizeof(glm::vec2);

		// Location 3: tangent (vec4)
		RHIVertexInputAttribute tangentAttr{};
		tangentAttr.location = 3;
		tangentAttr.binding = 0;
		tangentAttr.format = RHI_FORMAT_R32G32B32A32_SFLOAT;
		tangentAttr.offset = offset;
		pipelineInfo.vertexInputState.attributes.push_back(tangentAttr);
		
		// Input Assembly State
		pipelineInfo.inputAssemblyState.topology = RHI_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		pipelineInfo.inputAssemblyState.primitiveRestartEnable = false;
		
		// Viewport State (동적 상태로 설정)
		pipelineInfo.viewportState.viewportCount = 1;
		pipelineInfo.viewportState.scissorCount = 1;
		
		// Rasterization State
		pipelineInfo.rasterizationState.depthClampEnable = false;
		pipelineInfo.rasterizationState.rasterizerDiscardEnable = false;
		pipelineInfo.rasterizationState.polygonMode = RHI_POLYGON_MODE_FILL;
		pipelineInfo.rasterizationState.cullMode = RHI_CULL_MODE_BACK_BIT;
		pipelineInfo.rasterizationState.frontFace = RHI_FRONT_FACE_COUNTER_CLOCKWISE;
		pipelineInfo.rasterizationState.depthBiasEnable = false;
		pipelineInfo.rasterizationState.lineWidth = 1.0f;
		
		// Multisample State
		pipelineInfo.multisampleState.rasterizationSamples = RHI_SAMPLE_COUNT_1_BIT;
		pipelineInfo.multisampleState.sampleShadingEnable = false;
		
		// Depth Stencil State
		pipelineInfo.depthStencilState.depthTestEnable = false;
		pipelineInfo.depthStencilState.depthWriteEnable = true;
		pipelineInfo.depthStencilState.depthCompareOp = RHI_COMPARE_OP_LESS;
		pipelineInfo.depthStencilState.stencilTestEnable = false;
		
		// Color Blend State
		RHIPipelineColorBlendAttachment colorBlendAttachment{};
		colorBlendAttachment.blendEnable = false;
		colorBlendAttachment.colorWriteMask = 0xF; // RGBA
		pipelineInfo.colorBlendState.attachments.push_back(colorBlendAttachment);
		
		// Dynamic States
		pipelineInfo.dynamicStates.push_back(RHI_DYNAMIC_STATE_VIEWPORT);
		pipelineInfo.dynamicStates.push_back(RHI_DYNAMIC_STATE_SCISSOR);

		pipeline_ = rhi_->createPipeline(pipelineInfo);
		if (!pipeline_)
		{
			printLog("[ForwardPassRG] ❌ Failed to create pipeline");
			return;
		}

		printLog("[ForwardPassRG] ✅ Pipeline created successfully");
	}

	void ForwardPassRG::destroyPipeline()
	{
		if (pipeline_) {
			rhi_->destroyPipeline(pipeline_);
			pipeline_ = nullptr;
		}

		if (vertexShader_) {
			rhi_->destroyShader(vertexShader_);
			vertexShader_ = nullptr;
		}

		if (fragmentShader_) {
			rhi_->destroyShader(fragmentShader_);
			fragmentShader_ = nullptr;
		}
	}

	void ForwardPassRG::createDescriptorSets()
	{
		printLog("[ForwardPassRG] Creating descriptor sets for PBR rendering...");
		
		if (!renderer_)
		{
			printLog("[ForwardPassRG] ⚠️  Renderer is null, cannot create descriptor sets");
			return;
		}

		// ========================================
		// Set 0: Scene UBO (SceneData, Options, BoneData)
		// ========================================
		{
			RHIDescriptorSetLayoutCreateInfo layoutInfo{};
			
			// Binding 0: SceneDataUBO
			RHIDescriptorSetLayoutBinding sceneBinding{};
			sceneBinding.binding = 0;
			sceneBinding.descriptorType = RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			sceneBinding.descriptorCount = 1;
			sceneBinding.stageFlags = RHI_SHADER_STAGE_VERTEX_BIT | RHI_SHADER_STAGE_FRAGMENT_BIT;
			layoutInfo.bindings.push_back(sceneBinding);
			
			// Binding 1: OptionsUBO
			RHIDescriptorSetLayoutBinding optionsBinding{};
			optionsBinding.binding = 1;
			optionsBinding.descriptorType = RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			optionsBinding.descriptorCount = 1;
			optionsBinding.stageFlags = RHI_SHADER_STAGE_VERTEX_BIT | RHI_SHADER_STAGE_FRAGMENT_BIT;
			layoutInfo.bindings.push_back(optionsBinding);
			
			// Binding 2: BoneDataUBO
			RHIDescriptorSetLayoutBinding boneBinding{};
			boneBinding.binding = 2;
			boneBinding.descriptorType = RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			boneBinding.descriptorCount = 1;
			boneBinding.stageFlags = RHI_SHADER_STAGE_VERTEX_BIT;
			layoutInfo.bindings.push_back(boneBinding);
			
			sceneDescriptorLayout_ = rhi_->createDescriptorSetLayout(layoutInfo);
			if (!sceneDescriptorLayout_)
			{
				printLog("[ForwardPassRG] ❌ Failed to create scene descriptor layout");
				return;
			}
			printLog("[ForwardPassRG]   ✅ Scene descriptor layout created");
		}

		// ========================================
		// Set 1: Material (Material Buffer + Textures)
		// ========================================
		{
			RHIDescriptorSetLayoutCreateInfo layoutInfo{};
			
			// Binding 0: Material Storage Buffer
			RHIDescriptorSetLayoutBinding materialBufferBinding{};
			materialBufferBinding.binding = 0;
			materialBufferBinding.descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			materialBufferBinding.descriptorCount = 1;
			materialBufferBinding.stageFlags = RHI_SHADER_STAGE_FRAGMENT_BIT;
			layoutInfo.bindings.push_back(materialBufferBinding);
			
			// Binding 1: Material Textures (bindless, 512개)
			RHIDescriptorSetLayoutBinding texturesBinding{};
			texturesBinding.binding = 1;
			texturesBinding.descriptorType = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			texturesBinding.descriptorCount = 512; // TextureManager::kMaxTextures_
			texturesBinding.stageFlags = RHI_SHADER_STAGE_FRAGMENT_BIT;
			layoutInfo.bindings.push_back(texturesBinding);
			
			materialDescriptorLayout_ = rhi_->createDescriptorSetLayout(layoutInfo);
			if (!materialDescriptorLayout_)
			{
				printLog("[ForwardPassRG] ❌ Failed to create material descriptor layout");
				return;
			}
			printLog("[ForwardPassRG]   ✅ Material descriptor layout created");
		}

		// ========================================
		// Set 2: IBL (Dummy for now)
		// ========================================
		{
			RHIDescriptorSetLayoutCreateInfo layoutInfo{};
			
			// Binding 0: Prefiltered Map (cubemap)
			RHIDescriptorSetLayoutBinding prefilteredBinding{};
			prefilteredBinding.binding = 0;
			prefilteredBinding.descriptorType = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			prefilteredBinding.descriptorCount = 1;
			prefilteredBinding.stageFlags = RHI_SHADER_STAGE_FRAGMENT_BIT;
			layoutInfo.bindings.push_back(prefilteredBinding);
			
			// Binding 1: Irradiance Map (cubemap)
			RHIDescriptorSetLayoutBinding irradianceBinding{};
			irradianceBinding.binding = 1;
			irradianceBinding.descriptorType = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			irradianceBinding.descriptorCount = 1;
			irradianceBinding.stageFlags = RHI_SHADER_STAGE_FRAGMENT_BIT;
			layoutInfo.bindings.push_back(irradianceBinding);
			
			// Binding 2: BRDF LUT (2D texture)
			RHIDescriptorSetLayoutBinding brdfBinding{};
			brdfBinding.binding = 2;
			brdfBinding.descriptorType = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			brdfBinding.descriptorCount = 1;
			brdfBinding.stageFlags = RHI_SHADER_STAGE_FRAGMENT_BIT;
			layoutInfo.bindings.push_back(brdfBinding);
			
			iblDescriptorLayout_ = rhi_->createDescriptorSetLayout(layoutInfo);
			if (!iblDescriptorLayout_)
			{
				printLog("[ForwardPassRG] ❌ Failed to create IBL descriptor layout");
				return;
			}
			printLog("[ForwardPassRG]   ✅ IBL descriptor layout created");
		}

		// ========================================
		// Set 3: Shadow Map (Dummy for now)
		// ========================================
		{
			RHIDescriptorSetLayoutCreateInfo layoutInfo{};
			
			// Binding 0: Shadow Map (depth texture)
			RHIDescriptorSetLayoutBinding shadowBinding{};
			shadowBinding.binding = 0;
			shadowBinding.descriptorType = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			shadowBinding.descriptorCount = 1;
			shadowBinding.stageFlags = RHI_SHADER_STAGE_FRAGMENT_BIT;
			layoutInfo.bindings.push_back(shadowBinding);
			
			shadowDescriptorLayout_ = rhi_->createDescriptorSetLayout(layoutInfo);
			if (!shadowDescriptorLayout_)
			{
				printLog("[ForwardPassRG] ❌ Failed to create shadow descriptor layout");
				return;
			}
			printLog("[ForwardPassRG]   ✅ Shadow descriptor layout created");
		}

		// ========================================
		// Descriptor Pool 생성
		// ========================================
		{
			RHIDescriptorPoolCreateInfo poolInfo{};
			poolInfo.maxSets = 20; // 여유있게 할당
			
			// Uniform buffers (Set 0: 3 bindings * 2 frames)
			RHIDescriptorPoolSize uniformPoolSize{};
			uniformPoolSize.type = RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uniformPoolSize.descriptorCount = 6;
			poolInfo.poolSizes.push_back(uniformPoolSize);
			
			// Storage buffers (Set 1: Material buffer)
			RHIDescriptorPoolSize storagePoolSize{};
			storagePoolSize.type = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			storagePoolSize.descriptorCount = 2;
			poolInfo.poolSizes.push_back(storagePoolSize);
			
			// Combined image samplers (Set 1: 512 textures + Set 2: 3 IBL + Set 3: 1 shadow)
			RHIDescriptorPoolSize samplerPoolSize{};
			samplerPoolSize.type = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			samplerPoolSize.descriptorCount = 520;
			poolInfo.poolSizes.push_back(samplerPoolSize);
			
			descriptorPool_ = rhi_->createDescriptorPool(poolInfo);
			if (!descriptorPool_)
			{
				printLog("[ForwardPassRG] ❌ Failed to create descriptor pool");
				return;
			}
			printLog("[ForwardPassRG]   ✅ Descriptor pool created");
		}

		// ========================================
		// Scene Descriptor Sets 할당 (per-frame)
		// ========================================
		{
			uint32_t maxFrames = 2; // TODO: renderer에서 가져오기
			sceneDescriptorSets_.resize(maxFrames);
			
			for (uint32_t i = 0; i < maxFrames; i++)
			{
				sceneDescriptorSets_[i] = rhi_->allocateDescriptorSet(descriptorPool_, sceneDescriptorLayout_);
				if (!sceneDescriptorSets_[i])
				{
					printLog("[ForwardPassRG] ❌ Failed to allocate scene descriptor set {}", i);
					return;
				}
				
				// Uniform buffers 바인딩
				RHIBuffer* sceneBuffer = renderer_->getSceneUniformBuffer(i);
				RHIBuffer* optionsBuffer = renderer_->getOptionsUniformBuffer(i);
				RHIBuffer* boneBuffer = renderer_->getBoneDataUniformBuffer(i);
				
				if (sceneBuffer)
				{
					sceneDescriptorSets_[i]->updateBuffer(0, sceneBuffer, 0, sizeof(SceneUniform));
				}
				if (optionsBuffer)
				{
					sceneDescriptorSets_[i]->updateBuffer(1, optionsBuffer, 0, sizeof(OptionsUniform));
				}
				if (boneBuffer)
				{
					sceneDescriptorSets_[i]->updateBuffer(2, boneBuffer, 0, sizeof(BoneDataUniform));
				}
			}
			
			printLog("[ForwardPassRG]   ✅ Scene descriptor sets allocated and updated ({})", maxFrames);
		}

		// ========================================
		// Material Descriptor Set 할당 (공유)
		// ========================================
		{
			materialDescriptorSet_ = rhi_->allocateDescriptorSet(descriptorPool_, materialDescriptorLayout_);
			if (!materialDescriptorSet_)
			{
				printLog("[ForwardPassRG] ❌ Failed to allocate material descriptor set");
				return;
			}
			
			// ✅ Material buffer 바인딩
			if (dummyMaterialBuffer_)
			{
				materialDescriptorSet_->updateBuffer(0, dummyMaterialBuffer_, 0, 0);
			}

			// ✅ Dummy texture 바인딩 (binding 1에 첫 번째만 - 나머지는 나중에)
			// TODO: 512개 배열 바인딩 API 필요
			if (dummyTextureView_ && dummySampler_)
			{
				materialDescriptorSet_->updateImage(1, dummyTextureView_, dummySampler_);
			}

			printLog("[ForwardPassRG]   ✅ Material descriptor set allocated and bound");
		}

		// ========================================
		// IBL Descriptor Set 할당 (공유)
		// ========================================
		{
			iblDescriptorSet_ = rhi_->allocateDescriptorSet(descriptorPool_, iblDescriptorLayout_);
			if (!iblDescriptorSet_)
			{
				printLog("[ForwardPassRG] ❌ Failed to allocate IBL descriptor set");
				return;
			}
			
			// ✅ Dummy cubemaps 바인딩
			if (dummyCubemapView_ && dummySampler_)
			{
				iblDescriptorSet_->updateImage(0, dummyCubemapView_, dummySampler_); // Prefiltered
				iblDescriptorSet_->updateImage(1, dummyCubemapView_, dummySampler_); // Irradiance
				iblDescriptorSet_->updateImage(2, dummyTextureView_, dummySampler_);  // BRDF LUT (2D)
			}

			printLog("[ForwardPassRG]   ✅ IBL descriptor set allocated and bound");
		}

		// ========================================
		// Shadow Descriptor Set 할당 (공유)
		// ========================================
		{
			shadowDescriptorSet_ = rhi_->allocateDescriptorSet(descriptorPool_, shadowDescriptorLayout_);
			if (!shadowDescriptorSet_)
			{
				printLog("[ForwardPassRG] ❌ Failed to allocate shadow descriptor set");
				return;
			}
			
			// ✅ Dummy shadow map 바인딩
			if (dummyShadowMapView_ && dummySampler_)
			{
				shadowDescriptorSet_->updateImage(0, dummyShadowMapView_, dummySampler_);
			}

			printLog("[ForwardPassRG]   ✅ Shadow descriptor set allocated and bound");
		}
		
		printLog("[ForwardPassRG] ✅ All descriptor sets created successfully");
	}

	void ForwardPassRG::destroyDescriptorSets()
	{
		printLog("[ForwardPassRG] Cleaning up descriptor sets...");
		
		// Descriptor Sets는 Pool이 파괴되면 자동으로 해제됨
		sceneDescriptorSets_.clear();
		materialDescriptorSet_ = nullptr;
		iblDescriptorSet_ = nullptr;
		shadowDescriptorSet_ = nullptr;
		
		// Descriptor Pool 파괴
		if (descriptorPool_)
		{
			rhi_->destroyDescriptorPool(descriptorPool_);
			descriptorPool_ = nullptr;
		}
		
		// Descriptor Layouts 파괴
		if (sceneDescriptorLayout_)
		{
			rhi_->destroyDescriptorSetLayout(sceneDescriptorLayout_);
			sceneDescriptorLayout_ = nullptr;
		}
		if (materialDescriptorLayout_)
		{
			rhi_->destroyDescriptorSetLayout(materialDescriptorLayout_);
			materialDescriptorLayout_ = nullptr;
		}
		if (iblDescriptorLayout_)
		{
			rhi_->destroyDescriptorSetLayout(iblDescriptorLayout_);
			iblDescriptorLayout_ = nullptr;
		}
		if (shadowDescriptorLayout_)
		{
			rhi_->destroyDescriptorSetLayout(shadowDescriptorLayout_);
			shadowDescriptorLayout_ = nullptr;
		}
		
		printLog("[ForwardPassRG] ✅ Descriptor sets cleanup complete");
	}

	void ForwardPassRG::updateDescriptorSets(uint32_t frameIndex)
	{
		// TODO: Update descriptor sets per frame
		// - Scene UBO binding
		// - Material textures binding
	}

	void ForwardPassRG::createDummyResources()
	{
		printLog("[ForwardPassRG] Creating dummy resources...");

		// ========================================
		// Dummy Sampler
		// ========================================
		RHISamplerCreateInfo samplerInfo{};
		dummySampler_ = rhi_->createSampler(samplerInfo);
		if (!dummySampler_)
		{
			printLog("[ForwardPassRG] ❌ Failed to create dummy sampler");
			return;
		}

		// ========================================
		// Dummy Material Buffer (단일 material)
		// ========================================
		{
			struct DummyMaterialData {
				glm::vec4 emissiveFactor = glm::vec4(0.0f);
				glm::vec4 baseColorFactor = glm::vec4(1.0f); // 흰색
				float roughness = 1.0f;
				float transparency = 1.0f;
				float discardAlpha = 0.0f;
				float metallic = 0.0f;
				int32_t baseColorTextureIndex = -1;
				int32_t emissiveTextureIndex = -1;
				int32_t normalTextureIndex = -1;
				int32_t opacityTextureIndex = -1;
				int32_t metallicRoughnessTextureIndex = -1;
				int32_t occlusionTextureIndex = -1;
			};

			RHIBufferCreateInfo bufferInfo{};
			bufferInfo.size = sizeof(DummyMaterialData);
			bufferInfo.usage = RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			bufferInfo.memoryProperties = RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT;

			dummyMaterialBuffer_ = rhi_->createBuffer(bufferInfo);
			if (dummyMaterialBuffer_)
			{
				DummyMaterialData materialData;
				void* data = rhi_->mapBuffer(dummyMaterialBuffer_);
				memcpy(data, &materialData, sizeof(DummyMaterialData));
				rhi_->unmapBuffer(dummyMaterialBuffer_);
				printLog("[ForwardPassRG]   ✅ Dummy material buffer created");
			}
		}

		// ========================================
		// Dummy 2D Texture (흰색 4x4) - ✅ 1x1 대신 4x4 사용
		// ========================================
		{
			RHIImageCreateInfo imageInfo{};
			imageInfo.width = 4;   // ✅ 4x4로 변경 (1x1은 1D로 인식됨)
			imageInfo.height = 4;  // ✅
			imageInfo.depth = 1;   // ✅ 명시적으로 설정
			imageInfo.format = RHI_FORMAT_R8G8B8A8_UNORM;
			imageInfo.usage = RHI_IMAGE_USAGE_SAMPLED_BIT | RHI_IMAGE_USAGE_TRANSFER_DST_BIT;
			imageInfo.samples = RHI_SAMPLE_COUNT_1_BIT;

			dummyTexture_ = rhi_->createImage(imageInfo);
			if (dummyTexture_)
			{
				RHIImageViewCreateInfo viewInfo{};
				viewInfo.viewType = RHI_IMAGE_VIEW_TYPE_2D;
				viewInfo.aspectMask = RHI_IMAGE_ASPECT_COLOR_BIT;

				dummyTextureView_ = rhi_->createImageView(dummyTexture_, viewInfo);
				printLog("[ForwardPassRG]   ✅ Dummy 2D texture created (4x4)");
			}
		}

		// ========================================
		// Dummy Cubemap (검은색 4x4x6) - ✅ 임시로 2D 텍스처로 대체
		// ========================================
		{
			RHIImageCreateInfo imageInfo{};
			imageInfo.width = 4;   // ✅ 4x4로 변경
			imageInfo.height = 4;  // ✅
			imageInfo.depth = 1;   // ✅ 명시적으로 설정
			// ❌ Cubemap은 나중에 구현 - 지금은 2D로 대체
			// imageInfo.arrayLayers = 6;  
			imageInfo.format = RHI_FORMAT_R8G8B8A8_UNORM;
			imageInfo.usage = RHI_IMAGE_USAGE_SAMPLED_BIT | RHI_IMAGE_USAGE_TRANSFER_DST_BIT;
			imageInfo.samples = RHI_SAMPLE_COUNT_1_BIT;

			dummyCubemap_ = rhi_->createImage(imageInfo);
			if (dummyCubemap_)
			{
				RHIImageViewCreateInfo viewInfo{};
				viewInfo.viewType = RHI_IMAGE_VIEW_TYPE_2D;  // ✅ 2D로 변경
				viewInfo.aspectMask = RHI_IMAGE_ASPECT_COLOR_BIT;

				dummyCubemapView_ = rhi_->createImageView(dummyCubemap_, viewInfo);
				printLog("[ForwardPassRG]   ✅ Dummy cubemap created (4x4 2D - TODO: make real cubemap)");
			}
		}

		// ========================================
		// Dummy Shadow Map (4x4 depth) - ✅ 1x1 대신 4x4 사용
		// ========================================
		{
			RHIImageCreateInfo imageInfo{};
			imageInfo.width = 4;   // ✅ 4x4로 변경
			imageInfo.height = 4;  // ✅
			imageInfo.depth = 1;   // ✅ 명시적으로 설정
			imageInfo.format = RHI_FORMAT_D32_SFLOAT;
			imageInfo.usage = RHI_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | RHI_IMAGE_USAGE_SAMPLED_BIT;
			imageInfo.samples = RHI_SAMPLE_COUNT_1_BIT;

			dummyShadowMap_ = rhi_->createImage(imageInfo);
			if (dummyShadowMap_)
			{
				RHIImageViewCreateInfo viewInfo{};
				viewInfo.viewType = RHI_IMAGE_VIEW_TYPE_2D;
				viewInfo.aspectMask = RHI_IMAGE_ASPECT_DEPTH_BIT;

				dummyShadowMapView_ = rhi_->createImageView(dummyShadowMap_, viewInfo);
				printLog("[ForwardPassRG]   ✅ Dummy shadow map created (4x4)");
			}
		}

		printLog("[ForwardPassRG] ✅ All dummy resources created");
	}

	void ForwardPassRG::destroyDummyResources()
	{
		printLog("[ForwardPassRG] Cleaning up dummy resources...");

		if (dummyShadowMapView_) rhi_->destroyImageView(dummyShadowMapView_);
		if (dummyShadowMap_) rhi_->destroyImage(dummyShadowMap_);
		if (dummyCubemapView_) rhi_->destroyImageView(dummyCubemapView_);
		if (dummyCubemap_) rhi_->destroyImage(dummyCubemap_);
		if (dummyTextureView_) rhi_->destroyImageView(dummyTextureView_);
		if (dummyTexture_) rhi_->destroyImage(dummyTexture_);
		if (dummySampler_) rhi_->destroySampler(dummySampler_);
		if (dummyMaterialBuffer_) rhi_->destroyBuffer(dummyMaterialBuffer_);

		dummyShadowMapView_ = nullptr;
		dummyShadowMap_ = nullptr;
		dummyCubemapView_ = nullptr;
		dummyCubemap_ = nullptr;
		dummyTextureView_ = nullptr;
		dummyTexture_ = nullptr;
		dummySampler_ = nullptr;
		dummyMaterialBuffer_ = nullptr;

		printLog("[ForwardPassRG] ✅ Dummy resources cleanup complete");
	}

} // namespace BinRenderer
