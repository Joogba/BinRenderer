#include "ShadowPass.h"

namespace BinRenderer
{
    ShadowPass::ShadowPass(RHI* rhi)
   : RenderPassBase(rhi, "ShadowPass")
    {
    }

 ShadowPass::~ShadowPass()
    {
   shutdown();
    }

    bool ShadowPass::initialize()
    {
        createShadowMaps();
   createRenderPass();
        createPipeline();

   return true;
    }

    void ShadowPass::shutdown()
    {
if (pipeline_)
        {
 rhi_->destroyPipeline(pipeline_);
       pipeline_ = nullptr;
        }

   destroyShadowMaps();
    }

    void ShadowPass::resize(uint32_t width, uint32_t height)
    {
        // 그림자 맵은 화면 크기와 독립적
   // 필요시 shadowMapResolution_ 변경
    }

    void ShadowPass::execute(uint32_t frameIndex)
 {
   // 각 Cascade별로 그림자 맵 렌더링
 for (uint32_t i = 0; i < cascadeCount_; ++i)
 {
   RHIClearValue clearValue{};
       clearValue.depthStencil.depth = 1.0f;
     clearValue.depthStencil.stencil = 0;

     // TODO: Cascade별 Framebuffer 바인딩
   // beginRenderPass(frameIndex, &clearValue, 1);

       // TODO: 그림자 투사 물체 렌더링
       // rhi_->cmdBindPipeline(pipeline_);
            // ... draw shadow casters ...

      // endRenderPass();
   }
    }

    RHIImage* ShadowPass::getShadowMap(uint32_t cascadeIndex) const
    {
     if (cascadeIndex < shadowMaps_.size())
  {
     return shadowMaps_[cascadeIndex];
   }
        return nullptr;
    }

    void ShadowPass::setCascadeCount(uint32_t count)
    {
   if (cascadeCount_ == count)
   {
       return;
 }

    cascadeCount_ = count;
destroyShadowMaps();
        createShadowMaps();
    }

    void ShadowPass::setShadowMapResolution(uint32_t resolution)
 {
   if (shadowMapResolution_ == resolution)
 {
       return;
   }

     shadowMapResolution_ = resolution;
 destroyShadowMaps();
        createShadowMaps();
    }

void ShadowPass::createShadowMaps()
    {
   shadowMaps_.resize(cascadeCount_);
shadowMapViews_.resize(cascadeCount_);

      for (uint32_t i = 0; i < cascadeCount_; ++i)
 {
          // Shadow Map (Depth Only)
RHIImageCreateInfo shadowMapInfo{};
     shadowMapInfo.width = shadowMapResolution_;
shadowMapInfo.height = shadowMapResolution_;
     shadowMapInfo.depth = 1;
       shadowMapInfo.mipLevels = 1;
  shadowMapInfo.arrayLayers = 1;
     shadowMapInfo.format = RHI_FORMAT_D32_SFLOAT;
shadowMapInfo.tiling = RHI_IMAGE_TILING_OPTIMAL;
      shadowMapInfo.usage = RHI_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | RHI_IMAGE_USAGE_SAMPLED_BIT;
  shadowMapInfo.samples = RHI_SAMPLE_COUNT_1_BIT;
       shadowMaps_[i] = rhi_->createImage(shadowMapInfo);

    // TODO: Image View 생성
 // TODO: Framebuffer 생성
  }
    }

    void ShadowPass::destroyShadowMaps()
    {
   for (auto* shadowMap : shadowMaps_)
    {
       if (shadowMap)
  {
  rhi_->destroyImage(shadowMap);
     }
        }
shadowMaps_.clear();
   shadowMapViews_.clear();
    }

    void ShadowPass::createRenderPass()
    {
   // TODO: Depth-only RenderPass 생성
    }

    void ShadowPass::createPipeline()
  {
        // TODO: Shadow Map용 파이프라인 생성
        // Vertex Shader: Shadow MVP Transform
        // Fragment Shader: Empty (depth only)
    }

} // namespace BinRenderer
