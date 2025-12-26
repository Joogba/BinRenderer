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
        class IWindow* windowInterface = nullptr;  //  플랫폼 독립적 Window 인터페이스
        uint32_t windowWidth = 1280;
        uint32_t windowHeight = 720;
        bool enableValidationLayer = false;
        bool enableDebugUtils = false;
        std::vector<const char*> requiredInstanceExtensions;
        uint32_t maxFramesInFlight = 2;
    };



} // namespace BinRenderer
