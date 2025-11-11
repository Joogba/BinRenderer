#pragma once

#include "ClothConfig.h"
#include "../Context.h"
#include "../ShaderManager.h"

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace BinRenderer::Vulkan {

// Forward declarations
class StorageBuffer;
class MappedBuffer;
class Pipeline;
class DescriptorSet;

class ClothSimulation
{
  public:
    ClothSimulation(Context& ctx, ShaderManager& shaderManager, const ClothConfig& config);
    ~ClothSimulation();

    // 복사 금지
    ClothSimulation(const ClothSimulation&) = delete;
    ClothSimulation& operator=(const ClothSimulation&) = delete;

    // 초기화 및 정리
    void initialize();
    void cleanup();

    // 시뮬레이션 업데이트
    void update(float deltaTime);
    void simulate(VkCommandBuffer cmd, uint32_t frameIndex);

    // 설정 접근
    const ClothConfig& getConfig() const { return config_; }
    ClothConfig& getConfig() { return config_; }

    // 버퍼 접근 (렌더링용)
    StorageBuffer* getPositionBuffer() { return positionBuffer_.get(); }
    StorageBuffer* getIndexBuffer() { return indexBuffer_.get(); }

    uint32_t getParticleCount() const { return config_.getParticleCount(); }
    uint32_t getIndexCount() const { return indexCount_; }

    // 런타임 파라미터 조정
    void setGravity(const glm::vec3& gravity);
    void setWind(const glm::vec3& wind, float strength);
    void setDamping(float damping);

  private:
    // 초기화 헬퍼
    void initializeParticles();
    void initializeConstraints();
    void createBuffers();
    void createComputePipelines();
    void createIndices();

    // 제약 조건 생성
    void addStructuralConstraints(std::vector<ClothConstraint>& constraints);
    void addShearConstraints(std::vector<ClothConstraint>& constraints);
    void addBendConstraints(std::vector<ClothConstraint>& constraints);

    // 인덱스 계산 헬퍼
    uint32_t getParticleIndex(uint32_t x, uint32_t y) const;

  private:
    Context& ctx_;
    ShaderManager& shaderManager_;
    ClothConfig config_;

    // GPU 버퍼
    std::unique_ptr<StorageBuffer> positionBuffer_;      // ClothParticle (전체 구조체)
    std::unique_ptr<StorageBuffer> constraintBuffer_;    // ClothConstraints
    std::unique_ptr<MappedBuffer> paramsBuffer_;         // ClothSimParams
    std::unique_ptr<StorageBuffer> indexBuffer_;         // 렌더링용 인덱스

    // CPU 데이터
    std::vector<ClothParticle> particles_;
    std::vector<ClothConstraint> constraints_;
    ClothSimParams simParams_;
    uint32_t indexCount_{0};

    // Compute 파이프라인
    std::unique_ptr<Pipeline> integratePass_;          // 적분 (위치/속도 업데이트)
    std::unique_ptr<Pipeline> constraintPass_;         // 제약 조건 해결
    std::unique_ptr<Pipeline> normalPass_;      // 노말 재계산

    // Descriptor Sets
    std::vector<DescriptorSet> integrateDescriptorSets_;      // Per frame
    std::vector<DescriptorSet> constraintDescriptorSets_;     // Per frame
    std::vector<DescriptorSet> normalDescriptorSets_;         // Per frame

    // 시간 관리
    float accumulatedTime_{0.0f};
    const float fixedTimeStep_{1.0f / 60.0f};     // 60 FPS 고정
};

} // namespace BinRenderer::Vulkan
