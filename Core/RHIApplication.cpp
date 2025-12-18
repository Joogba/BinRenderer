#include "RHIApplication.h"
#include "RHIScene.h"
#include "Logger.h"

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

	void RHIApplication::setListener(IRHIApplicationListener* listener)
	{
		listener_ = listener;
	}

	void RHIApplication::run()
	{
		initialize();

		if (!initialized_)
		{
			printLog("RHIApplication initialization failed");
			return;
		}

		if (listener_)
		{
			listener_->onInit(*scene_, *renderGraph_, camera_);
		}

		mainLoop();
	}

	void RHIApplication::initialize()
	{
		printLog("=== RHIApplication::initialize ===");
		printLog("API: {}", apiType_ == RHIApiType::Vulkan ? "Vulkan" : "Unknown");
		printLog("Window: {}x{}", config_.windowWidth, config_.windowHeight);
		printLog("Title: {}", config_.windowTitle);
		printLog("Assets: {}", config_.assetsPath);
		printLog("Shaders: {}", config_.shaderPath);

		// RHI 생성
		rhi_ = RHIFactory::createUnique(apiType_);
		if (!rhi_)
		{
			printLog("ERROR: Failed to create RHI");
			return;
		}

		// RHI 초기화
		RHIInitInfo initInfo{};
		// TODO: 윈도우 핸들 및 추가 설정
		
		if (!rhi_->initialize(initInfo))
		{
			printLog("ERROR: Failed to initialize RHI");
			return;
		}

		// Camera 초기화
		float aspect = static_cast<float>(config_.windowWidth) / static_cast<float>(config_.windowHeight);
		camera_.setPerspective(60.0f, aspect, 0.1f, 1000.0f);
		camera_.setPosition(glm::vec3(0.0f, 0.0f, -10.0f));

		// Scene 생성
		scene_ = std::make_unique<RHIScene>(rhi_.get());

		// RenderGraph 생성
		renderGraph_ = std::make_unique<RenderGraph>(rhi_.get());
		
		// 기본 RenderGraph 설정
		setupDefaultRenderGraph();

		initialized_ = true;
		printLog("RHIApplication initialized successfully");
	}

	void RHIApplication::setupDefaultRenderGraph()
	{
		// Forward Pass 추가
		auto forwardPass = std::make_unique<RHIForwardPassRG>(rhi_.get());
		forwardPass->setScene(scene_.get());
		
		// 포인터 저장 (RenderGraph가 소유권을 가져가기 전에)
		forwardPass_ = forwardPass.get();
		
		// RenderGraph에 패스 추가 (소유권 이전)
		renderGraph_->addPass(std::move(forwardPass));
		
		printLog("Forward pass added to RenderGraph");

		// RenderGraph 컴파일
		renderGraph_->compile();
		printLog("RenderGraph compiled");
	}

	void RHIApplication::shutdown()
	{
		if (!initialized_)
			return;

		printLog("=== RHIApplication::shutdown ===");

		if (listener_)
		{
			listener_->onShutdown();
		}

		// Forward pass는 RenderGraph가 소유하므로 삭제하지 않음
		forwardPass_ = nullptr;

		scene_.reset();
		renderGraph_.reset();

		if (rhi_)
		{
			rhi_->waitIdle();
			rhi_->shutdown();
		}

		rhi_.reset();

		initialized_ = false;
		printLog("RHIApplication shutdown complete");
	}

	void RHIApplication::mainLoop()
	{
		printLog("=== Starting main loop ===");
		
		// TODO: 실제 윈도우 루프
		// 현재는 테스트용 고정 프레임
		const int maxFrames = 10;
		int frameCount = 0;

		while (frameCount < maxFrames)
		{
			uint32_t imageIndex = 0;
			
			// Input 업데이트
			inputManager_.update();

			// Camera 업데이트
			camera_.update(deltaTime_);

			// 프레임 시작
			if (rhi_->beginFrame(imageIndex))
			{
				// Scene 업데이트
				scene_->update(deltaTime_);

				// 리스너 업데이트
				if (listener_)
				{
					listener_->onUpdate(deltaTime_, frameIndex_);
					listener_->onGui();
				}

				// RenderGraph 실행
				renderGraph_->execute(frameIndex_);

				// 프레임 종료
				rhi_->endFrame(imageIndex);
			}

			frameIndex_++;
			frameCount++;
			deltaTime_ = 0.016f; // 60 FPS
		}

		printLog("Main loop finished ({} frames)", frameCount);
	}

} // namespace BinRenderer
