#pragma once

#include "../RHI/Core/RHI.h"
#include "../RHI/Core/RHIDefinitions.h"
#include "../RHI/Core/RHISwapchain.h"
#include "../RHI/Pipeline/RHIRenderPass.h"
#include "../RHI/Pipeline/RHIFramebuffer.h"
#include <memory>
#include <string>

namespace BinRenderer
{
	/**
	 * @brief 렌더 패스 기본 클래스
	 */
	class RenderPassBase
	{
	public:
		RenderPassBase(RHI* rhi, const std::string& name);
		virtual ~RenderPassBase();

		// 렌더 패스 생명주기
		virtual bool initialize() = 0;
		virtual void shutdown() = 0;
		virtual void resize(uint32_t width, uint32_t height) = 0;

		// 렌더링 실행
		virtual void execute(uint32_t frameIndex) = 0;

		// 정보 접근
		const std::string& getName() const { return name_; }
		uint32_t getWidth() const { return width_; }
		uint32_t getHeight() const { return height_; }

	protected:
		RHI* rhi_;
		std::string name_;
		uint32_t width_ = 0;
		uint32_t height_ = 0;

		// 공통 리소스
		RHIRenderPass* renderPass_ = nullptr;
		RHIFramebuffer* framebuffer_ = nullptr;

		// 헬퍼 함수
		void beginRenderPass(uint32_t frameIndex, const RHIClearValue* clearValues, uint32_t clearValueCount);
		void endRenderPass();
	};

	/**
	 * @brief 렌더 패스 관리자
	 */
	class RenderPassManager
	{
	public:
		RenderPassManager(RHI* rhi);
		~RenderPassManager();

		// 렌더 패스 추가/제거
		void addRenderPass(std::unique_ptr<RenderPassBase> renderPass);
		RenderPassBase* getRenderPass(const std::string& name);

		// 전체 렌더링 실행
		void executeAll(uint32_t frameIndex);

		// 리사이즈
		void resize(uint32_t width, uint32_t height);

	private:
		RHI* rhi_;
		std::vector<std::unique_ptr<RenderPassBase>> renderPasses_;
	};

} // namespace BinRenderer
