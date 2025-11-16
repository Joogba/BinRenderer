#include "../../Vulkan/Application.h"
#include "../../Vulkan/EngineConfig.h"
#include "../../Vulkan/Logger.h"
#include "../../Vulkan/InputManager.h"

using namespace BinRenderer::Vulkan;

/**
 * @brief Input System 테스트 예제
 * 
 * IApplicationListener와 IInputListener를 다중 상속하여
 * 입력 이벤트를 처리하는 방법을 보여줍니다.
 */
class InputTestListener : public IApplicationListener, public IInputListener
{
public:
	void onInit(Scene& scene, Renderer& renderer) override
	{
		printLog("=== Input System Test ===");
		printLog("Press keys to test input system:");
		printLog("  - WASD: Move (handled by Application)");
		printLog("  - Arrow Keys: Custom input (handled by this listener)");
		printLog("  - Mouse Click: Print mouse position");
		printLog("  - Mouse Scroll: Print scroll amount");
		printLog("  - ESC: Exit");

		// 간단한 테스트 모델 추가
		const string helmetPath = "../../assets/models/DamagedHelmet.glb";
		glm::mat4 transform = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));
		scene.addModelInstance(helmetPath, "Test_Helmet", transform, renderer.getContext());

		// 카메라 설정
		auto& camera = scene.getCamera();
		camera.type = Camera::CameraType::firstperson;
		camera.position = glm::vec3(0.0f, 0.0f, -5.0f);
		camera.setMovementSpeed(5.0f);
		camera.setPerspective(75.0f, 1280.0f / 720.0f, 0.1f, 100.0f);
		camera.updateViewMatrix();
	}

	void onUpdate(float deltaTime, uint32_t frameIndex) override
	{
		// 매 프레임 업데이트
	}

	// ========================================
	// IInputListener 구현
	// ========================================

	void onKeyPressed(int key, int mods) override
	{
		switch (key) {
		case GLFW_KEY_UP:
			printLog("⬆️ UP Arrow pressed");
			break;
		case GLFW_KEY_DOWN:
			printLog("⬇️ DOWN Arrow pressed");
			break;
		case GLFW_KEY_LEFT:
			printLog("⬅️ LEFT Arrow pressed");
			break;
		case GLFW_KEY_RIGHT:
			printLog("➡️ RIGHT Arrow pressed");
			break;
		case GLFW_KEY_ENTER:
			printLog("✅ ENTER pressed");
			break;
		case GLFW_KEY_TAB:
			printLog("⇥ TAB pressed");
			break;
		case GLFW_KEY_I:
			printLog("ℹ️ INFO: This is a custom key handler!");
			break;
		}
	}

	void onKeyReleased(int key, int mods) override
	{
		// 키 릴리즈 처리
	}

	void onMouseButtonPressed(MouseButton button, double x, double y) override
	{
		const char* buttonName = "Unknown";
		switch (button) {
		case MouseButton::Left:
			buttonName = "LEFT";
			break;
		case MouseButton::Right:
			buttonName = "RIGHT";
			break;
		case MouseButton::Middle:
			buttonName = "MIDDLE";
			break;
		}

		printLog("🖱️ Mouse {} clicked at ({:.1f}, {:.1f})", buttonName, x, y);
	}

	void onMouseButtonReleased(MouseButton button, double x, double y) override
	{
		// 마우스 버튼 릴리즈 처리
	}

	void onMouseMoved(double x, double y, double deltaX, double deltaY) override
	{
		// 마우스 이동 처리 (너무 빈번하므로 로그 출력 안 함)
		
		// 예: 큰 이동만 출력
		if (std::abs(deltaX) > 50 || std::abs(deltaY) > 50) {
			printLog("🖱️ Large mouse movement: Δ({:.1f}, {:.1f})", deltaX, deltaY);
		}
	}

	void onMouseScrolled(double xOffset, double yOffset) override
	{
		printLog("🔄 Mouse scroll: ({:.1f}, {:.1f})", xOffset, yOffset);
	}

	void onGui() override
	{
		ImGui::SetNextWindowPos(ImVec2(10, 150), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("Input Test Controls")) {
			ImGui::End();
			return;
		}

		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Input System Active!");
		ImGui::Separator();

		ImGui::Text("Keyboard Controls:");
		ImGui::BulletText("WASD, Q, E: Camera movement (Application)");
		ImGui::BulletText("Arrow Keys: Custom handler (This listener)");
		ImGui::BulletText("I: Info message");
		ImGui::BulletText("ESC: Exit");

		ImGui::Separator();
		ImGui::Text("Mouse Controls:");
		ImGui::BulletText("Left Drag: Rotate camera (Application)");
		ImGui::BulletText("Right Drag: Move camera forward/back");
		ImGui::BulletText("Middle Drag: Pan camera");
		ImGui::BulletText("Scroll: Zoom in/out");

		ImGui::Separator();
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), 
			"💡 Tip: Check console for input events!");

		ImGui::End();
	}

	void onShutdown() override
	{
		printLog("Input test shutting down...");
	}
};

int main()
{
	try {
		// EngineConfig 생성
		auto config = EngineConfig::createDevelopment()
			.setWindowTitle("Input System Test")
			.setWindowSize(1280, 720);

		// Listener 생성
		InputTestListener listener;

		// Application 생성 및 실행
		Application app(config, &listener);
		app.run();

		return 0;
	}
	catch (const std::exception& e) {
		printLog("Fatal error: {}", e.what());
		return -1;
	}
}
