#include "RenderPassBase.h"

namespace BinRenderer
{
    // ========================================
    // RenderPassBase
// ========================================

    RenderPassBase::RenderPassBase(RHI* rhi, const std::string& name)
  : rhi_(rhi), name_(name)
    {
    }

    RenderPassBase::~RenderPassBase()
    {
    }

    void RenderPassBase::beginRenderPass(uint32_t frameIndex, const RHIClearValue* clearValues, uint32_t clearValueCount)
    {
     // TODO: RHI에 cmdBeginRenderPass 추가 필요
 // rhi_->cmdBeginRenderPass(renderPass_, framebuffer_, clearValues, clearValueCount);
    }

    void RenderPassBase::endRenderPass()
    {
   // TODO: RHI에 cmdEndRenderPass 추가 필요
// rhi_->cmdEndRenderPass();
  }

    // ========================================
  // RenderPassManager
    // ========================================

    RenderPassManager::RenderPassManager(RHI* rhi)
        : rhi_(rhi)
    {
    }

  RenderPassManager::~RenderPassManager()
    {
  renderPasses_.clear();
    }

    void RenderPassManager::addRenderPass(std::unique_ptr<RenderPassBase> renderPass)
    {
        renderPasses_.push_back(std::move(renderPass));
    }

    RenderPassBase* RenderPassManager::getRenderPass(const std::string& name)
    {
        for (auto& pass : renderPasses_)
        {
            if (pass->getName() == name)
            {
    return pass.get();
            }
        }
        return nullptr;
  }

    void RenderPassManager::executeAll(uint32_t frameIndex)
    {
        for (auto& pass : renderPasses_)
        {
         pass->execute(frameIndex);
        }
    }

    void RenderPassManager::resize(uint32_t width, uint32_t height)
    {
        for (auto& pass : renderPasses_)
        {
      pass->resize(width, height);
        }
  }

} // namespace BinRenderer
