#pragma once

#include "IRenderPass.h"
#include <vector>
#include <memory>
#include <string>

namespace BinRenderer::Vulkan {

/**
 * @brief 모든 렌더 패스를 관리하는 매니저
 * 
 * 렌더 패스를 동적으로 추가/제거하고, 순서대로 실행합니다.
 * Strategy Pattern: 각 렌더 패스는 독립적인 전략으로 동작
 */
class RenderPassManager
{
public:
    RenderPassManager() = default;
    ~RenderPassManager() = default;

    /**
     * @brief 렌더 패스 추가
     * @param renderPass 추가할 렌더 패스
     * @param priority 실행 순서 (낮을수록 먼저 실행)
     */
    void addRenderPass(std::unique_ptr<IRenderPass> renderPass, int priority = 100);

    /**
     * @brief 이름으로 렌더 패스 찾기
     */
    IRenderPass* getRenderPass(const std::string& name);

/**
     * @brief 모든 렌더 패스 초기화
     */
    bool initializeAll(Context& ctx);

    /**
     * @brief 모든 활성화된 렌더 패스 업데이트
     */
    void updateAll(float deltaTime, uint32_t frameIndex);

 /**
     * @brief 모든 활성화된 렌더 패스 렌더링
     */
    void renderAll(VkCommandBuffer cmd, uint32_t frameIndex);

    /**
     * @brief 모든 렌더 패스 정리
     */
    void cleanupAll();

    /**
     * @brief 렌더 패스 개수 반환
     */
 size_t getPassCount() const { return renderPasses_.size(); }

private:
    struct RenderPassEntry
    {
     std::unique_ptr<IRenderPass> pass;
        int priority;

     bool operator<(const RenderPassEntry& other) const
        {
   return priority < other.priority;
        }
    };

    std::vector<RenderPassEntry> renderPasses_;
    bool needsSort_ = false;

    void sortRenderPasses();
};

} // namespace BinRenderer::Vulkan
