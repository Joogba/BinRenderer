#pragma once

#include "RenderPassBase.h"
#include "GBufferPass.h"
#include "LightingPass.h"
#include "ShadowPass.h"
#include "ForwardPass.h"
#include "PostProcessPass.h"

namespace BinRenderer
{
    /**
  * @brief Deferred Rendering 파이프라인
     * 
     * 전체 렌더링 파이프라인을 관리하고 실행
     */
    class DeferredRenderer
    {
    public:
        DeferredRenderer(RHI* rhi);
   ~DeferredRenderer();

// 초기화
        bool initialize(uint32_t width, uint32_t height);
void shutdown();

        // 렌더링 실행
  void render(uint32_t frameIndex);

  // 리사이즈
        void resize(uint32_t width, uint32_t height);

     // 렌더 패스 접근
   GBufferPass* getGBufferPass() const { return gbufferPass_.get(); }
        LightingPass* getLightingPass() const { return lightingPass_.get(); }
 ShadowPass* getShadowPass() const { return shadowPass_.get(); }
        ForwardPass* getForwardPass() const { return forwardPass_.get(); }
     PostProcessPass* getPostProcessPass() const { return postProcessPass_.get(); }

        // 최종 출력 텍스처
RHIImage* getFinalOutput() const;

 private:
  RHI* rhi_;

   // Render Passes (실행 순서대로)
        std::unique_ptr<ShadowPass> shadowPass_;
   std::unique_ptr<GBufferPass> gbufferPass_;
   std::unique_ptr<LightingPass> lightingPass_;
        std::unique_ptr<ForwardPass> forwardPass_;
        std::unique_ptr<PostProcessPass> postProcessPass_;
    };

} // namespace BinRenderer
