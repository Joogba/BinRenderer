#include "ClothSimulation.h"
#include "../Logger.h"
#include "../MappedBuffer.h"
#include "../StorageBuffer.h"
#include "../Pipeline.h"
#include "../DescriptorSet.h"
#include "../PipelineConfig.h"

#include <glm/glm.hpp>
#include <algorithm>

namespace BinRenderer::Vulkan {

	ClothSimulation::ClothSimulation(Context& ctx, ShaderManager& shaderManager,
		const ClothConfig& config)
		: ctx_(ctx), shaderManager_(shaderManager), config_(config)
	{
	}

	ClothSimulation::~ClothSimulation()
	{
		cleanup();
	}

	void ClothSimulation::initialize()
	{
		BinRenderer::printLog("Initializing cloth simulation...");
		BinRenderer::printLog("Grid size: {}x{}", config_.getGridWidth(), config_.getGridHeight());
		BinRenderer::printLog("Particle count: {}", getParticleCount());

		initializeParticles();
		initializeConstraints();
		createIndices();
		createBuffers();
		createComputePipelines();

		// 초기화 검증
		if (!positionBuffer_ || !indexBuffer_) {
			BinRenderer::printLog("[ERROR] Cloth buffers not created properly!");
			return;
		}

		BinRenderer::printLog("Cloth simulation initialized successfully");
		BinRenderer::printLog(" - Position buffer: {}", positionBuffer_->buffer() != VK_NULL_HANDLE ? "OK" : "FAILED");
		BinRenderer::printLog(" - Index buffer: {}", indexBuffer_->buffer() != VK_NULL_HANDLE ? "OK" : "FAILED");
		BinRenderer::printLog(" - Index count: {}", indexCount_);
	}

	void ClothSimulation::cleanup()
	{
		positionBuffer_.reset();
		constraintBuffer_.reset();
		paramsBuffer_.reset();
		indexBuffer_.reset();

		integratePass_.reset();
		constraintPass_.reset();
		normalPass_.reset();
	}

	void ClothSimulation::initializeParticles()
	{
		particles_.clear();
		particles_.reserve(getParticleCount());

		const uint32_t width = config_.getGridWidth();
		const uint32_t height = config_.getGridHeight();
		const float spacing = config_.getSpacing();
		const float mass = config_.getMass();
		const float invMass = 1.0f / mass;

		// 그리드 중심을 원점에 배치
		const float offsetX = -(static_cast<float>(width) - 1.0f) * spacing * 0.5f;
		const float offsetZ = -(static_cast<float>(height) - 1.0f) * spacing * 0.5f;

		for (uint32_t y = 0; y < height; ++y) {
			for (uint32_t x = 0; x < width; ++x) {
				ClothParticle particle;
				particle.position = glm::vec4(offsetX + x * spacing, 0.0f, offsetZ + y * spacing, invMass);
				particle.velocity = glm::vec4(0.0f);
				particle.normal = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

				// 상단 모서리 고정 (핀)
				if (config_.isPinnedCorners() && y == 0 && (x == 0 || x == width - 1)) {
					particle.position.w = 0.0f; // invMass = 0 -> 무한 질량 (고정)
				}

				particles_.push_back(particle);
			}
		}

		BinRenderer::printLog("Initialized {} particles", particles_.size());
	}

	void ClothSimulation::initializeConstraints()
	{
		constraints_.clear();

		addStructuralConstraints(constraints_);
		addShearConstraints(constraints_);
		addBendConstraints(constraints_);

		BinRenderer::printLog("Created {} constraints", constraints_.size());
	}

	void ClothSimulation::addStructuralConstraints(std::vector<ClothConstraint>& constraints)
	{
		const uint32_t width = config_.getGridWidth();
		const uint32_t height = config_.getGridHeight();
		const float spacing = config_.getSpacing();
		const float stiffness = config_.getStiffness();

		// 수평 스프링
		for (uint32_t y = 0; y < height; ++y) {
			for (uint32_t x = 0; x < width - 1; ++x) {
				uint32_t idx1 = getParticleIndex(x, y);
				uint32_t idx2 = getParticleIndex(x + 1, y);
				constraints.push_back({ idx1, idx2, spacing, stiffness });
			}
		}

		// 수직 스프링
		for (uint32_t y = 0; y < height - 1; ++y) {
			for (uint32_t x = 0; x < width; ++x) {
				uint32_t idx1 = getParticleIndex(x, y);
				uint32_t idx2 = getParticleIndex(x, y + 1);
				constraints.push_back({ idx1, idx2, spacing, stiffness });
			}
		}
	}

	void ClothSimulation::addShearConstraints(std::vector<ClothConstraint>& constraints)
	{
		const uint32_t width = config_.getGridWidth();
		const uint32_t height = config_.getGridHeight();
		const float spacing = config_.getSpacing();
		const float diagonalLength = spacing * std::sqrt(2.0f);
		const float stiffness = config_.getStiffness() * 0.7f; // 전단 스프링은 약간 약하게

		// 대각선 스프링 (\)
		for (uint32_t y = 0; y < height - 1; ++y) {
			for (uint32_t x = 0; x < width - 1; ++x) {
				uint32_t idx1 = getParticleIndex(x, y);
				uint32_t idx2 = getParticleIndex(x + 1, y + 1);
				constraints.push_back({ idx1, idx2, diagonalLength, stiffness });
			}
		}

		// 대각선 스프링 (/)
		for (uint32_t y = 0; y < height - 1; ++y) {
			for (uint32_t x = 1; x < width; ++x) {
				uint32_t idx1 = getParticleIndex(x, y);
				uint32_t idx2 = getParticleIndex(x - 1, y + 1);
				constraints.push_back({ idx1, idx2, diagonalLength, stiffness });
			}
		}
	}

	void ClothSimulation::addBendConstraints(std::vector<ClothConstraint>& constraints)
	{
		const uint32_t width = config_.getGridWidth();
		const uint32_t height = config_.getGridHeight();
		const float spacing = config_.getSpacing();
		const float bendLength = spacing * 2.0f;
		const float stiffness = config_.getStiffness() * 0.5f; // 굽힘 스프링은 더 약하게

		// 수평 굽힘 스프링
		for (uint32_t y = 0; y < height; ++y) {
			for (uint32_t x = 0; x < width - 2; ++x) {
				uint32_t idx1 = getParticleIndex(x, y);
				uint32_t idx2 = getParticleIndex(x + 2, y);
				constraints.push_back({ idx1, idx2, bendLength, stiffness });
			}
		}

		// 수직 굽힘 스프링
		for (uint32_t y = 0; y < height - 2; ++y) {
			for (uint32_t x = 0; x < width; ++x) {
				uint32_t idx1 = getParticleIndex(x, y);
				uint32_t idx2 = getParticleIndex(x, y + 2);
				constraints.push_back({ idx1, idx2, bendLength, stiffness });
			}
		}
	}

	void ClothSimulation::createIndices()
	{
		std::vector<uint32_t> indices;
		const uint32_t width = config_.getGridWidth();
		const uint32_t height = config_.getGridHeight();

		BinRenderer::printLog("Creating indices for {}x{} grid...", width, height);

		// 삼각형 메쉬 인덱스 생성
		for (uint32_t y = 0; y < height - 1; ++y) {
			for (uint32_t x = 0; x < width - 1; ++x) {
				uint32_t topLeft = getParticleIndex(x, y);
				uint32_t topRight = getParticleIndex(x + 1, y);
				uint32_t bottomLeft = getParticleIndex(x, y + 1);
				uint32_t bottomRight = getParticleIndex(x + 1, y + 1);

				// 첫 번째 삼각형
				indices.push_back(topLeft);
				indices.push_back(bottomLeft);
				indices.push_back(topRight);

				// 두 번째 삼각형
				indices.push_back(topRight);
				indices.push_back(bottomLeft);
				indices.push_back(bottomRight);
			}
		}

		indexCount_ = static_cast<uint32_t>(indices.size());
		BinRenderer::printLog("Generated {} indices ({} triangles)", indexCount_, indexCount_ / 3);

		// 인덱스 버퍼 생성 - INDEX_BUFFER_BIT 포함
		VkDeviceSize bufferSize = indices.size() * sizeof(uint32_t);
		indexBuffer_ = std::make_unique<StorageBuffer>(
			ctx_, 
			indices.data(), 
			bufferSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT  // ✅ 인덱스 버퍼로 사용 가능
		);
		
		BinRenderer::printLog("Created index buffer successfully (size: {} bytes)", bufferSize);
	}

	void ClothSimulation::createBuffers()
	{
		BinRenderer::printLog("Creating GPU buffers...");
		BinRenderer::printLog(" - Particles vector size: {}", particles_.size());
		BinRenderer::printLog(" - Constraints vector size: {}", constraints_.size());
		
		// Particle 버퍼 생성 (Position + Velocity + Normal 통합)
		BinRenderer::printLog("Creating particle buffer ({} bytes)...", 
			particles_.size() * sizeof(ClothParticle));
		
		positionBuffer_ = std::make_unique<StorageBuffer>(
			ctx_, particles_.data(), particles_.size() * sizeof(ClothParticle));

		BinRenderer::printLog("Creating constraint buffer ({} bytes)...", 
			constraints_.size() * sizeof(ClothConstraint));
		
		// Constraint 버퍼 생성
		constraintBuffer_ = std::make_unique<StorageBuffer>(
			ctx_, constraints_.data(), constraints_.size() * sizeof(ClothConstraint));

		// Simulation parameters 초기화
		simParams_.gravity = glm::vec4(config_.getGravity(), 0.0f);
		simParams_.wind = glm::vec4(1.0f, 0.0f, 0.0f, config_.getWindSpeed());
		simParams_.deltaTime = fixedTimeStep_;
		simParams_.damping = config_.getDamping();
		simParams_.constraintIters = config_.getConstraintIterations();
		simParams_.particleCount = getParticleCount();
		simParams_.friction = config_.getFriction();

		BinRenderer::printLog("Creating params buffer...");
		
		// Params 버퍼 생성 (MappedBuffer 사용 - CPU에서 자주 업데이트)
		paramsBuffer_ = std::make_unique<MappedBuffer>(ctx_);
		paramsBuffer_->createUniformBuffer(simParams_);

		BinRenderer::printLog("Created GPU buffers successfully");
	}

	void ClothSimulation::createComputePipelines()
	{
		BinRenderer::printLog("Creating cloth compute pipelines...");

		const uint32_t frameCount = 2; // kMaxFramesInFlight

		// 1. Integrate Pass 파이프라인
		{
			PipelineConfig config = PipelineConfig::createCompute();
			config.name = "cloth_integrate";

			integratePass_ = std::make_unique<Pipeline>(ctx_, shaderManager_, config);

			// Descriptor Sets 생성 (각 프레임당)
			integrateDescriptorSets_.resize(frameCount);
			for (uint32_t i = 0; i < frameCount; ++i) {
				std::vector<std::reference_wrapper<Resource>> resources = {
					*positionBuffer_,  // binding 0: ParticleBuffer
					*paramsBuffer_     // binding 1: ParamsBuffer
				};

				integrateDescriptorSets_[i].create(
					ctx_, 
					integratePass_->layouts()[0], 
					resources
				);
			}

			BinRenderer::printLog(" - Created cloth_integrate pipeline");
		}

		// 2. Constraint Pass 파이프라인
		{
			PipelineConfig config = PipelineConfig::createCompute();
			config.name = "cloth_constraints";

			constraintPass_ = std::make_unique<Pipeline>(ctx_, shaderManager_, config);

			// Descriptor Sets 생성
			constraintDescriptorSets_.resize(frameCount);
			for (uint32_t i = 0; i < frameCount; ++i) {
				std::vector<std::reference_wrapper<Resource>> resources = {
					*positionBuffer_,      // binding 0: ParticleBuffer
					*constraintBuffer_,    // binding 1: ConstraintBuffer
					*paramsBuffer_         // binding 2: ParamsBuffer
				};

				constraintDescriptorSets_[i].create(
					ctx_,
					constraintPass_->layouts()[0],
					resources
				);
			}

			BinRenderer::printLog(" - Created cloth_constraints pipeline");
		}

		// 3. Normal Pass 파이프라인
		{
			PipelineConfig config = PipelineConfig::createCompute();
			config.name = "cloth_normals";

			normalPass_ = std::make_unique<Pipeline>(ctx_, shaderManager_, config);

			// Descriptor Sets 생성
			normalDescriptorSets_.resize(frameCount);
			for (uint32_t i = 0; i < frameCount; ++i) {
				std::vector<std::reference_wrapper<Resource>> resources = {
					*positionBuffer_,  // binding 0: ParticleBuffer
					*paramsBuffer_     // binding 1: ParamsBuffer
				};

				normalDescriptorSets_[i].create(
					ctx_,
					normalPass_->layouts()[0],
					resources
				);
			}

			BinRenderer::printLog(" - Created cloth_normals pipeline");
		}

		BinRenderer::printLog("Cloth compute pipelines created successfully!");
	}
	
	void ClothSimulation::update(float deltaTime)
	{
		accumulatedTime_ += deltaTime;
	}

	void ClothSimulation::simulate(VkCommandBuffer cmd, uint32_t frameIndex)
	{
		// 고정 타임스텝으로 시뮬레이션
		while (accumulatedTime_ >= fixedTimeStep_) {
			// 파라미터 버퍼 업데이트 (deltaTime 등)
			simParams_.deltaTime = fixedTimeStep_;
			paramsBuffer_->updateFromCpuData();

			const uint32_t particleCount = getParticleCount();
			const uint32_t constraintCount = static_cast<uint32_t>(constraints_.size());
			const uint32_t workGroupSize = 256;

			// 1. Integrate Pass: 위치/속도 업데이트
			{
				vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, integratePass_->pipeline());

				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
					integratePass_->pipelineLayout(), 0, 1,
					&integrateDescriptorSets_[frameIndex].handle(), 0, nullptr);

				uint32_t groupCount = (particleCount + workGroupSize - 1) / workGroupSize;
				vkCmdDispatch(cmd, groupCount, 1, 1);

				// Memory barrier
				VkMemoryBarrier barrier{ VK_STRUCTURE_TYPE_MEMORY_BARRIER };
				barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

				vkCmdPipelineBarrier(cmd,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					0, 1, &barrier, 0, nullptr, 0, nullptr);
			}

			// 2. Constraint Pass: 제약 조건 해결 (여러 번 반복)
			{
				vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, constraintPass_->pipeline());

				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
					constraintPass_->pipelineLayout(), 0, 1,
					&constraintDescriptorSets_[frameIndex].handle(), 0, nullptr);

				for (uint32_t iter = 0; iter < config_.getConstraintIterations(); ++iter)
				{
					// Push constants: constraintCount
					vkCmdPushConstants(cmd, constraintPass_->pipelineLayout(),
						VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &constraintCount);

					uint32_t groupCount = (constraintCount + workGroupSize - 1) / workGroupSize;
					vkCmdDispatch(cmd, groupCount, 1, 1);

					// Memory barrier (파티클 위치가 업데이트되었으므로)
					VkMemoryBarrier barrier{ VK_STRUCTURE_TYPE_MEMORY_BARRIER };
					barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
					barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

					vkCmdPipelineBarrier(cmd,
						VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
						VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
						0, 1, &barrier, 0, nullptr, 0, nullptr);
				}
			}

			// 3. Normal Pass: 노말 재계산
			{
				vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, normalPass_->pipeline());

				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
					normalPass_->pipelineLayout(), 0, 1,
					&normalDescriptorSets_[frameIndex].handle(), 0, nullptr);

				// Push constants: gridWidth, gridHeight
				struct PushConstants {
					uint32_t gridWidth;
					uint32_t gridHeight;
				} pushConsts;

				pushConsts.gridWidth = config_.getGridWidth();
				pushConsts.gridHeight = config_.getGridHeight();

				vkCmdPushConstants(cmd, normalPass_->pipelineLayout(),
					VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants), &pushConsts);

				uint32_t groupCount = (particleCount + workGroupSize - 1) / workGroupSize;
				vkCmdDispatch(cmd, groupCount, 1, 1);

				// Final memory barrier
				VkMemoryBarrier barrier{ VK_STRUCTURE_TYPE_MEMORY_BARRIER };
				barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				vkCmdPipelineBarrier(cmd,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
					0, 1, &barrier, 0, nullptr, 0, nullptr);
			}

			accumulatedTime_ -= fixedTimeStep_;
		}
	}
	
	uint32_t ClothSimulation::getParticleIndex(uint32_t x, uint32_t y) const
	{
		return y * config_.getGridWidth() + x;
	}

	void ClothSimulation::setGravity(const glm::vec3& gravity)
	{
		config_.setGravity(gravity);
		simParams_.gravity = glm::vec4(gravity, 0.0f);
	}

	void ClothSimulation::setWind(const glm::vec3& wind, float strength)
	{
		simParams_.wind = glm::vec4(wind, strength);
	}

	void ClothSimulation::setDamping(float damping)
	{
		config_.setDamping(damping);
		simParams_.damping = damping;
	}

} // namespace BinRenderer::Vulkan
