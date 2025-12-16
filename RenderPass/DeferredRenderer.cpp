#include "DeferredRenderer.h"

namespace BinRenderer
{
    DeferredRenderer::DeferredRenderer(RHI* rhi)
        : rhi_(rhi)
    {
    }

    DeferredRenderer::~DeferredRenderer()
    {
   shutdown();
    }

    bool DeferredRenderer::initialize(uint32_t width, uint32_t height)
    {
     // 1. Shadow Pass 생성
  shadowPass_ = std::make_unique<ShadowPass>(rhi_);
   if (!shadowPass_->initialize())
     {
  return false;
        }

   // 2. G-Buffer Pass 생성
        gbufferPass_ = std::make_unique<GBufferPass>(rhi_);
if (!gbufferPass_->initialize())
   {
      return false;
   }
 gbufferPass_->resize(width, height);

   // 3. Lighting Pass 생성
 lightingPass_ = std::make_unique<LightingPass>(rhi_, gbufferPass_.get());
   if (!lightingPass_->initialize())
     {
  return false;
   }

        // 4. Forward Pass 생성
      forwardPass_ = std::make_unique<ForwardPass>(rhi_);
   if (!forwardPass_->initialize())
     {
    return false;
     }
   forwardPass_->setDepthBuffer(gbufferPass_->getDepthTexture());
   forwardPass_->setOutputTexture(lightingPass_->getLightingTexture());

   // 5. Post-Process Pass 생성
        postProcessPass_ = std::make_unique<PostProcessPass>(rhi_);
 if (!postProcessPass_->initialize())
 {
   return false;
        }
        postProcessPass_->setInputTexture(lightingPass_->getLightingTexture());
        postProcessPass_->resize(width, height);

   return true;
    }

void DeferredRenderer::shutdown()
    {
   postProcessPass_.reset();
  forwardPass_.reset();
        lightingPass_.reset();
   gbufferPass_.reset();
shadowPass_.reset();
 }

    void DeferredRenderer::render(uint32_t frameIndex)
    {
   // 렌더링 파이프라인 실행 순서:
// 1. Shadow Maps 생성
     shadowPass_->execute(frameIndex);

        // 2. G-Buffer 생성 (Geometry Pass)
      gbufferPass_->execute(frameIndex);

// 3. Deferred Lighting (조명 계산)
   lightingPass_->execute(frameIndex);

   // 4. Forward Pass (투명 오브젝트)
        forwardPass_->execute(frameIndex);

   // 5. Post-Processing (Tone Mapping, FXAA 등)
postProcessPass_->execute(frameIndex);
    }

    void DeferredRenderer::resize(uint32_t width, uint32_t height)
    {
        if (gbufferPass_)
gbufferPass_->resize(width, height);

        if (lightingPass_)
       lightingPass_->resize(width, height);

  if (forwardPass_)
   forwardPass_->resize(width, height);

  if (postProcessPass_)
     postProcessPass_->resize(width, height);
    }

RHIImage* DeferredRenderer::getFinalOutput() const
    {
   return postProcessPass_ ? postProcessPass_->getOutputTexture() : nullptr;
    }

} // namespace BinRenderer
