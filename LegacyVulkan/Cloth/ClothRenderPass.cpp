#include "ClothRenderPass.h"
#include "../Logger.h"

namespace BinRenderer::Vulkan {

ClothRenderPass::ClothRenderPass(ShaderManager& shaderManager,
         const ClothConfig& config,
     Camera& camera,
          VkFormat colorFormat,
  VkFormat depthFormat)
    : shaderManager_(shaderManager)
    , config_(config)
    , camera_(camera)
    , colorFormat_(colorFormat)
    , depthFormat_(depthFormat)
{
}

ClothRenderPass::~ClothRenderPass()
{
    cleanup();
}

bool ClothRenderPass::initialize(Context& ctx)
{
    if (initialized_) {
        BinRenderer::printLog("[ClothRenderPass] Already initialized");
    return true;
    }

    BinRenderer::printLog("[ClothRenderPass] Initializing...");

    try {
        // 1. ClothSimulation 생성 및 초기화
        simulation_ = std::make_unique<ClothSimulation>(ctx, shaderManager_, config_);
   simulation_->initialize();

        // 2. ClothMesh 생성 및 초기화
   mesh_ = std::make_unique<ClothMesh>(ctx, shaderManager_, *simulation_);
        mesh_->initialize(colorFormat_, depthFormat_, VK_SAMPLE_COUNT_1_BIT);

     initialized_ = true;
        BinRenderer::printLog("[ClothRenderPass] Initialization complete");
        return true;
    }
    catch (const std::exception& e) {
   BinRenderer::printLog("[ClothRenderPass] Initialization failed: {}", e.what());
        return false;
    }
}

void ClothRenderPass::update(float deltaTime, uint32_t frameIndex)
{
    if (!initialized_ || !isEnabled()) {
     return;
    }

    // Cloth 시뮬레이션 업데이트 (CPU 측)
    simulation_->update(deltaTime);
}

void ClothRenderPass::render(VkCommandBuffer cmd, uint32_t frameIndex)
{
    if (!initialized_ || !isEnabled()) {
      return;
    }

    // 1. Cloth 시뮬레이션 실행 (GPU Compute)
    simulation_->simulate(cmd, frameIndex);

    // 2. Cloth 렌더링 (Graphics Pipeline)
  glm::mat4 viewProjection = camera_.matrices.perspective * camera_.matrices.view;
    mesh_->render(cmd, frameIndex, viewProjection, camera_.position);
}

void ClothRenderPass::cleanup()
{
    if (initialized_) {
        BinRenderer::printLog("[ClothRenderPass] Cleaning up...");

   mesh_.reset();
        simulation_.reset();

   initialized_ = false;
    }
}

void ClothRenderPass::setGravity(const glm::vec3& gravity)
{
    if (simulation_) {
    simulation_->setGravity(gravity);
  }
}

void ClothRenderPass::setWind(const glm::vec3& wind, float strength)
{
    if (simulation_) {
        simulation_->setWind(wind, strength);
    }
}

void ClothRenderPass::setDamping(float damping)
{
    if (simulation_) {
      simulation_->setDamping(damping);
  }
}

} // namespace BinRenderer::Vulkan
