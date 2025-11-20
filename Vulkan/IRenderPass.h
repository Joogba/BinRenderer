#pragma once

#include "Context.h"
#include <vulkan/vulkan.h>
#include <string>

namespace BinRenderer::Vulkan {

    struct RenderPassContext {
        // 렌더 패스 실행에 필요한 외부 컨텍스트 (Renderer에서 전달)
        // 예: 뷰 행렬, 라이트 정보가 담긴 버퍼, 디스크립터 세트 등
        // Context::GetGlobalDescriptorSet() 등으로 대체될 수 있음.
    };
/**
 * @brief 모든 렌더 패스의 기본 인터페이스
 * 
 * 각 렌더링 단계(Scene, Cloth, Post, GUI 등)는 이 인터페이스를 구현합니다.
 * Open-Closed Principle: 확장에는 열려있고, 수정에는 닫혀있습니다.
 */
class IRenderPass
{
public:
    virtual ~IRenderPass() = default;

    /**
     * @brief 렌더 패스 초기화
     * @param ctx Vulkan 컨텍스트
     * @return 성공 여부
     */
    virtual bool initialize(Context& ctx) = 0;

    /**
     * @brief 프레임 시작 시 호출 (업데이트 로직)
     * @param deltaTime 프레임 간 시간
     * @param frameIndex 현재 프레임 인덱스
     */
    virtual void update(float deltaTime, uint32_t frameIndex) = 0;

    /**
     * @brief 렌더링 명령 기록
     * @param cmd 커맨드 버퍼
  * @param frameIndex 현재 프레임 인덱스
     */
    virtual void render(VkCommandBuffer cmd, uint32_t frameIndex) = 0;

 /**
 * @brief 리소스 정리
     */
virtual void cleanup() = 0;

    /**
     * @brief 렌더 패스 이름 반환
     */
    virtual const std::string& getName() const = 0;

    /**
     * @brief 렌더 패스 활성화 여부
     */
    virtual bool isEnabled() const { return enabled_; }
    virtual void setEnabled(bool enabled) { enabled_ = enabled; }

protected:
    bool enabled_ = true;
};

} // namespace BinRenderer::Vulkan
