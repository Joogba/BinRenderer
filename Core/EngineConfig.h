#pragma once

#include <string>
#include <cstdint>

namespace BinRenderer
{
	/**
	 * @brief 플랫폼 독립적 엔진 설정
	 */
	struct EngineConfig
	{
		// ========================================
		// Path Configuration
		// ========================================
		std::string assetsPath = "../../assets/";
		std::string shaderPath = "../../assets/shaders/";

		// ========================================
		// Rendering Configuration
		// ========================================
		uint32_t maxFramesInFlight = 2;
		bool enableValidationLayers = true;
		bool enableVsync = true;

		// ========================================
		// Window Configuration
		// ========================================
		uint32_t windowWidth = 1920;
		uint32_t windowHeight = 1080;
		std::string windowTitle = "BinRenderer";
		bool windowResizable = true;
		bool windowFullscreen = false;

		// ========================================
		// Feature Flags
		// ========================================
		bool enableGui = true;
		bool enableProfiling = false;
		bool enableGpuTiming = true;
		bool enableMSAA = false;

		// ========================================
		// Performance Configuration
		// ========================================
		float fpsUpdateInterval = 0.1f;
		float gpuTimeUpdateInterval = 0.1f;

		// ========================================
		// Helper Methods
		// ========================================

		static EngineConfig createDefault()
		{
			return EngineConfig{};
		}

		static EngineConfig createDevelopment()
		{
			EngineConfig config;
			config.enableValidationLayers = true;
			config.enableProfiling = true;
			config.enableGpuTiming = true;
			return config;
		}

		static EngineConfig createRelease()
		{
			EngineConfig config;
			config.enableValidationLayers = false;
			config.enableProfiling = false;
			config.enableGui = false;
			return config;
		}

		// Fluent interface
		EngineConfig& setAssetsPath(const std::string& path)
		{
			assetsPath = path;
			shaderPath = assetsPath + "shaders/";
			return *this;
		}

		EngineConfig& setShaderPath(const std::string& path)
		{
			shaderPath = path;
			return *this;
		}

		EngineConfig& setWindowSize(uint32_t width, uint32_t height)
		{
			windowWidth = width;
			windowHeight = height;
			return *this;
		}

		EngineConfig& setWindowTitle(const std::string& title)
		{
			windowTitle = title;
			return *this;
		}

		EngineConfig& setMaxFramesInFlight(uint32_t frames)
		{
			maxFramesInFlight = frames;
			return *this;
		}

		EngineConfig& setVsync(bool enable)
		{
			enableVsync = enable;
			return *this;
		}

		EngineConfig& setValidation(bool enable)
		{
			enableValidationLayers = enable;
			return *this;
		}
	};

} // namespace BinRenderer
