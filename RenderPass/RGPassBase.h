#pragma once

#include "RenderGraph/RGTypes.h"
#include "../RHI/Core/RHI.h"
#include <string>
#include <vector>

namespace BinRenderer
{
	// ✅ 전방 선언
	class RenderGraphBuilder;

	/**
	 * @brief RenderGraph 통합 렌더 패스 기본 클래스
	 * 
	 * 기존 RenderPassBase + RenderGraphPassBase를 통합하여
	 * RenderGraph 시스템과 완벽하게 호환되는 기본 클래스
	 * 
	 * @features
	 * - RenderGraph 자동 의존성 관리
	 * - Setup/Execute 분리
	 * - 리소스 자동 추적
	 * - 기존 RenderPass API 호환
	 */
	class RGPassBase
	{
	public:
		RGPassBase(RHI* rhi, const std::string& name);
		virtual ~RGPassBase();

		// ========================================
		// RenderGraph 인터페이스 (필수 구현)
		// ========================================

		/**
		 * @brief Setup 단계: RenderGraph에 리소스 선언
		 * 
		 * 이 함수에서:
		 * - createTexture() / createBuffer()로 리소스 생성
		 * - readTexture() / writeTexture()로 의존성 선언
		 * 
		 * @param builder RenderGraph 빌더
		 */
		virtual void setup(RenderGraphBuilder& builder) = 0;

		/**
		 * @brief Execute 단계: 실제 렌더링 수행
		 * 
		 * 이 함수에서:
		 * - 실제 GPU 커맨드 기록
		 * - 파이프라인 바인딩
		 * - Draw Call 실행
		 * 
		 * @param rhi RHI 인스턴스
		 * @param frameIndex 현재 프레임 인덱스
		 */
		virtual void execute(RHI* rhi, uint32_t frameIndex) = 0;

		// ========================================
		// 기존 RenderPass API 호환 (옵션)
		// ========================================

		/**
		 * @brief 초기화 (파이프라인, 셰이더 등)
		 * 
		 * RenderGraph 시스템에서는 필수가 아니지만,
		 * 기존 코드 호환성을 위해 제공
		 */
		virtual bool initialize() { return true; }

		/**
		 * @brief 종료 (리소스 해제)
		 */
		virtual void shutdown() {}

		/**
		 * @brief 화면 크기 변경
		 * 
		 * @param width 새로운 너비
		 * @param height 새로운 높이
		 */
		virtual void resize(uint32_t width, uint32_t height)
		{
			width_ = width;
			height_ = height;
		}

		// ========================================
		// 정보 접근
		// ========================================

		const std::string& getName() const { return name_; }
		uint32_t getWidth() const { return width_; }
		uint32_t getHeight() const { return height_; }
		uint32_t getExecutionOrder() const { return executionOrder_; }
		void setExecutionOrder(uint32_t order) { executionOrder_ = order; }

		// 의존성 정보
		const std::vector<RGResourceDependency>& getDependencies() const { return dependencies_; }
		void addDependency(const RGResourceDependency& dep) { dependencies_.push_back(dep); }

	protected:
		RHI* rhi_;
		std::string name_;
		uint32_t width_ = 0;
		uint32_t height_ = 0;
		uint32_t executionOrder_ = 0;

		// RenderGraph 의존성 (자동 관리)
		std::vector<RGResourceDependency> dependencies_;
	};

} // namespace BinRenderer

// ✅ 템플릿 클래스는 별도 파일로 분리하거나 여기에 RenderGraphBuilder include
#include "RenderGraph/RGBuilder.h"

namespace BinRenderer
{
	/**
	 * @brief 타입 안전 RenderGraph Pass (템플릿 버전)
	 * 
	 * PassData 구조체를 사용하여 타입 안전하게 리소스 관리
	 * 
	 * @example
	 * ```cpp
	 * struct GBufferPassData {
	 *     RGTextureHandle albedo;
	 *     RGTextureHandle normal;
	 *     RGTextureHandle depth;
	 * };
	 * 
	 * class GBufferPass : public RGPass<GBufferPassData> {
	 *     void setup(GBufferPassData& data, RenderGraphBuilder& builder) override {
	 *      data.albedo = builder.createTexture(...);
	 *         data.normal = builder.createTexture(...);
	 *         data.depth = builder.createTexture(...);
	 *     }
	 *     
	 *     void execute(const GBufferPassData& data, RHI* rhi, uint32_t frameIndex) override {
	 *         // 실제 렌더링
	 *     }
	 * };
	 * ```
	 */
	template<typename PassData>
	class RGPass : public RGPassBase
	{
	public:
		RGPass(RHI* rhi, const std::string& name)
			: RGPassBase(rhi, name)
		{}

		// ========================================
		// 타입 안전 인터페이스
		// ========================================

		/**
		 * @brief Setup with PassData
		 */
		virtual void setup(PassData& data, RenderGraphBuilder& builder) = 0;

		/**
		 * @brief Execute with PassData
		 */
		virtual void execute(const PassData& data, RHI* rhi, uint32_t frameIndex) = 0;

		// ========================================
		// RGPassBase 구현 (자동)
		// ========================================

		void setup(RenderGraphBuilder& builder) final override
		{
			setup(data_, builder);
		}

		void execute(RHI* rhi, uint32_t frameIndex) final override
		{
			execute(data_, rhi, frameIndex);
		}

		// PassData 접근
		PassData& getData() { return data_; }
		const PassData& getData() const { return data_; }

	protected:
		PassData data_;
	};

} // namespace BinRenderer
