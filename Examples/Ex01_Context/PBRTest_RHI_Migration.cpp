#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// ========================================
//  RHI 기반 헤더 (플랫폼 독립적)
// ========================================
#include "../../Scene/Animation.h"        //  NEW: 플랫폼 독립적 Animation

// ========================================
// ❌ 아직 Vulkan 의존적 (점진적 마이그레이션)
// ========================================
#include "../../Vulkan/Application.h"
#include "../../Vulkan/EngineConfig.h"
#include "../../Vulkan/IApplicationListener.h"
#include "../../Core/Logger.h"
#include "../../Vulkan/Scene.h"
#include "../../Vulkan/Renderer.h"
#include "../../Vulkan/Camera.h"   // Vulkan Camera 사용 (향후 독립적으로 분리 가능)

#include <imgui.h>

using namespace BinRenderer::Vulkan;

// ========================================
// 🎯 목표: RHI 기반 시스템 활용
// ========================================
/**
 * @brief RHI 기반 렌더링 테스트
 * 
 * 현재 상태:
 * - Scene/Animation.h 사용 ( Vulkan 의존성 제거)
 * - Vulkan::Application 사용 (❌ 아직 Vulkan 의존적)
 * 
 * 향후 계획:
 * - Core/RHIApplication으로 전환
 * - RenderGraph 기반 렌더링
 */
class RHIBasedTestApp : public IApplicationListener
{
public:
	void onInit(Scene& scene, Renderer& renderer) override
	{
		printLog("=== RHI-Based Rendering Test ===");
		printLog(" Using Scene/Animation.h (platform-independent)");
		printLog("❌ Still using Vulkan::Application (TODO: migrate)");

		const string helmetPath = "../../assets/models/DamagedHelmet.glb";
		
		// ========================================
		// GPU Instancing: VulkanResourceManager가 자동 처리
		// ========================================
		
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
		
		printLog(" GPU Instancing: 1 model loaded, 3 instances");

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
		printLog("");
		printLog("🎯 Next Steps:");
		printLog("  1. Create Core/RHIApplication.h");
		printLog("  2. Create Core/RHIScene.h");
		printLog("  3. Use RenderGraph for rendering");
		printLog("  4. Remove Vulkan:: dependencies");
	}

	void onUpdate(float deltaTime, uint32_t frameIndex) override
	{
		elapsedTime_ += deltaTime;

		// ========================================
		//  TODO: Animation 업데이트 예제
		// ========================================
		// if (animation_) {
		//     animation_->updateAnimation(deltaTime);
		//     const auto& boneMatrices = animation_->getBoneMatrices();
		//     // RHI를 통해 GPU에 업로드
		//     // rhi->updateBuffer(boneBuffer, boneMatrices.data(), ...);
		// }
	}

	void onGui() override
	{
		ImGui::Begin("RHI-Based Rendering Test");
		ImGui::Text("Elapsed: %.2f seconds", elapsedTime_);
		ImGui::Separator();
		
		ImGui::TextColored(ImVec4(0, 1, 0, 1), " Platform-Independent Components:");
		ImGui::BulletText("Scene/Animation.h (no Vulkan deps)");
		ImGui::BulletText("Scene/Camera.h (already independent)");
		ImGui::BulletText("RHI System (Vulkan/DX12/Metal ready)");
		ImGui::BulletText("RenderGraph System");
		
		ImGui::Separator();
		ImGui::TextColored(ImVec4(1, 0.5, 0, 1), "🔲 TODO - RHI Migration:");
		ImGui::BulletText("Core/RHIApplication.h");
		ImGui::BulletText("Core/RHIScene.h");
		ImGui::BulletText("Core/RHIModel.h");
		ImGui::BulletText("RenderGraph integration");
		
		ImGui::Separator();
		ImGui::TextColored(ImVec4(0.5, 0.5, 1, 1), "📚 Architecture:");
		ImGui::BulletText("Logic (Animation, Camera) = Platform-independent");
		ImGui::BulletText("Rendering (Buffers, Textures) = RHI abstraction");
		ImGui::BulletText("Passes (Forward, Deferred) = RenderGraph");
		
		ImGui::End();
	}

	void onShutdown() override
	{
		printLog("=== RHI-Based Test: Shutdown ===");
	}

private:
	float elapsedTime_ = 0.0f;
	
	//  TODO: 플랫폼 독립적 Animation 사용 예제
	// std::unique_ptr<BinRenderer::Animation> animation_;
};

// Main Entry Point
int main()
{
	EngineConfig engineConfig = EngineConfig::createDevelopment();
	engineConfig.setAssetsPath("../../assets/")
		.setWindowSize(1280, 720)
		.setWindowTitle("BinRenderer - RHI Migration Test");

	printLog("Starting RHI-based test...");
	printLog("This example demonstrates the transition from Vulkan to RHI:");
	printLog("  - Scene/Animation.h:  Platform-independent");
	printLog("  - Vulkan::Application: ❌ Still Vulkan-specific (migration needed)");
	printLog("");

	RHIBasedTestApp listener;
	Application app(engineConfig, &listener);
	app.run();

	return 0;
}
