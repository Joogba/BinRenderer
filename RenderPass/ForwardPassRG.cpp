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
		
		// 1. Descriptor Sets 생성
		createDescriptorSets();
		
		// 2. 파이프라인 생성 (PBR 셰이더 사용)
		createPipeline();
		
		printLog("[ForwardPassRG] Initialized successfully");
		return true;
	}

	void ForwardPassRG::shutdown()
	{
		destroyDescriptorSets();
		destroyPipeline();
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

		// ✅ Dynamic Rendering 시작
		rhi->cmdBeginRendering(1280, 720, swapchainImageView, nullptr);
		
		// Viewport 및 Scissor 설정
		RHIViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = 1280.0f;
		viewport.height = 720.0f;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		rhi->cmdSetViewport(viewport);

		RHIRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = {1280, 720};
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

		// ✅ Descriptor Sets 바인딩 (Set 0: Scene UBO)
		if (!sceneDescriptorSets_.empty() && pipeline_)
		{
			uint32_t currentFrame = frameIndex % sceneDescriptorSets_.size();
			RHIDescriptorSet* descriptorSets[] = { sceneDescriptorSets_[currentFrame] };
			
			// ✅ RHI API 사용하여 descriptor sets 바인딩
			rhi->cmdBindDescriptorSets(pipeline_, 0, descriptorSets, 1);
			
			if (frameIndex % 60 == 0)
			{
				printLog("[ForwardPassRG]   ✅ Descriptor set bound (Set 0, frame: {})", currentFrame);
			}
		}

		// ========================================
		// Renderer 책임: 실제 렌더링 로직
		// ========================================
		
		if (scene_ && renderer_)
		{
			renderer_->renderForwardModels(rhi, *scene_, pipeline_, frameIndex);
			
			if (frameIndex % 60 == 0)
			{
				printLog("[ForwardPassRG]   - Models rendered via Renderer");
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

		// ✅ PBR Forward 셰이더 사용
		auto vertCode = readShaderFile("../../assets/shaders/pbrForward.vert.spv");
		if (vertCode.empty())
		{
			printLog("[ForwardPassRG] ❌ Failed to read PBR vertex shader file");
			return;
		}

		RHIShaderCreateInfo vertShaderInfo{};
		vertShaderInfo.stage = RHI_SHADER_STAGE_VERTEX_BIT;
		vertShaderInfo.name = "pbrForward.vert";
		vertShaderInfo.entryPoint = "main";
		vertShaderInfo.code = std::move(vertCode);

		vertexShader_ = rhi_->createShader(vertShaderInfo);
		if (!vertexShader_)
		{
			printLog("[ForwardPassRG] ❌ Failed to create vertex shader");
			return;
		}
		printLog("[ForwardPassRG]   - PBR Vertex shader created");

		// Fragment Shader 로드
		auto fragCode = readShaderFile("../../assets/shaders/pbrForward.frag.spv");
		if (fragCode.empty())
		{
			printLog("[ForwardPassRG] ❌ Failed to read PBR fragment shader file");
			return;
		}

		RHIShaderCreateInfo fragShaderInfo{};
		fragShaderInfo.stage = RHI_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderInfo.name = "pbrForward.frag";
		fragShaderInfo.entryPoint = "main";
		fragShaderInfo.code = std::move(fragCode);

		fragmentShader_ = rhi_->createShader(fragShaderInfo);
		if (!fragmentShader_)
		{
			printLog("[ForwardPassRG] ❌ Failed to create fragment shader");
			return;
		}
		printLog("[ForwardPassRG]   - PBR Fragment shader created");

		// Pipeline 생성
		RHIPipelineCreateInfo pipelineInfo{};
		
		// Shader stages 설정
		pipelineInfo.shaderStages.push_back(vertexShader_);
		pipelineInfo.shaderStages.push_back(fragmentShader_);
		
		// ✅ Descriptor Set Layouts (PBR 셰이더용)
		if (sceneDescriptorLayout_)
		{
			pipelineInfo.descriptorSetLayouts.push_back(sceneDescriptorLayout_);
		}
		// TODO: Material, IBL, Shadow layouts 추가
		
		// ✅ Push Constants (PBR 셰이더용)
		RHIPipelineCreateInfo::PushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = RHI_SHADER_STAGE_VERTEX_BIT | RHI_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PbrPushConstants);
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
		
		// Location 0: position (vec3) - vec4로 패딩
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
		pipelineInfo.depthStencilState.depthTestEnable = true;
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
		// Descriptor Pool 생성
		// ========================================
		{
			RHIDescriptorPoolCreateInfo poolInfo{};
			poolInfo.maxSets = 10; // 여유있게 할당
			
			RHIDescriptorPoolSize uniformPoolSize{};
			uniformPoolSize.type = RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uniformPoolSize.descriptorCount = 30; // 3 bindings * 10 sets
			poolInfo.poolSizes.push_back(uniformPoolSize);
			
			// TODO: Material textures, IBL, Shadow map pool sizes 추가
			
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

		// TODO: Set 1 (Material), Set 2 (IBL), Set 3 (Shadow) 추가
		
		printLog("[ForwardPassRG] ✅ Descriptor sets created successfully");
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

} // namespace BinRenderer
