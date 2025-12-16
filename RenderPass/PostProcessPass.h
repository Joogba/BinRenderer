#pragma once

#include "RenderPassBase.h"

namespace BinRenderer
{
    /**
  * @brief 포스트 프로세싱 패스
* 
  * Tone Mapping, Bloom, FXAA 등
  */
    class PostProcessPass : public RenderPassBase
    {
    public:
        PostProcessPass(RHI* rhi);
        ~PostProcessPass() override;

   bool initialize() override;
        void shutdown() override;
        void resize(uint32_t width, uint32_t height) override;
   void execute(uint32_t frameIndex) override;

   // 입력 설정
   void setInputTexture(RHIImage* inputTexture);

        // 출력 텍스처
        RHIImage* getOutputTexture() const { return outputImage_; }

        // 포스트 프로세싱 옵션
        struct Options
        {
bool enableToneMapping = true;
      bool enableBloom = false;
      bool enableFXAA = true;
            float exposure = 1.0f;
  };

        void setOptions(const Options& options) { options_ = options; }

    private:
   RHIImage* inputImage_ = nullptr;  // 외부에서 설정
   RHIImage* outputImage_ = nullptr;
        RHIImageView* outputView_ = nullptr;

    RHIPipeline* toneMappingPipeline_ = nullptr;
  RHIPipeline* fxaaPipeline_ = nullptr;
 RHISampler* sampler_ = nullptr;

   Options options_;

   void createRenderTargets();
     void destroyRenderTargets();
        void createRenderPass();
   void createFramebuffer();
        void createPipelines();
    };

} // namespace BinRenderer
