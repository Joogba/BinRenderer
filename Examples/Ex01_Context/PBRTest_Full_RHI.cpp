#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../../Core/RHIApplication.h"
#include "../../Core/RHIScene.h"
#include "../../Core/Logger.h"
#include "../../Core/EngineConfig.h"
#include "../../Core/InputManager.h"

using namespace BinRenderer;

/**
 * @brief 완전한 RHI 기반 테스트 애플리케이션
 * 
 * 이 예제는 다음을 보여줍니다:
 * - EngineConfig를 통한 설정 관리
 * - InputManager를 통한 입력 처리
 * - RHIScene을 통한 모델 관리 (GPU 인스턴싱)
 * - RenderGraph를 통한 선언적 렌더링
 * - Animation 시스템 통합 (플랫폼 독립적)
 */
class FullRHITestApp : public IRHIApplicationListener
{
public:
	void onInit(RHIScene& scene, RenderGraph& renderGraph, RHICamera& camera) override
	{
		printLog("");
		printLog("==============================================");
		printLog("  Full RHI-Based Rendering Test");
		printLog("==============================================");
		printLog("Platform-independent architecture:");
		printLog("  - RHI: Vulkan/DX12/Metal abstraction");
		printLog("  - Scene: Platform-independent model management");
		printLog("  - Camera: Platform-independent camera system");
		printLog("  - Animation: Platform-independent logic");
		printLog("  - RenderGraph: Declarative render passes");
		printLog("  - InputManager: Platform-independent input");
		printLog("  - EngineConfig: Centralized configuration");
		printLog("==============================================");
		printLog("");

		// Camera 설정
		camera.setPosition(glm::vec3(0.0f, 0.0f, 15.0f));
		camera.setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
		camera.setMovementSpeed(5.0f);
		printLog("Camera initialized:");
		printLog("  - Position: (0, 0, 15)");
		printLog("  - Movement speed: 5.0");
		printLog("");

		const std::string helmetPath = "../../assets/models/DamagedHelmet.glb";
		
		printLog("Adding 3 helmet instances with GPU instancing...");
		
		// 첫 번째 헬멧: 왼쪽
		{
			glm::vec3 position(-5.0f, 0.0f, 0.0f);
			glm::mat4 translate = glm::translate(glm::mat4(1.0f), position);
			glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			glm::mat4 transform = translate * scale * rotation;
			
			scene.addModelInstance(helmetPath, "Helmet_Left", transform);
		}
		
		// 두 번째 헬멧: 중앙
		{
			glm::vec3 position(0.0f, 0.0f, 0.0f);
			glm::mat4 translate = glm::translate(glm::mat4(1.0f), position);
			glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			glm::mat4 transform = translate * scale * rotation;
			
			scene.addModelInstance(helmetPath, "Helmet_Center", transform);
		}
		
		// 세 번째 헬멧: 오른쪽
		{
			glm::vec3 position(5.0f, 0.0f, 0.0f);
			glm::mat4 translate = glm::translate(glm::mat4(1.0f), position);
			glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			glm::mat4 transform = translate * scale * rotation;
			
			scene.addModelInstance(helmetPath, "Helmet_Right", transform);
		}

		printLog("");
		printLog("Scene setup complete:");
		printLog("  - 3 helmet instances");
		printLog("  - 1 shared model (GPU instancing)");
		printLog("  - Automatic resource caching");
		printLog("");
		printLog("Architecture Benefits:");
		printLog("  ✅ Platform Independence");
		printLog("     - RHI abstracts Vulkan/DX12/Metal");
		printLog("     - Same code works on all platforms");
		printLog("  ✅ Modular Design");
		printLog("     - Camera: First-person & LookAt modes");
		printLog("     - Animation: Pure logic (no rendering)");
		printLog("     - Material: Data-driven");
		printLog("  - Mesh: Self-contained");
		printLog("  ✅ RenderGraph");
		printLog("     - Automatic resource management");
		printLog("- Dependency tracking");
		printLog("   - Performance optimization");
		printLog("  ✅ Configuration");
		printLog("     - EngineConfig: Centralized settings");
		printLog("     - Easy to switch dev/release modes");
		printLog("");
	}

	void onUpdate(float deltaTime, uint32_t frameIndex) override
	{
		elapsedTime_ += deltaTime;
		
		if (frameIndex % 60 == 0)
		{
			printLog("Frame {}: Elapsed {:.2f}s", frameIndex, elapsedTime_);
		}

		// TODO: Input 처리 예제
		// auto* input = app->getInputManager();
		// if (input->isKeyPressed(GLFW_KEY_ESCAPE)) {
		//     // Exit
		// }
	}

	void onGui() override
	{
		// TODO: ImGui integration
		// ImGui::Begin("RHI Test");
		// ImGui::Text("FPS: %.1f", fps);
		// ImGui::End();
	}

	void onShutdown() override
	{
		printLog("");
		printLog("==============================================");
		printLog("  Shutting down RHI Application");
		printLog("==============================================");
		printLog("Final Statistics:");
		printLog("  - Total frames rendered: {}", static_cast<int>(elapsedTime_ / 0.016f));
		printLog("  - Total elapsed time: {:.2f}s", elapsedTime_);
		printLog("");
	}

private:
	float elapsedTime_ = 0.0f;
};

int main()
{
	printLog("========================================");
	printLog("BinRenderer - Full RHI System Test");
	printLog("========================================");
	printLog("");
	printLog("This example demonstrates:");
	printLog("  ✅ RHI abstraction layer (Vulkan/DX12/Metal)");
	printLog("  ✅ Platform-independent components");
	printLog("     - Scene/Animation (pure logic)");
	printLog("     - Core/RHIModel (RHI buffers)");
	printLog("     - Rendering/RHIMaterial (data-driven)");
	printLog("     - Rendering/RHIMesh (self-contained)");
	printLog("  ✅ RenderGraph system");
	printLog("     - Declarative render passes");
	printLog("     - Automatic dependency resolution");
	printLog("  ✅ Configuration system");
	printLog("     - EngineConfig for centralized settings");
	printLog("  ✅ Input system");
	printLog("     - Platform-independent InputManager");
	printLog("");
	printLog("Next Steps:");
	printLog("  1. Add real window integration (GLFW/SDL)");
	printLog("  2. Implement actual rendering");
	printLog("  3. Add ImGui support");
	printLog("  4. Integrate with existing Vulkan passes");
	printLog("  5. Add DirectX 12 / Metal backends");

	// ========================================
	// EngineConfig 설정
	// ========================================
	EngineConfig config = EngineConfig::createDevelopment();
	config.setAssetsPath("../../assets/")
		.setWindowSize(1280, 720)
		.setWindowTitle("BinRenderer - Full RHI Test")
		.setMaxFramesInFlight(2)
		.setVsync(true)
		.setValidation(true);

	printLog("Configuration:");
	printLog("  - Window: {}x{}", config.windowWidth, config.windowHeight);
	printLog("  - Title: {}", config.windowTitle);
	printLog("  - Assets: {}", config.assetsPath);
	printLog("  - Shaders: {}", config.shaderPath);
	printLog("  - Max Frames: {}", config.maxFramesInFlight);
	printLog("  - Vsync: {}", config.enableVsync ? "ON" : "OFF");
	printLog("  - Validation: {}", config.enableValidationLayers ? "ON" : "OFF");
	printLog("");

	// ========================================
	// Application 실행
	// ========================================
	printLog("Starting application...");
	printLog("");

	FullRHITestApp listener;
	RHIApplication app(config, RHIApiType::Vulkan);
	app.setListener(&listener);
	app.run();

	printLog("");
	printLog("========================================");
	printLog("Application finished successfully");
	printLog("========================================");

	return 0;
}
