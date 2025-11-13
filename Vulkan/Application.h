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

	// Model configuration structure
	struct ModelConfig
	{
		string filePath;                       // Relative to assets path
		string displayName;                    // Display name for GUI
		glm::mat4 transform = glm::mat4(1.0f); // Model transformation matrix
		bool isBistroObj = false;              // Special handling for Bistro models

		// Animation settings
		bool autoPlayAnimation = true;      // Start animation automatically
		uint32_t initialAnimationIndex = 0; // Which animation to start with
		float animationSpeed = 1.0f;        // Animation playback speed
		bool loopAnimation = true;          // Loop the animation

		// Helper constructors
		ModelConfig() = default;

		ModelConfig(const string& path, const string& name = "",
			const glm::mat4& trans = glm::mat4(1.0f), bool bistro = false)
			: filePath(path), displayName(name.empty() ? path : name), transform(trans),
			isBistroObj(bistro)
		{
		}

		// Fluent interface for easy configuration
		ModelConfig& setName(const string& name)
		{
			displayName = name;
			return *this;
		}
		ModelConfig& setTransform(const glm::mat4& trans)
		{
			transform = trans;
			return *this;
		}
		ModelConfig& setBistroModel(bool bistro)
		{
			isBistroObj = bistro;
			return *this;
		}
		ModelConfig& setAnimation(bool autoPlay, uint32_t index = 0, float speed = 1.0f,
			bool loop = true)
		{
			autoPlayAnimation = autoPlay;
			initialAnimationIndex = index;
			animationSpeed = speed;
			loopAnimation = loop;
			return *this;
		}
	};

	// Camera configuration structure
	struct CameraConfig
	{
		Camera::CameraType type = Camera::CameraType::firstperson;
		glm::vec3 position = glm::vec3(17.794752, -7.657472, 7.049862); // For Bistro model
		glm::vec3 rotation = glm::vec3(8.799977, 107.899704, 0.000000);
		glm::vec3 viewPos = glm::vec3(-17.794752, -7.657472, -7.049862);

		// Camera properties
		float fov = 75.0f;
		float nearPlane = 0.1f;
		float farPlane = 256.0f;
		float movementSpeed = 10.0f;
		float rotationSpeed = 0.1f;

		CameraConfig() = default;

		CameraConfig(const glm::vec3& pos, const glm::vec3& rot = glm::vec3(0.0f),
			const glm::vec3& viewP = glm::vec3(0.0f))
			: position(pos), rotation(rot), viewPos(viewP)
		{
		}

		// Preset configurations
		static CameraConfig forBistro()
		{
			return CameraConfig(glm::vec3(17.794752, -7.657472, 7.049862),  // position
				glm::vec3(8.799977, 107.899704, 0.000000),  // rotation
				glm::vec3(-17.794752, -7.657472, -7.049862) // viewPos
			);
		}

		static CameraConfig forHelmet()
		{
			return CameraConfig(glm::vec3(0.0f, 0.0f, -2.5f), // position
				glm::vec3(0.0f),              // rotation
				glm::vec3(0.0f)               // viewPos
			);
		}

		static CameraConfig forCharacter()
		{
			return CameraConfig(glm::vec3(0.035510, 1.146003, -2.438253), // position
				glm::vec3(-0.210510, 1.546003, 2.438253), // rotation
				glm::vec3(-0.035510, 1.146003, 2.438253)  // viewPos
			);
		}
	};

	// Application configuration structure
	struct ApplicationConfig
	{
		vector<ModelConfig> models;
		CameraConfig camera;

		// Default configuration (current hardcoded setup)
		static ApplicationConfig createDefault()
		{
			ApplicationConfig config;

			// Character model
			ModelConfig character("characters/Leonard/Bboy Hip Hop Move.fbx", "character");
			character.transform = glm::rotate(
				glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-6.719f, 0.375f, -1.860f)),
					glm::vec3(0.012f)),
				glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			character.autoPlayAnimation = true;

			// Bistro scene
			ModelConfig bistro("models/AmazonLumberyardBistroMorganMcGuire/exterior.obj", "distance",
				glm::scale(glm::mat4(1.0f), glm::vec3(0.01f)),
				true // isBistroObj
			);
			bistro.autoPlayAnimation = false;

			config.models = { character, bistro };
			config.camera = CameraConfig::forBistro();

			return config;
		}

		// GLTF showcase configuration
		static ApplicationConfig createGltfShowcase()
		{
			ApplicationConfig config;

			ModelConfig helmet("models/DamagedHelmet.glb", "Damaged Helmet",
				glm::scale(glm::mat4(1.0f), glm::vec3(2.0f)), false);

			config.models = { helmet };
			config.camera = CameraConfig::forHelmet();

			return config;
		}

		// Animation demo configuration
		static ApplicationConfig createAnimationDemo()
		{
			ApplicationConfig config;

			ModelConfig character("characters/Leonard/Bboy Hip Hop Move.fbx", "Animated Character");
			character.setTransform(glm::scale(glm::mat4(1.0f), glm::vec3(0.02f)))
				.setAnimation(true, 0, 1.5f, true);

			config.models = { character };
			config.camera = CameraConfig::forCharacter();

			return config;
		}
	};

	class Application
	{
	public:
		// Legacy constructors (backward compatibility)
		Application();
		Application(const ApplicationConfig& config);
		Application(const string& configFile);

		// New constructor with EngineConfig
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

		// Legacy methods
		void updateGui();
		void handleMouseMove(int32_t x, int32_t y);

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
		// Legacy members (for backward compatibility)
		// ========================================
		const uint32_t kMaxFramesInFlight = 2;
		const string kAssetsPathPrefix = "../../assets/";
		const string kShaderPathPrefix = kAssetsPathPrefix + "shaders/";

		VkExtent2D windowSize_{};
		MouseState mouseState_;
		Camera camera_;  // Legacy: deprecated, use scene_.getCamera()

		// ========================================
		// Synchronization
		// ========================================
		vector<CommandBuffer> commandBuffers_{};
		vector<VkFence> waitFences_{};
		vector<VkSemaphore> presentCompleteSemaphores_{};
		vector<VkSemaphore> renderCompleteSemaphores_{};

		// ========================================
		// Legacy model storage (deprecated)
		// ========================================
		vector<unique_ptr<Model>> models_;

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

		// Legacy configuration loading
		void setupCamera(const CameraConfig& cameraConfig);
		void loadModels(const vector<ModelConfig>& modelConfigs);

		// Performance tracking
		void updatePerformanceMetrics(float deltaTime);

		// GUI windows
		void renderHDRControlWindow();
		void renderPostProcessingControlWindow();
		void renderCameraControlWindow();
		void renderSSAOControlWindow();
	};

} // namespace BinRenderer::Vulkan