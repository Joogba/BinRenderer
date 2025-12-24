#pragma once

#include "../IRenderPass.h"
#include "ClothSimulation.h"
#include "ClothMesh.h"
#include "ClothConfig.h"
#include "../ShaderManager.h"
#include "../Camera.h"

#include <memory>

namespace BinRenderer::Vulkan {

/**
 * @brief Cloth 시뮬레이션 및 렌더링을 담당하는 독립적인 렌더 패스
 * 
 * 이 클래스는 Application을 수정하지 않고도 Cloth 기능을 추가/제거할 수 있습니다.
 * Single Responsibility Principle: Cloth 관련 로직만 담당
 */
class ClothRenderPass : public IRenderPass
{
public:
    ClothRenderPass(ShaderManager& shaderManager, 
       const ClothConfig& config,
                    Camera& camera,
   VkFormat colorFormat,
    VkFormat depthFormat);
    
  ~ClothRenderPass() override;

    // IRenderPass 인터페이스 구현
    bool initialize(Context& ctx) override;
    void update(float deltaTime, uint32_t frameIndex) override;
    void render(VkCommandBuffer cmd, uint32_t frameIndex) override;
    void cleanup() override;
    const std::string& getName() const override { return name_; }

    // Cloth 전용 메서드
 ClothSimulation* getSimulation() { return simulation_.get(); }
    ClothMesh* getMesh() { return mesh_.get(); }

    // 설정 접근
    void setGravity(const glm::vec3& gravity);
    void setWind(const glm::vec3& wind, float strength);
    void setDamping(float damping);

private:
    std::string name_ = "ClothRenderPass";

    ShaderManager& shaderManager_;
    ClothConfig config_;
    Camera& camera_;
    VkFormat colorFormat_;
    VkFormat depthFormat_;

    std::unique_ptr<ClothSimulation> simulation_;
    std::unique_ptr<ClothMesh> mesh_;

    // Initialization state
    bool initialized_ = false;
};

} // namespace BinRenderer::Vulkan
