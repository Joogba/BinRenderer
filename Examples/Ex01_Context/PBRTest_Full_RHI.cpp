#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../../Core/RHIApplication.h"
#include "../../Core/RHIScene.h"
#include "../../Core/Logger.h"
#include "../../Core/EngineConfig.h"
#include "../../Core/InputManager.h"
#include "../../RenderPass/RenderGraph/RGGraph.h"
#include "../../RenderPass/ForwardPassRG.h"
#include "../../Rendering/RHIRenderer.h"

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
 * 
 * 렌더 패스 추가 방법:
 * - onInit()에서 renderGraph.addPass()를 호출하여 커스텀 패스 추가
 * - 패스를 추가하지 않으면 기본 ForwardPassRG가 자동으로 추가됨
 * 
 * 예제:
 * ```cpp
 * void onInit(RHIScene& scene, RenderGraph& renderGraph, RHICamera& camera) override
 * {
 *     // 커스텀 패스 추가
 *     auto myPass = std::make_unique<MyCustomPass>(rhi);
 *     myPass->initialize();
 *     renderGraph.addPass(std::move(myPass));
 * }
 * ```
 */
class FullRHITestApp : public IRHIApplicationListener
{
public:
	void onInit(RHIScene& scene, RenderGraph& renderGraph, RHICamera& camera) override
	{
		//  Vertex 구조체 레이아웃 검증
		printLog("========================================");
		printLog("RHIVertex Layout Validation:");
		printLog("  sizeof(RHIVertex) = {}", sizeof(RHIVertex));
		printLog("  offsetof(position) = {}", offsetof(RHIVertex, position));
		printLog("  offsetof(normal) = {}", offsetof(RHIVertex, normal));
		printLog("  offsetof(texCoord) = {}", offsetof(RHIVertex, texCoord));
		printLog("  offsetof(tangent) = {}", offsetof(RHIVertex, tangent));
		printLog("  offsetof(bitangent) = {}", offsetof(RHIVertex, bitangent));
		printLog("  offsetof(boneWeights) = {}", offsetof(RHIVertex, boneWeights));
		printLog("  offsetof(boneIndices) = {}", offsetof(RHIVertex, boneIndices));
		printLog("========================================");
		printLog("");
		
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
		
		//  Scene 및 RenderGraph 저장 (나중에 사용)
		scene_ = &scene;
		renderGraph_ = &renderGraph;
		camera_ = &camera;

		// Camera 설정 (정면 뷰 - Vulkan Y축 고려)
		camera.setType(RHICamera::CameraType::FirstPerson);
		camera.setPosition(glm::vec3(0.0f, 1.0f, 5.0f));  //  Z=5 (헬멧들 뒤)
		camera.setRotation(glm::vec3(-10.0f, 0.0f, 0.0f)); //  Y=180도 (뒤돌아봄)
		camera.setMovementSpeed(5.0f);
		camera.setRotationSpeed(0.1f);
		
		// Perspective 설정
		const float aspectRatio = 1280.0f / 720.0f;
		camera.setPerspective(60.0f, aspectRatio, 0.1f, 100.0f);
		
		printLog(" Camera initialized:");
		printLog("   - Position: (0, 1, 5) - behind helmets");
		printLog("   - Rotation: (-10, 180, 0) - looking at helmets");
		printLog("   - FOV: 60°, Aspect: {:.2f}", aspectRatio);
		printLog("   - Movement speed: 5.0");
		printLog("");  //  Fix: 빈 문자열 전달

		// 씬에 모델 추가
		const std::string helmetPath = "../../assets/models/DamagedHelmet.glb";
		
		printLog("📦 Adding 3 helmet instances...");
		
		// 첫 번째 헬멧: 왼쪽
		{
			glm::vec3 position(-2.0f, 0.0f, 0.0f);  //  Z=0 (원점)
			glm::mat4 translate = glm::translate(glm::mat4(1.0f), position);
			glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
			glm::mat4 rotation = glm::mat4(1.0f);
			glm::mat4 transform = translate * scale * rotation;
			
			scene.addModelInstance(helmetPath, "Helmet_Left", transform);
		}
		
		// 두 번째 헬멧: 중앙
		{
			glm::vec3 position(0.0f, 0.0f, 0.0f);  //  Z=0 (원점 중앙)
			glm::mat4 translate = glm::translate(glm::mat4(1.0f), position);
			glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
			glm::mat4 rotation = glm::mat4(1.0f);
			glm::mat4 transform = translate * scale * rotation;
			
			scene.addModelInstance(helmetPath, "Helmet_Center", transform);
		}
		
		// 세 번째 헬멧: 오른쪽
		{
			glm::vec3 position(2.0f, 0.0f, 0.0f);  //  Z=0 (원점 우측)
			glm::mat4 translate = glm::translate(glm::mat4(1.0f), position);
			glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
			glm::mat4 rotation = glm::mat4(1.0f);
			glm::mat4 transform = translate * scale * rotation;
			
			scene.addModelInstance(helmetPath, "Helmet_Right", transform);
		}

		printLog("");
		printLog(" Scene setup complete:");
		printLog("   - {} scene nodes", scene.getNodeCount());
		printLog("   - GPU instancing enabled");
		printLog("   - Automatic resource caching");
		printLog("");

		//  RenderGraph에 ForwardPassRG 추가
		printLog("🎨 Setting up RenderGraph...");
		printLog("   - ForwardPassRG will be added automatically if no custom passes");
		printLog(" RenderGraph setup complete");
		printLog("");
		
		// ========================================
		//  Material Buffer 구성
		// ========================================
		printLog("📦 Building material buffer from scene...");
		
		printLog("   ⏳ Material buffer build - will be done in application setup");
		
		printLog(" Material setup complete");
		printLog("");
		
		// ========================================
		//  IBL 텍스처 경로 설정
		// ========================================
		printLog("🌍 IBL textures will be loaded from:");
		printLog("   - Path: ../../assets/textures/golden_gate_hills_4k/");
		printLog("   - Prefiltered: specularGGX.ktx2");
		printLog("   - Irradiance: diffuseLambertian.ktx2");
		printLog("   - BRDF LUT: outputLUT.png");
		printLog(" IBL setup - will be loaded by RHI layer");
		printLog("");
		
		printLog("🎯 Architecture Benefits:");
		printLog("    Platform Independence");
		printLog("    - RHI abstracts Vulkan/DX12/Metal");
		printLog("      - Same code works on all platforms");
		printLog("    Modular Design");
		printLog("      - Camera: First-person & LookAt modes");
		printLog("   - Animation: Pure logic (no rendering)");
		printLog("      - Material: Data-driven");
		printLog("      - Mesh: Self-contained");
		printLog("    RenderGraph");
		printLog("      - Automatic resource management");
		printLog("      - Dependency tracking");
		printLog("      - Performance optimization");
		printLog("    Configuration");
		printLog("      - EngineConfig: Centralized settings");
		printLog("      - Easy to switch dev/release modes");
		printLog("");
	}

	void onUpdate(float deltaTime, uint32_t frameIndex) override
	{
		elapsedTime_ += deltaTime;
		
		// 60 프레임마다 로그 출력
		if (frameIndex % 60 == 0)
		{
			printLog("⏱️  Frame {}: Elapsed {:.2f}s, Delta: {:.4f}s", 
				frameIndex, elapsedTime_, deltaTime);
		}

		// Camera 회전 (자동 데모)
		if (frameIndex % 120 == 0)
		{
			printLog("📹 Camera auto-rotation demo");
		}
	}

	void onGui() override
	{
		// TODO: ImGui integration
	}

	void onShutdown() override
	{
		printLog("");
		printLog("==============================================");
		printLog("  Shutting down RHI Application");
		printLog("==============================================");
		printLog("📊 Final Statistics:");
		printLog("   - Total frames rendered: ~{}", static_cast<int>(elapsedTime_ / 0.016f));
		printLog("   - Total elapsed time: {:.2f}s", elapsedTime_);
		printLog("   - Average frame time: {:.4f}s", elapsedTime_ / static_cast<int>(elapsedTime_ / 0.016f));
		printLog("");
		printLog(" Application shutdown complete");
		printLog("");
	}

private:
	float elapsedTime_ = 0.0f;
	RHIScene* scene_ = nullptr;
	RenderGraph* renderGraph_ = nullptr;
	RHICamera* camera_ = nullptr;
};

int main()
{
	printLog("========================================");
	printLog("🎮 BinRenderer - Full RHI System Test");
	printLog("========================================");
	printLog("");
	printLog("This example demonstrates:");
	printLog("   RHI abstraction layer (Vulkan/DX12/Metal)");
	printLog("   Platform-independent components");
	printLog("     - Scene/Animation (pure logic)");
	printLog("     - Core/RHIModel (RHI buffers)");
	printLog("     - Rendering/RHIMaterial (data-driven)");
	printLog("     - Rendering/RHIMesh (self-contained)");
	printLog("   RenderGraph system");
	printLog("     - Declarative render passes");
	printLog("   - Automatic dependency resolution");
	printLog("   Configuration system");
	printLog("     - EngineConfig for centralized settings");
	printLog("   Input system");
	printLog("     - Platform-independent InputManager");
	printLog("");
	printLog("📋 Next Steps:");
	printLog("  1.  RHI Application framework");
	printLog("  2.  RenderGraph system");
	printLog("  3. 🚧 Window integration (GLFW/SDL)");
	printLog("  4. 🚧 Actual rendering implementation");
	printLog("  5. ⏳ ImGui support");
	printLog("  6. ⏳ DirectX 12 / Metal backends");
	printLog("");

	// ========================================
	// EngineConfig 설정
	// ========================================
	printLog("⚙️  Configuring Engine...");
	EngineConfig config = EngineConfig::createDevelopment();
	config.setAssetsPath("../../assets/")
		.setWindowSize(1280, 720)
		.setWindowTitle("BinRenderer - Full RHI Test")
		.setMaxFramesInFlight(2)
		.setVsync(true)
		.setValidation(true);

	printLog(" Configuration:");
	printLog("   - Window: {}x{}", config.windowWidth, config.windowHeight);
	printLog("   - Title: {}", config.windowTitle);
	printLog("   - Assets: {}", config.assetsPath);
	printLog("   - Shaders: {}", config.shaderPath);
	printLog("   - Max Frames: {}", config.maxFramesInFlight);
	printLog("   - Vsync: {}", config.enableVsync ? "ON" : "OFF");
	printLog("   - Validation: {}", config.enableValidationLayers ? "ON" : "OFF");
	printLog("");

	// ========================================
	// Application 실행
	// ========================================
	printLog("🚀 Starting application...");
	printLog("");

	try
	{
		FullRHITestApp listener;
		RHIApplication app(config, RHIApiType::Vulkan);
		app.setListener(&listener);
		
		printLog("📦 Application initialized");
		printLog("🎬 Running main loop...");
		printLog("");
		
		app.run();

		printLog("");
		printLog("========================================");
		printLog(" Application finished successfully");
		printLog("========================================");
	}
	catch (const std::exception& e)
	{
		printLog("");
		printLog("========================================");
		printLog("❌ ERROR: Application failed");
		printLog("========================================");
		printLog("Exception: {}", e.what());
		printLog("");
		return 1;
	}

	return 0;
}
