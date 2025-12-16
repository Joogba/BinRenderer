#include "LightingPass.h"

namespace BinRenderer
{
    LightingPass::LightingPass(RHI* rhi, GBufferPass* gbufferPass)
        : RenderPassBase(rhi, "LightingPass"), gbufferPass_(gbufferPass)
    {
    }

    LightingPass::~LightingPass()
    {
  shutdown();
  }

    bool LightingPass::initialize()
    {
   width_ = gbufferPass_->getWidth();
  height_ = gbufferPass_->getHeight();

      createRenderTargets();
     createRenderPass();
 createFramebuffer();
        createPipeline();

        return true;
    }

 void LightingPass::shutdown()
    {
   if (pipeline_)
   {
 rhi_->destroyPipeline(pipeline_);
       pipeline_ = nullptr;
   }

  destroyRenderTargets();
  }

void LightingPass::resize(uint32_t width, uint32_t height)
    {
   if (width_ == width && height_ == height)
 {
    return;
}

   width_ = width;
        height_ = height;

     destroyRenderTargets();
   createRenderTargets();
     createFramebuffer();
    }

    void LightingPass::execute(uint32_t frameIndex)
    {
     RHIClearValue clearValue{};
        clearValue.color[0] = 0.0f;
     clearValue.color[1] = 0.0f;
  clearValue.color[2] = 0.0f;
 clearValue.color[3] = 1.0f;

     beginRenderPass(frameIndex, &clearValue, 1);

   // Fullscreen Quad로 조명 계산
     // rhi_->cmdBindPipeline(pipeline_);
        // rhi_->cmdBindDescriptorSets(...); // G-Buffer 텍스처들 바인딩
    // rhi_->cmdDraw(3, 1, 0, 0); // Fullscreen Triangle

    endRenderPass();
    }

    void LightingPass::createRenderTargets()
{
   // Lighting Result (RGBA16F - HDR)
        RHIImageCreateInfo lightingInfo{};
        lightingInfo.width = width_;
   lightingInfo.height = height_;
        lightingInfo.depth = 1;
lightingInfo.mipLevels = 1;
    lightingInfo.arrayLayers = 1;
lightingInfo.format = RHI_FORMAT_R16G16B16A16_SFLOAT;
        lightingInfo.tiling = RHI_IMAGE_TILING_OPTIMAL;
   lightingInfo.usage = RHI_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | RHI_IMAGE_USAGE_SAMPLED_BIT;
        lightingInfo.samples = RHI_SAMPLE_COUNT_1_BIT;
   lightingImage_ = rhi_->createImage(lightingInfo);

        // TODO: Image View 생성
    // TODO: Sampler 생성 (G-Buffer 샘플링용)
    }

    void LightingPass::destroyRenderTargets()
    {
   if (lightingImage_)
 {
       rhi_->destroyImage(lightingImage_);
 lightingImage_ = nullptr;
 }
    }

    void LightingPass::createRenderPass()
    {
     // TODO: RenderPass 생성
    }

void LightingPass::createFramebuffer()
  {
   // TODO: Framebuffer 생성
    }

    void LightingPass::createPipeline()
{
   // TODO: Fullscreen Quad용 파이프라인 생성
        // Vertex Shader: Fullscreen Triangle
     // Fragment Shader: Deferred Lighting
    }

} // namespace BinRenderer
