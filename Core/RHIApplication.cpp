#include "RHIApplication.h"
#include "RHIScene.h"
#include "Logger.h"
#include "../Platform/WindowFactory.h"
#include "../RenderPass/ForwardPassRG.h"
#include <chrono>
#include <memory>

namespace BinRenderer
{
	RHIApplication::RHIApplication(const EngineConfig& config, RHIApiType apiType)
		: config_(config), apiType_(apiType)
	{
	}

	RHIApplication::~RHIApplication()
	{
		shutdown();
	}

	void RHIApplication::run()
	{
		initialize();

		if (!initialized_)
		{
			printLog("ERROR: RHIApplication initialization failed");
			return;
		}

		// 리스너 초기화 (씬 구성, RenderGraph 커스터마이징)
		if (listener_)
		{
			listener_->onInit(*scene_, *renderGraph_, camera_);
		}

		// 사용자가 렌더 패스를 추가하지 않았다면 기본 ForwardPassRG 추가
		if (renderGraph_->getPassCount() == 0)
		{
			printLog("No render passes added by user, adding default ForwardPassRG");
			auto forwardPass = std::make_unique<ForwardPassRG>(rhi_.get(), scene_.get(), renderer_.get());
			if (forwardPass->initialize())
			{
				renderGraph_->addPass(std::move(forwardPass));
				printLog("    Default ForwardPassRG added (with Scene and Renderer)");
			}
			else
			{
				printLog("   ❌ Failed to initialize default ForwardPassRG");
			}
		}
		else
		{
			printLog("Using user-defined render passes ({} pass(es))", renderGraph_->getPassCount());
		}

		// ========================================
		//  Material Buffer 빌드 및 ForwardPass에 바인딩
		// ========================================
		printLog("Building material buffer from scene...");
		if (renderer_ && scene_)
		{
			// Scene의 모든 모델에서 material 데이터 수집 및 GPU 버퍼 생성
			renderer_->buildMaterialBuffer(*scene_);
			printLog("    Material buffer built: {} materials", renderer_->getMaterialCount());
		}
		
		// RenderGraph 컴파일 (모든 Pass 추가 후)
		renderGraph_->compile();
		printLog(" RenderGraph compiled");

		// 메인 루프 실행
		mainLoop();
	}

	// ========================================
	// 초기화
	// ========================================

	void RHIApplication::initialize()
	{
		printLog("=== RHIApplication::initialize ===");
		printLog("API: {}", apiType_ == RHIApiType::Vulkan ? "Vulkan" : "Unknown");
		printLog("Window: {}x{}", config_.windowWidth, config_.windowHeight);
		printLog("Title: {}", config_.windowTitle);

		// 1. Window 생성 (플랫폼 독립적)
		window_ = WindowFactory::create(WindowBackend::Auto);
		if (!window_)
		{
			printLog("ERROR: Failed to create window factory");
			return;
		}

		if (!window_->create(config_.windowWidth, config_.windowHeight, config_.windowTitle))
		{
			printLog("ERROR: Failed to create window");
			return;
		}
		printLog(" Window created");

		// 2. RHI 생성
		rhi_ = RHIFactory::createUnique(apiType_);
		if (!rhi_)
		{
			printLog("ERROR: Failed to create RHI");
			return;
		}

		// 3. RHI 초기화 (Window 전달)
		RHIInitInfo initInfo{};
		initInfo.windowWidth = config_.windowWidth;
		initInfo.windowHeight = config_.windowHeight;
		initInfo.window = window_->getNativeHandle();  // 레거시 (VulkanRHI 내부에서 사용 안 함)
		initInfo.windowInterface = window_.get();  //  IWindow 인터페이스 전달!
		initInfo.maxFramesInFlight = config_.maxFramesInFlight;
		
		// Vulkan 확장 (Window에서 가져오기)
		uint32_t extensionCount = 0;
		const char** extensions = window_->getRequiredExtensions(extensionCount);
		initInfo.requiredInstanceExtensions.assign(extensions, extensions + extensionCount);
		initInfo.enableValidationLayer = config_.enableValidationLayers;
		
		if (!rhi_->initialize(initInfo))
		{
			printLog("ERROR: Failed to initialize RHI");
			return;
		}
		printLog(" RHI initialized");

		// 4. Renderer 생성
		renderer_ = std::make_unique<RHIRenderer>(rhi_.get(), config_.maxFramesInFlight);

		// Swapchain 포맷 가져오기
		RHIFormat colorFormat = RHI_FORMAT_B8G8R8A8_UNORM;
		RHIFormat depthFormat = RHI_FORMAT_D32_SFLOAT;

		auto* swapchain = rhi_->getSwapchain();
		if (swapchain)
		{
			// Swapchain이 있으면 실제 포맷 사용
			// colorFormat = swapchain->getFormat(); // TODO: RHISwapchain에 getFormat() 추가 필요
			printLog("📺 Using swapchain color format");
		}
		else
		{
			printLog("ERROR: Swapchain is null!");
			return;
		}

		if (!renderer_->initialize(config_.windowWidth, config_.windowHeight, colorFormat, depthFormat))
		{
			printLog("ERROR: Failed to initialize Renderer");
			return;
		}
		printLog(" Renderer initialized");

		// 5. Camera 초기화
		float aspect = static_cast<float>(config_.windowWidth) / static_cast<float>(config_.windowHeight);
		camera_.setPerspective(60.0f, aspect, 0.1f, 1000.0f);
		camera_.setPosition(glm::vec3(0.0f, 5.0f, -10.0f));
		printLog(" Camera initialized");

		// 6. Scene 생성
		scene_ = std::make_unique<RHIScene>(rhi_.get());
		scene_->setCamera(camera_);
		printLog(" Scene created");

		// 7. RenderGraph 생성
		renderGraph_ = std::make_unique<RenderGraph>(rhi_.get());
		setupDefaultRenderGraph();
		printLog(" RenderGraph created");

		// 8. Input 시스템 초기화
		// TODO: Window 핸들 전달
		// inputManager_.initialize(window_.get());

		initialized_ = true;
		running_ = true;
		printLog(" RHIApplication initialized successfully");
	}

	void RHIApplication::shutdown()
	{
		if (!initialized_)
			return;

		printLog("=== RHIApplication::shutdown ===");

		running_ = false;

		// 1. GPU 작업 완료 대기 (가장 먼저)
		if (rhi_)
		{
			try
			{
				rhi_->waitIdle();
			}
			catch (const std::exception& e)
			{
				printLog("⚠️  Warning: RHI waitIdle failed: {}", e.what());
			}
		}

		// 2. 리스너 종료
		if (listener_)
		{
			listener_->onShutdown();
		}

		// 3. RenderGraph 정리 (RHI 리소스 사용)
		if (renderGraph_)
		{
			printLog("   Cleaning up RenderGraph...");
			renderGraph_.reset();
		}

		// 4. Renderer 정리 (RHI 리소스 사용)
		if (renderer_)
		{
			printLog("   Cleaning up Renderer...");
			renderer_->shutdown();
			renderer_.reset();
		}

		// 5. Scene 정리 (RHI 리소스 사용)
		if (scene_)
		{
			printLog("   Cleaning up Scene...");
			scene_.reset();
		}

		// 6. RHI 정리 (모든 리소스가 정리된 후)
		if (rhi_)
		{
			printLog("   Shutting down RHI...");
			rhi_->shutdown();
			rhi_.reset();
		}

		// 7. Window 정리 (마지막)
		if (window_)
		{
			printLog("   Destroying Window...");
			window_->destroy();
			window_.reset();
		}

		initialized_ = false;
		printLog(" RHIApplication shutdown complete");
	}

	void RHIApplication::setupDefaultRenderGraph()
	{
		// 기본 ForwardPassRG는 사용자가 패스를 추가하지 않았을 때만 추가
		printLog("Default RenderGraph setup");
		
		// 이 시점에서는 listener의 onInit이 호출되기 전이므로
		// setupDefaultRenderGraph를 제거하고 listener 호출 후에 체크하도록 수정 필요
		// 현재는 주석 처리
	}

	// ========================================
	// 메인 루프
	// ========================================

	void RHIApplication::mainLoop()
	{
		printLog("=== Starting main loop ===");

		lastFrameTime_ = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

		// 플랫폼 독립적 이벤트 루프
		while (!window_->shouldClose() && running_)
		{
			// 이벤트 폴링
			window_->pollEvents();

			// Delta time 계산
			auto currentTime = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
			deltaTime_ = static_cast<float>(currentTime - lastFrameTime_);
			lastFrameTime_ = currentTime;

			// 프레임 업데이트
			updateFrame(deltaTime_);

			// 렌더링
			uint32_t imageIndex = 0;
			if (rhi_->beginFrame(imageIndex))
			{
				renderFrame(frameIndex_);
				rhi_->endFrame(imageIndex);
			}

			frameIndex_++;

			// 60 프레임마다 로그
			if (frameIndex_ % 60 == 0)
			{
				printLog("⏱️  Frame {}: {:.2f} FPS", frameIndex_, 1.0f / deltaTime_);
			}
		}

		printLog("=== Main loop finished ({} frames) ===", frameIndex_);
	}

	void RHIApplication::updateFrame(float deltaTime)
	{
		// Input 업데이트
		inputManager_.update();

		// Camera 업데이트
		camera_.update(deltaTime);

		// Scene 업데이트 (애니메이션 등)
		if (scene_)
		{
			scene_->update(deltaTime);
		}

		// Renderer uniform 업데이트
		if (renderer_ && scene_)
		{
			uint32_t currentFrame = frameIndex_ % config_.maxFramesInFlight;
			renderer_->updateUniforms(camera_, *scene_, currentFrame, lastFrameTime_);
		}

		// 리스너 업데이트
		if (listener_)
		{
			listener_->onUpdate(deltaTime, frameIndex_);
		}
	}

	void RHIApplication::renderFrame(uint32_t frameIndex)
	{
		// RenderGraph 실행
		if (renderGraph_)
		{
			uint32_t currentFrame = frameIndex % config_.maxFramesInFlight;
			renderGraph_->execute(currentFrame);
		}

		// GUI 렌더링
		if (listener_)
		{
			listener_->onGui();
		}
	}

} // namespace BinRenderer
