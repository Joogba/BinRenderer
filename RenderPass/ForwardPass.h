#pragma once

#include "RenderPassBase.h"

namespace BinRenderer
{
    /**
     * @brief Forward 렌더링 패스
* 
     * Deferred Rendering으로 처리할 수 없는 투명 오브젝트 렌더링
     */
    class ForwardPass : public RenderPassBase
    {
  public:
  ForwardPass(RHI* rhi);
   ~ForwardPass() override;

 bool initialize() override;
        void shutdown() override;
   void resize(uint32_t width, uint32_t height) override;
     void execute(uint32_t frameIndex) override;

   // Depth Buffer 공유 (G-Buffer의 Depth 사용)
        void setDepthBuffer(RHIImage* depthImage);

     // 출력 텍스처 (Lighting Pass 결과 위에 렌더링)
        void setOutputTexture(RHIImage* outputTexture);

    private:
   RHIImage* depthImage_ = nullptr;  // 외부에서 설정 (G-Buffer Depth)
        RHIImage* outputImage_ = nullptr;  // 외부에서 설정 (Lighting Result)

RHIPipeline* pipeline_ = nullptr;

        void createRenderPass();
     void createFramebuffer();
        void createPipeline();
    };

} // namespace BinRenderer
