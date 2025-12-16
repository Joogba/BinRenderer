#include "ForwardPass.h"

namespace BinRenderer
{
    ForwardPass::ForwardPass(RHI* rhi)
        : RenderPassBase(rhi, "ForwardPass")
    {
    }

    ForwardPass::~ForwardPass()
    {
shutdown();
    }

    bool ForwardPass::initialize()
    {
   createRenderPass();
        createPipeline();

   return true;
    }

  void ForwardPass::shutdown()
    {
   if (pipeline_)
   {
    rhi_->destroyPipeline(pipeline_);
 pipeline_ = nullptr;
     }
    }

    void ForwardPass::resize(uint32_t width, uint32_t height)
    {
        width_ = width;
        height_ = height;
  createFramebuffer();
    }

    void ForwardPass::execute(uint32_t frameIndex)
    {
   if (!depthImage_ || !outputImage_)
        {
       return;
   }

      // Depth는 clear하지 않음 (G-Buffer의 depth 재사용)
   RHIClearValue clearValue{};
 clearValue.color[0] = 0.0f;
        clearValue.color[1] = 0.0f;
clearValue.color[2] = 0.0f;
clearValue.color[3] = 0.0f;

   beginRenderPass(frameIndex, &clearValue, 1);

 // 투명 오브젝트 렌더링 (Back-to-Front 정렬 필요)
        // rhi_->cmdBindPipeline(pipeline_);
   // ... draw transparent objects ...

   endRenderPass();
    }

  void ForwardPass::setDepthBuffer(RHIImage* depthImage)
    {
   depthImage_ = depthImage;
    }

void ForwardPass::setOutputTexture(RHIImage* outputTexture)
    {
        outputImage_ = outputTexture;
    }

    void ForwardPass::createRenderPass()
 {
// TODO: RenderPass 생성
   // Load Op: LOAD (기존 컬러 유지)
     // Depth: Depth Test만 수행, Write 안함
    }

    void ForwardPass::createFramebuffer()
    {
// TODO: Framebuffer 생성
    }

    void ForwardPass::createPipeline()
  {
   // TODO: Forward 렌더링 파이프라인 생성
 // Blend: Alpha Blending 활성화
// Depth: Test=ON, Write=OFF
    }

} // namespace BinRenderer
