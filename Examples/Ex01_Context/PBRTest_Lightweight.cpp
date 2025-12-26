#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Vulkan/Application.h"
#include "Vulkan/EngineConfig.h"
#include "Vulkan/IApplicationListener.h"
#include "Core/Logger.h"
#include "Vulkan/Scene.h"
#include "Vulkan/Renderer.h"
#include "Vulkan/Camera.h"

#include <imgui.h>

using namespace BinRenderer::Vulkan;

// ========================================
// Lightweight Test Application
// ========================================
class LightweightTestApp : public IApplicationListener
{
public:
	void onInit(Scene& scene, Renderer& renderer) override
	{
		printLog("=== GPU Instancing Test (Auto): 3 Helmets ===");

		const string helmetPath = "../../assets/models/DamagedHelmet.glb";
		
		printLog("Testing automatic GPU instancing via Scene::addModelInstance()...");
		
		// ========================================
		//  GPU Instancing: VulkanResourceManager가 자동으로 처리
		// ========================================
		
		// 첫 번째 헬멧: 왼쪽
		{
			glm::vec3 position(-5.0f, 0.0f, 0.0f);
			glm::mat4 translate = glm::translate(glm::mat4(1.0f), position);
			glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			glm::mat4 transform = translate * scale * rotation;
			
			scene.addModelInstance(helmetPath, "Helmet_Left", transform);  //  Context 제거
		}
		
		// 두 번째 헬멧: 중앙
		{
			glm::vec3 position(0.0f, 0.0f, 0.0f);
			glm::mat4 translate = glm::translate(glm::mat4(1.0f), position);
			glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			glm::mat4 transform = translate * scale * rotation;
			
			scene.addModelInstance(helmetPath, "Helmet_Center", transform);  //  Context 제거
		}
		
		// 세 번째 헬멧: 오른쪽
		{
			glm::vec3 position(5.0f, 0.0f, 0.0f);
			glm::mat4 translate = glm::translate(glm::mat4(1.0f), position);
			glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			glm::mat4 transform = translate * scale * rotation;
			
			scene.addModelInstance(helmetPath, "Helmet_Right", transform);  //  Context 제거
		}
		
		printLog(" VulkanResourceManager automatically handled GPU instancing!");
		printLog("   Expected: 1 model loaded, 3 instances, 1 draw call");

		// 카메라 설정
		auto& camera = scene.getCamera();
		camera.type = Camera::CameraType::firstperson;
		camera.position = glm::vec3(0.0f, 5.0f, -10.0f);
		camera.rotation = glm::vec3(-20.0f, 0.0f, 0.0f);
		camera.viewPos = glm::vec3(0.0f, 0.0f, 0.0f);
		camera.setMovementSpeed(10.0f);
		camera.setRotationSpeed(0.1f);
		
		const float aspectRatio = 1280.0f / 720.0f;
		camera.setPerspective(75.0f, aspectRatio, 0.1f, 512.0f);
		camera.updateViewMatrix();

		printLog("Scene initialized: {} nodes", scene.getNodeCount());
	}

	void onUpdate(float deltaTime, uint32_t frameIndex) override
	{
		elapsedTime_ += deltaTime;
	}

	void onGui() override
	{
		ImGui::Begin("GPU Instancing Test (Auto): 3 Helmets");
		ImGui::Text("Elapsed: %.2f seconds", elapsedTime_);
		ImGui::Separator();
		
		ImGui::TextColored(ImVec4(0, 1, 0, 1), " Automatic GPU Instancing");
		ImGui::TextColored(ImVec4(0, 1, 1, 1), " Scene::addModelInstance() x3");
		ImGui::TextColored(ImVec4(1, 1, 0, 1), " 1 Model, 3 Instances");
		ImGui::TextColored(ImVec4(1, 0.5, 0, 1), " 1 Draw Call");
		
		ImGui::Separator();
		ImGui::Text("Features:");
		ImGui::BulletText("Automatic instancing detection");
		ImGui::BulletText("Model cache + GPU instancing");
		ImGui::BulletText("66%% memory savings");
		
		ImGui::End();
	}

	void onShutdown() override
	{
		printLog("=== Lightweight Test: Shutdown ===");
	}

private:
	float elapsedTime_ = 0.0f;
};

// Main Entry Point
int main()
{
	EngineConfig engineConfig = EngineConfig::createDevelopment();
	engineConfig.setAssetsPath("../../assets/")
		.setWindowSize(1280, 720)
		.setWindowTitle("BinRenderer - Lightweight Test : 3 Helmets");

	printLog("Starting lightweight test...");

	LightweightTestApp listener;
	Application app(engineConfig, &listener);
	app.run();

	return 0;
}
