#pragma once

#include "Camera.h"
#include "Context.h"
#include "EngineConfig.h"
#include "GuiRenderer.h"
#include "GpuTimer.h"
#include "IApplicationListener.h"
#include "Image2D.h"
#include "InputManager.h"
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
#include "VulkanResourceManager.h"  //  VulkanResourceManager 추가
#include "RenderPassManager.h" //  NEW: RenderPass system

#include "../Resources/ResourceManager.h"

#include <format>
#include <fstream>
#include <glm/gtx/string_cast.hpp>
#include <optional>

namespace BinRenderer::Vulkan {

	/**
	 * @brief Application-level input handler
	 * 
	 * 엔진 레벨의 기본 입력 처리 (카메라 이동, 애니메이션 제어 등)
	 */
	class ApplicationInputHandler : public IInputListener
	{
	public:
		ApplicationInputHandler(class Application* app) : app_(app) {}

		void onKeyPressed(int key, int mods) override;
		void onKeyReleased(int key, int mods) override;
		void onMouseButtonPressed(MouseButton button, double x, double y) override;
		void onMouseButtonReleased(MouseButton button, double x, double y) override;
		void onMouseMoved(double x, double y, double deltaX, double deltaY) override;
		void onMouseScrolled(double xOffset, double yOffset) override;

	private:
		class Application* app_;
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

		/**
		 * @brief InputManager 접근
		 */
		InputManager& getInputManager()
		{
			return inputManager_;
		}

		/**
		 * @brief 플랫폼 독립적 ResourceManager 접근
		 */
		BinRenderer::ResourceManager& getResourceManager()
		{
			return *resourceManager_;
		}

		/**
		 * @brief Vulkan 전용 ResourceManager 접근
		 */
		VulkanResourceManager& getVulkanResourceManager()
		{
			return *vulkanResourceManager_;
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
		unique_ptr<BinRenderer::ResourceManager> resourceManager_;  //  플랫폼 독립적
		unique_ptr<VulkanResourceManager> vulkanResourceManager_;    //  Vulkan 전용
		Scene scene_;
		unique_ptr<Renderer> renderer_;
		GuiRenderer guiRenderer_;
		InputManager inputManager_;
		unique_ptr<ApplicationInputHandler> inputHandler_;

		// ========================================
		// Rendering State
		// ========================================
		VkExtent2D windowSize_{};
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
		void initializeInputSystem();

		// ========================================
		// GUI
		// ========================================
		void updateGui();

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

		// Friend class for input handling
		friend class ApplicationInputHandler;
	};

} // namespace BinRenderer::Vulkan