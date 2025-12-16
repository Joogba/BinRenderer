#pragma once

#include "RenderPassBase.h"
#include "GBufferPass.h"

namespace BinRenderer
{
    /**
     * @brief 조명 계산 패스 (Deferred Rendering 2단계)
  * 
     * G-Buffer의 데이터를 읽어 조명 계산 수행
  */
class LightingPass : public RenderPassBase
    {
    public:
      LightingPass(RHI* rhi, GBufferPass* gbufferPass);
   ~LightingPass() override;

        bool initialize() override;
   void shutdown() override;
  void resize(uint32_t width, uint32_t height) override;
        void execute(uint32_t frameIndex) override;

 // 조명 결과 텍스처
  RHIImage* getLightingTexture() const { return lightingImage_; }

    private:
GBufferPass* gbufferPass_;

     // 조명 결과 렌더 타겟
        RHIImage* lightingImage_ = nullptr;  // RGB: Lighting Result, A: Unused
   RHIImageView* lightingView_ = nullptr;

     RHIPipeline* pipeline_ = nullptr;
        RHISampler* sampler_ = nullptr;

        void createRenderTargets();
void destroyRenderTargets();
        void createRenderPass();
     void createFramebuffer();
   void createPipeline();
    };

} // namespace BinRenderer
