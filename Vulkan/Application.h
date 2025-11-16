#pragma once

#include "Camera.h"
#include "Context.h"
#include "EngineConfig.h"
#include "GuiRenderer.h"
#include "GpuTimer.h"
#include "IApplicationListener.h"
#include "Image2D.h"
#include "Logger.h"
#include "MappedBuffer.h"
#include "Model.h"
#include "Pipeline.h"
#include "Renderer.h"
#include "Sampler.h"
#include "Scene.h"
#include "ShaderManager.h"
#include "StorageBuffer.h"
#include "Swapchain.h"
#include "TracyProfiler.h"
#include "Window.h"

#include <format>
#include <fstream>
#include <glm/gtx/string_cast.hpp>
#include <optional>

namespace BinRenderer::Vulkan {

	struct MouseState
	{
		struct
		{
			bool left = false;
			bool right = false;
			bool middle = false;
		} buttons;
		glm::vec2 position{ 0.0f, 0.0f };
	};

	class Application
	{
	public:
		// ========================================
		// Constructor with EngineConfig
		// ========================================
		Application(const EngineConfig& engineConfig,
			IApplicationListener* listener = nullptr);

		~Application();

		/**
		 * @brief 메인 루프 실행
		 */
		void run();

		/**
		 * @brief 리스너 설정 (런타임에 변경 가능)
		 */
		void setListener(IApplicationListener* listener)
		{
			listener_ = listener;
		}

		/**
		 * @brief 씬 접근
		 */
		Scene& getScene()
		{
			return scene_;
		}

		/**
			 * @brief 렌더러 접근
		 */
		Renderer& getRenderer()
		{
			return *renderer_;
		}

		/**
		 * @brief 엔진 설정 접근
		 */
		const EngineConfig& getEngineConfig() const
		{
			return engineConfig_;
		}

		/**
		 * @brief 카메라 접근 (씬 카메라의 alias)
		 */
		Camera& getCamera()
		{
			return scene_.getCamera();
		}

	private:
		// ========================================
		// Engine Configuration
		// ========================================
		EngineConfig engineConfig_;
		IApplicationListener* listener_ = nullptr;

		// ========================================
		// Core Systems
		// ========================================
		Window window_;
		Context ctx_;
		Swapchain swapchain_;
		ShaderManager shaderManager_;
		Scene scene_;
		unique_ptr<Renderer> renderer_;
		GuiRenderer guiRenderer_;

		// ========================================
		// Rendering State
		// ========================================
		VkExtent2D windowSize_{};
		MouseState mouseState_;
		Camera camera_;  // Application-level camera (synchronized with Scene)

		// ========================================
		// Synchronization
		// ========================================
		vector<CommandBuffer> commandBuffers_{};
		vector<VkFence> waitFences_{};
		vector<VkSemaphore> presentCompleteSemaphores_{};
		vector<VkSemaphore> renderCompleteSemaphores_{};

		// ========================================
		// Profiling
		// ========================================
		GpuTimer gpuTimer_;
		unique_ptr<TracyProfiler> tracyProfiler_;
		float currentGpuTimeMs_{ 0.0f };
		float gpuTimeUpdateTimer_{ 0.0f };
		uint32_t gpuFramesSinceLastUpdate_{ 0 };
		static constexpr float kGpuTimeUpdateInterval = 0.1f;

		float currentFPS_{ 0.0f };
		float fpsUpdateTimer_{ 0.0f };
		uint32_t framesSinceLastUpdate_{ 0 };
		static constexpr float kFpsUpdateInterval = 0.1f;

		// ========================================
		// Initialization Methods
		// ========================================
		void initializeVulkanResources();
		void setupCallbacks();

		// ========================================
		// GUI & Input
		// ========================================
		void updateGui();
		void handleMouseMove(int32_t x, int32_t y);

		// Performance tracking
		void updatePerformanceMetrics(float deltaTime);

		// GUI windows
		void renderHDRControlWindow();
		void renderPostProcessingControlWindow();
		void renderCameraControlWindow();
		void renderSSAOControlWindow();
		
		// ========================================
		// Scene Integration Helpers
		// ========================================
		
		/**
		 * @brief Scene의 모든 visible 모델을 Model* 배열로 반환 (Transform 적용됨)
		 */
		vector<Model*> getSceneModels();
		
		/**
		 * @brief Scene의 모든 visible 노드의 Transform을 Model에 적용
		 */
		void syncSceneTransforms();
	};

} // namespace BinRenderer::Vulkan