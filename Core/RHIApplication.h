#pragma once

#include "../RHI/Core/RHI.h"
#include "../RHI/Util/RHIFactory.h"
#include "../Rendering/RHIRenderer.h"
#include "RHIScene.h"
#include "../RenderPass/RenderGraph/RGGraph.h"
#include "../Scene/Animation.h"
#include "../Scene/RHICamera.h"
#include "../Platform/IWindow.h"
#include "EngineConfig.h"
#include "InputManager.h"
#include <memory>
#include <string>
#include <functional>

namespace BinRenderer
{
	/**
	 * @brief RHI Application 리스너 인터페이스
	 * 
	 * 사용자는 이 인터페이스를 구현하여:
	 * - 씬 구성 (onInit)
	 * - RenderGraph에 커스텀 Pass 추가
	 * - 매 프레임 업데이트 (onUpdate)
	 */
	class IRHIApplicationListener
	{
	public:
		virtual ~IRHIApplicationListener() = default;

		/**
		 * @brief 초기화 (씬 구성, RenderGraph 설정)
		 * 
		 * @param scene 씬 (모델 추가)
		 * @param renderGraph RenderGraph (커스텀 Pass 추가)
		 * @param camera 카메라 (위치, 방향 설정)
		 */
		virtual void onInit(RHIScene& scene, RenderGraph& renderGraph, RHICamera& camera) = 0;

		/**
		 * @brief 매 프레임 업데이트
		 */
		virtual void onUpdate(float deltaTime, uint32_t frameIndex) {}

		/**
		 * @brief GUI 렌더링
		 */
		virtual void onGui() {}

		/**
		 * @brief 종료
		 */
		virtual void onShutdown() {}
	};

	/**
	 * @brief 플랫폼 독립적 RHI Application
	 * 
	 * RenderGraph 기반 렌더링 시스템 사용
	 * 사용자는 IRHIApplicationListener를 통해 커스터마이징
	 */
	class RHIApplication
	{
	public:
		RHIApplication(const EngineConfig& config = EngineConfig::createDefault(), 
			RHIApiType apiType = RHIApiType::Vulkan);
		~RHIApplication();

		// ========================================
		// 리스너 설정
		// ========================================
		void setListener(IRHIApplicationListener* listener) { listener_ = listener; }

		// ========================================
		// 메인 루프
		// ========================================
		void run();

		// ========================================
		// 접근자
		// ========================================
		RHI* getRHI() const { return rhi_.get(); }
		RenderGraph* getRenderGraph() const { return renderGraph_.get(); }
		RHIRenderer* getRenderer() const { return renderer_.get(); }
		RHIScene* getScene() const { return scene_.get(); }
		RHICamera* getCamera() { return &camera_; }
		InputManager* getInputManager() { return &inputManager_; }
		const EngineConfig& getConfig() const { return config_; }
		IWindow* getWindow() const { return window_.get(); }

	private:
		// ========================================
		// 초기화 및 종료
		// ========================================
		void initialize();
		void shutdown();

		// ========================================
		// 메인 루프
		// ========================================
		void mainLoop();
		void updateFrame(float deltaTime);
		void renderFrame(uint32_t frameIndex);

		// ========================================
		// RenderGraph 설정
		// ========================================
		void setupDefaultRenderGraph();

		// ========================================
		// 멤버 변수
		// ========================================
		EngineConfig config_;
		RHIApiType apiType_;

		// Window (플랫폼 독립적)
		std::unique_ptr<IWindow> window_;

		// RHI 시스템
		std::unique_ptr<RHI> rhi_;
		std::unique_ptr<RenderGraph> renderGraph_;
		std::unique_ptr<RHIRenderer> renderer_;
		std::unique_ptr<RHIScene> scene_;

		// 리스너
		IRHIApplicationListener* listener_ = nullptr;

		// 카메라
		RHICamera camera_;

		// 입력 시스템
		InputManager inputManager_;

		// 프레임 정보
		float deltaTime_ = 0.0f;
		double lastFrameTime_ = 0.0;
		uint32_t frameIndex_ = 0;
		bool initialized_ = false;
		bool running_ = false;
	};

} // namespace BinRenderer
