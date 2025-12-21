#pragma once

#include "RHIType.h"
#include <cstdint>
#include <vector>

namespace BinRenderer
{
    // Forward declaration
    class IWindow;

    // API 타입
    enum class RHIApiType
    {
        Vulkan,
        D3D12,
        Metal,
        OpenGL
    };

    // 초기화 정보
    struct RHIInitInfo
    {
        void* window = nullptr;  // 네이티브 핸들 (레거시, 사용 안 함)
        class IWindow* windowInterface = nullptr;  // ✅ 플랫폼 독립적 Window 인터페이스
        uint32_t windowWidth = 1280;
        uint32_t windowHeight = 720;
        bool enableValidationLayer = false;
        bool enableDebugUtils = false;
        std::vector<const char*> requiredInstanceExtensions;
        uint32_t maxFramesInFlight = 2;
    };

    // 뷰포트
    struct RHIViewport
    {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
        float minDepth = 0.0f;
        float maxDepth = 1.0f;
    };

    // 시저 영역
    struct RHIRect2D
    {
        struct Offset { int32_t x, y; } offset;
        struct Extent { uint32_t width, height; } extent;
    };

    // 클리어 컬러
    struct RHIClearValue
    {
        union {
            float color[4];
            struct { float depth; uint32_t stencil; } depthStencil;
        };
    };

    // Offset 2D
    struct RHIOffset2D
    {
        int32_t x = 0;
        int32_t y = 0;
    };

    // Extent 2D
    struct RHIExtent2D
    {
        uint32_t width = 0;
        uint32_t height = 0;
    };

    // Extent 3D
    struct RHIExtent3D
    {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t depth = 1;
    };

} // namespace BinRenderer
