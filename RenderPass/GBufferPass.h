#pragma once

#include "RenderPassBase.h"

namespace BinRenderer
{
    /**
     * @brief G-Buffer 렌더 패스 (Deferred Rendering 1단계)
  * 
  * Albedo, Normal, Position, Metallic-Roughness 등을 렌더 타겟에 기록
  */
    class GBufferPass : public RenderPassBase
    {
    public:
   GBufferPass(RHI* rhi);
        ~GBufferPass() override;

  bool initialize() override;
   void shutdown() override;
        void resize(uint32_t width, uint32_t height) override;
   void execute(uint32_t frameIndex) override;

    // G-Buffer 텍스처 접근
        RHIImage* getAlbedoTexture() const { return albedoImage_; }
 RHIImage* getNormalTexture() const { return normalImage_; }
        RHIImage* getPositionTexture() const { return positionImage_; }
  RHIImage* getMetallicRoughnessTexture() const { return metallicRoughnessImage_; }
   RHIImage* getDepthTexture() const { return depthImage_; }

private:
     // G-Buffer 렌더 타겟들
        RHIImage* albedoImage_ = nullptr;       // RGB: Albedo, A: AO
 RHIImage* normalImage_ = nullptr;             // RGB: Normal (World Space)
 RHIImage* positionImage_ = nullptr;         // RGB: Position (World Space)
   RHIImage* metallicRoughnessImage_ = nullptr;         // R: Metallic, G: Roughness
        RHIImage* depthImage_ = nullptr;         // Depth/Stencil

   RHIImageView* albedoView_ = nullptr;
 RHIImageView* normalView_ = nullptr;
        RHIImageView* positionView_ = nullptr;
     RHIImageView* metallicRoughnessView_ = nullptr;
        RHIImageView* depthView_ = nullptr;

 RHIPipeline* pipeline_ = nullptr;

        void createRenderTargets();
      void destroyRenderTargets();
        void createRenderPass();
    void createFramebuffer();
        void createPipeline();
    };

} // namespace BinRenderer
