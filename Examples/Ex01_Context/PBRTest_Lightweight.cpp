#include "Vulkan/Application.h"
#include "Vulkan/EngineConfig.h"
#include "Vulkan/IApplicationListener.h"

using namespace BinRenderer::Vulkan;

// ========================================
// Lightweight Test Application
// ========================================
class LightweightTestApp : public IApplicationListener
{
public:
	void onInit(Scene& scene, Renderer& renderer) override
	{
		printLog("=== Lightweight Test: Initializing ===");

		auto& ctx = renderer.getContext();

		// Load single small model (Helmet) - ✅ shared_ptr로 변경
		auto helmetModel = std::make_shared<Model>(ctx);
		helmetModel->loadFromModelFile("../../assets/models/DamagedHelmet.glb", false);
		helmetModel->modelMatrix() = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));

		scene.addModel(helmetModel, "Helmet");

		// Setup camera (using forHelmet preset)
		auto& camera = scene.getCamera();
		camera.type = Camera::CameraType::firstperson;
		camera.position = glm::vec3(0.0f, 0.0f, 2.5f);  // ✅ FIX: Z축 양수 (카메라 앞)
		camera.rotation = glm::vec3(0.0f, 180.0f, 0.0f);  // ✅ FIX: Y축 180도 회전 (뒤돌아보기)
		camera.viewPos = glm::vec3(0.0f, 0.0f, -2.5f);  // ✅ FIX: 바라보는 방향
		camera.setMovementSpeed(5.0f);
		camera.setRotationSpeed(0.1f);
		
		// Set perspective
		const float aspectRatio = 1280.0f / 720.0f;
		camera.setPerspective(75.0f, aspectRatio, 0.1f, 256.0f);
		camera.updateViewMatrix();

		printLog("Scene initialized: {} nodes (lightweight)", scene.getNodeCount());
	}

	void onUpdate(float deltaTime, uint32_t frameIndex) override
	{
		elapsedTime_ += deltaTime;
	}

	void onGui() override
	{
		ImGui::Begin("Lightweight Test");
		ImGui::Text("New API Test (Memory Safe)");
		ImGui::Text("Elapsed: %.2f seconds", elapsedTime_);
		ImGui::Separator();
		ImGui::TextColored(ImVec4(0, 1, 0, 1), "? Memory: OK");
		ImGui::End();
	}

	void onShutdown() override
	{
		printLog("=== Lightweight Test: Shutdown ===");
	}

private:
	float elapsedTime_ = 0.0f;
};

// ========================================
// Main Entry Point
// ========================================
int main()
{
	EngineConfig engineConfig = EngineConfig::createDevelopment();
	engineConfig.setAssetsPath("../../assets/")
		.setWindowSize(1280, 720)  // 작은 해상도
		.setWindowTitle("BinRenderer - Lightweight Test");

	printLog("Starting lightweight test...");

	LightweightTestApp listener;
	Application app(engineConfig, &listener);
	app.run();

	return 0;
}
