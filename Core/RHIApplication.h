#pragma once

#include "../RHI/Core/RHI.h"
#include "../RHI/Util/RHIFactory.h"
#include "../RenderPass/RenderGraph/RGGraph.h"
#include "../Scene/Animation.h"
#include "../Scene/RHICamera.h"
#include "../RenderPass/RHIForwardPassRG.h"
#include "EngineConfig.h"
#include "InputManager.h"
#include <memory>
#include <string>
#include <functional>

namespace BinRenderer
{
	class RHIScene;

	/**
	 * @brief RHI 기반 Application 리스너
	 */
	class IRHIApplicationListener
	{
	public:
		virtual ~IRHIApplicationListener() = default;

		virtual void onInit(RHIScene& scene, RenderGraph& renderGraph, RHICamera& camera) = 0;
		virtual void onUpdate(float deltaTime, uint32_t frameIndex) {}
		virtual void onGui() {}
		virtual void onShutdown() {}
	};

	/**
	 * @brief 플랫폼 독립적 RHI Application
	 */
	class RHIApplication
	{
	public:
		RHIApplication(const EngineConfig& config = EngineConfig::createDefault(), 
		 RHIApiType apiType = RHIApiType::Vulkan);
		~RHIApplication();

		void setListener(IRHIApplicationListener* listener);
		void run();

		RHI* getRHI() const { return rhi_.get(); }
		RenderGraph* getRenderGraph() const { return renderGraph_.get(); }
		RHIScene* getScene() const { return scene_.get(); }
		RHICamera* getCamera() { return &camera_; }
		InputManager* getInputManager() { return &inputManager_; }

		const EngineConfig& getConfig() const { return config_; }

	private:
		void initialize();
		void shutdown();
		void mainLoop();
		void setupDefaultRenderGraph();

		EngineConfig config_;
		RHIApiType apiType_;
		std::unique_ptr<RHI> rhi_;
		std::unique_ptr<RenderGraph> renderGraph_;
		std::unique_ptr<RHIScene> scene_;
		IRHIApplicationListener* listener_ = nullptr;

		// Camera
		RHICamera camera_;

		// Input system
		InputManager inputManager_;

		// Render passes
		RHIForwardPassRG* forwardPass_ = nullptr;

		float deltaTime_ = 0.0f;
		uint32_t frameIndex_ = 0;
		bool initialized_ = false;
	};

} // namespace BinRenderer
