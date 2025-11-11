#pragma once

#include "../Context.h"
#include "../Pipeline.h"
#include "../ShaderManager.h"
#include "ClothSimulation.h"

#include <memory>
#include <vulkan/vulkan.h>

namespace BinRenderer::Vulkan {

// Forward declarations
class DescriptorSet;

// Cloth 렌더링을 위한 래퍼 클래스
class ClothMesh
{
  public:
    ClothMesh(Context& ctx, ShaderManager& shaderManager, ClothSimulation& simulation);
    ~ClothMesh();

    // 복사 금지
    ClothMesh(const ClothMesh&) = delete;
    ClothMesh& operator=(const ClothMesh&) = delete;

    // 초기화 및 정리
    void initialize(VkFormat colorFormat, VkFormat depthFormat, VkSampleCountFlagBits msaaSamples);
    void cleanup();

    // 렌더링
    void render(VkCommandBuffer cmd, uint32_t frameIndex, const glm::mat4& viewProjection, 
      const glm::vec3& cameraPos);

    // Transform
    void setModelMatrix(const glm::mat4& model) { modelMatrix_ = model; }
    const glm::mat4& getModelMatrix() const { return modelMatrix_; }

  private:
    void createRenderPipeline(VkFormat colorFormat, VkFormat depthFormat, 
     VkSampleCountFlagBits msaaSamples);
    void createDescriptorSets();

  private:
    Context& ctx_;
    ShaderManager& shaderManager_;
    ClothSimulation& simulation_;

    glm::mat4 modelMatrix_{1.0f};

    // 렌더링 파이프라인
    std::unique_ptr<Pipeline> renderPipeline_;

    // Descriptor Sets (per frame)
    std::vector<DescriptorSet> descriptorSets_;

  // Scene Uniform Buffer (shared with other objects - 임시로 여기서 생성)
    struct SceneUBO {
        glm::mat4 viewProjection;
     glm::vec3 cameraPos;
        float time;
    };
    SceneUBO sceneData_;
    std::vector<std::unique_ptr<MappedBuffer>> sceneBuffers_; // Per frame
};

} // namespace BinRenderer::Vulkan
