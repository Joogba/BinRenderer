#pragma once

#include <string>
#include <cstdint>

namespace BinRenderer::Vulkan {

using namespace std;

/**
 * @brief 렌더링 엔진의 전역 설정
 * 
 * 하드코딩된 경로, 상수 등을 외부에서 설정 가능하도록 구조화
 */
struct EngineConfig
{
    // ========================================
    // Path Configuration
    // ========================================
    string assetsPath = "../../assets/";         // Asset root directory
    string shaderPath = "../../assets/shaders/";   // Shader directory (relative or absolute)
    
    // ========================================
    // Rendering Configuration
    // ========================================
    uint32_t maxFramesInFlight = 2;        // Double/Triple buffering
    bool enableValidationLayers = true;    // Vulkan validation layers
    bool enableVsync = true;  // Vertical sync
    
    // ========================================
    // Window Configuration
  // ========================================
    uint32_t windowWidth = 1920;     // Initial window width
    uint32_t windowHeight = 1080;              // Initial window height
    string windowTitle = "BinRenderer";            // Window title
    bool windowResizable = true;          // Allow window resize
  bool windowFullscreen = false;   // Start in fullscreen
    
    // ========================================
    // Feature Flags
    // ========================================
    bool enableGui = true;     // ImGui rendering
    bool enableProfiling = false;       // Tracy profiler (requires TRACY_ENABLE)
    bool enableGpuTiming = true;            // GPU timestamp queries
    bool enableMSAA = false;    // Multi-sampling (future)
    
    // ========================================
    // Performance Configuration
    // ========================================
    float fpsUpdateInterval = 0.1f;   // FPS counter update frequency (seconds)
    float gpuTimeUpdateInterval = 0.1f;            // GPU timing update frequency (seconds)
    
    // ========================================
    // Helper Methods
    // ========================================
    
    /**
     * @brief 기본 설정 반환
     */
    static EngineConfig createDefault()
    {
        return EngineConfig{};
    }
 
    /**
     * @brief 개발용 설정 (Validation 활성화)
     */
    static EngineConfig createDevelopment()
    {
        EngineConfig config;
     config.enableValidationLayers = true;
        config.enableProfiling = true;
    config.enableGpuTiming = true;
     return config;
    }
    
    /**
     * @brief 릴리즈용 설정 (최적화)
     */
    static EngineConfig createRelease()
    {
        EngineConfig config;
        config.enableValidationLayers = false;
      config.enableProfiling = false;
 config.enableGui = false;
        return config;
    }
    
    /**
     * @brief Fluent interface: 경로 설정
     */
    EngineConfig& setAssetsPath(const string& path)
    {
        assetsPath = path;
      // Shader path를 자동으로 업데이트
        if (shaderPath.find(assetsPath) == string::npos) {
  shaderPath = assetsPath + "shaders/";
     }
        return *this;
    }
    
    EngineConfig& setShaderPath(const string& path)
    {
        shaderPath = path;
 return *this;
    }
 
    /**
     * @brief Fluent interface: 윈도우 설정
     */
    EngineConfig& setWindowSize(uint32_t width, uint32_t height)
    {
        windowWidth = width;
        windowHeight = height;
  return *this;
    }
    
    EngineConfig& setWindowTitle(const string& title)
    {
        windowTitle = title;
        return *this;
    }
    
    /**
     * @brief Fluent interface: 기능 토글
     */
    EngineConfig& enableFeature(bool gui, bool profiling, bool validation)
    {
      enableGui = gui;
        enableProfiling = profiling;
     enableValidationLayers = validation;
        return *this;
    }
};

} // namespace BinRenderer::Vulkan
