#include "RenderPassManager.h"
#include "Logger.h"
#include <algorithm>

namespace BinRenderer::Vulkan {

void RenderPassManager::addRenderPass(std::unique_ptr<IRenderPass> renderPass, int priority)
{
    if (!renderPass) {
     BinRenderer::printLog("[RenderPassManager] Warning: Null render pass");
        return;
    }

    BinRenderer::printLog("[RenderPassManager] Adding render pass: {} (priority: {})",
        renderPass->getName(), priority);

    renderPasses_.push_back({std::move(renderPass), priority});
    needsSort_ = true;
}

IRenderPass* RenderPassManager::getRenderPass(const std::string& name)
{
    for (auto& entry : renderPasses_) {
   if (entry.pass->getName() == name) {
            return entry.pass.get();
 }
    }
    return nullptr;
}

bool RenderPassManager::initializeAll(Context& ctx)
{
    BinRenderer::printLog("[RenderPassManager] Initializing {} render passes...",
      renderPasses_.size());

    sortRenderPasses();

    for (auto& entry : renderPasses_) {
        BinRenderer::printLog("[RenderPassManager] Initializing: {}", entry.pass->getName());
   
      if (!entry.pass->initialize(ctx)) {
      BinRenderer::printLog("[RenderPassManager] Failed to initialize: {}",
  entry.pass->getName());
          return false;
        }
    }

    BinRenderer::printLog("[RenderPassManager] All render passes initialized successfully");
    return true;
}

void RenderPassManager::updateAll(float deltaTime, uint32_t frameIndex)
{
  for (auto& entry : renderPasses_) {
        if (entry.pass->isEnabled()) {
   entry.pass->update(deltaTime, frameIndex);
        }
    }
}

void RenderPassManager::renderAll(VkCommandBuffer cmd, uint32_t frameIndex)
{
    for (auto& entry : renderPasses_) {
   if (entry.pass->isEnabled()) {
     entry.pass->render(cmd, frameIndex);
        }
    }
}

void RenderPassManager::cleanupAll()
{
    BinRenderer::printLog("[RenderPassManager] Cleaning up render passes...");

    for (auto& entry : renderPasses_) {
entry.pass->cleanup();
    }

    renderPasses_.clear();
}

void RenderPassManager::sortRenderPasses()
{
    if (needsSort_) {
  std::sort(renderPasses_.begin(), renderPasses_.end());
        needsSort_ = false;

        BinRenderer::printLog("[RenderPassManager] Render pass execution order:");
for (const auto& entry : renderPasses_) {
 BinRenderer::printLog("  - {} (priority: {})",
   entry.pass->getName(), entry.priority);
        }
    }
}

} // namespace BinRenderer::Vulkan
