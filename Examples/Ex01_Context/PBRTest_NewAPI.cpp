#include "Vulkan/Application.h"
#include "Vulkan/EngineConfig.h"
#include "Vulkan/IApplicationListener.h"

using namespace BinRenderer::Vulkan;

// ========================================
// Custom Application Listener Example
// ========================================
class MyAppListener : public IApplicationListener
{
public:
	void onInit(Scene& scene, Renderer& renderer) override
	{
		printLog("=== MyApp: Initializing Scene ===");

		// Add Dancer model
		auto& ctx = renderer.getContext();

		auto dancerModel = std::make_unique<Model>(ctx);
		dancerModel->loadFromModelFile("../../assets/characters/Leonard/Bboy Hip Hop Move.fbx", false);
		dancerModel->modelMatrix() = glm::rotate(
			glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-6.719f, 0.21f, -1.860f)),
				glm::vec3(0.012f)),
			glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		if (dancerModel->hasAnimations()) {
			dancerModel->playAnimation();
			printLog("Dancer animation started!");
		}

		scene.addModel(std::move(dancerModel), "Dancer");

		// ========================================
		// TEMP FIX: Bistro 씬 비활성화 (메모리 부족 방지)
		// ========================================
		/*
				auto bistroModel = std::make_unique<Model>(ctx);
			bistroModel->loadFromModelFile("../../assets/models/AmazonLumberyardBistroMorganMcGuire/exterior.obj", true);
			bistroModel->modelMatrix() = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f));
			scene.addModel(std::move(bistroModel), "Bistro");
		*/

		printLog("NOTE: Bistro scene disabled to prevent GPU memory exhaustion");

		// Setup camera (Dancer 중심으로 조정)
		auto& camera = scene.getCamera();
		camera.type = Camera::CameraType::firstperson;
		camera.position = glm::vec3(0.0f, 2.0f, 5.0f);  // Dancer를 바라보는 위치
		camera.rotation = glm::vec3(0.0f, 180.0f, 0.0f);
		camera.viewPos = glm::vec3(0.0f, 2.0f, -5.0f);
		camera.setMovementSpeed(5.0f);
		camera.setRotationSpeed(0.1f);
		camera.updateViewMatrix();

		printLog("Scene initialized: {} nodes", scene.getNodeCount());
	}

	void onUpdate(float deltaTime, uint32_t frameIndex) override
	{
		// Custom game logic here
		// Example: Rotate a model every frame
		elapsedTime_ += deltaTime;
	}

	void onGui() override
	{
		ImGui::Begin("My Custom Window");
		ImGui::Text("Welcome to New API Example!");
		ImGui::Text("Elapsed Time: %.2f seconds", elapsedTime_);
		ImGui::Separator();
		ImGui::Text("This is a custom GUI from IApplicationListener");
		ImGui::End();
	}

	void onShutdown() override
	{
		printLog("=== MyApp: Shutting down ===");
	}

private:
	float elapsedTime_ = 0.0f;
};

// ========================================
// Main Entry Point (New API)
// ========================================
int main()
{
	// Configure engine
	EngineConfig engineConfig = EngineConfig::createDevelopment();
	engineConfig.setAssetsPath("../../assets/")
		.setWindowSize(1920, 1080)
		.setWindowTitle("BinRenderer - New API Example");

	printLog("Starting BinRenderer with New API...");

	// Create listener
	MyAppListener listener;

	// Create and run application
	Application app(engineConfig, &listener);
	app.run();

	return 0;
}
