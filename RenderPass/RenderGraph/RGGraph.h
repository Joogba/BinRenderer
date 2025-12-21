#pragma once

#include "RGTypes.h"
#include "RGBuilder.h"
#include "../RGPassBase.h"  // ✅ 새로운 통합 기본 클래스
#include "../../RHI/Core/RHI.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>

namespace BinRenderer
{
	// ========================================
	// 람다 기반 RenderGraph Pass (하위 호환)
	// ========================================

	/**
	 * @brief 람다 기반 RenderGraph Pass
	 * 
	 * 기존 코드 호환성을 위해 유지
	 * 새로운 코드는 RGPass<PassData>를 사용하는 것을 권장
	 */
	template<typename PassData>
	class RenderGraphPass : public RGPassBase
	{
	public:
		using SetupFunc = std::function<void(PassData&, RenderGraphBuilder&)>;
		using ExecuteFunc = std::function<void(const PassData&, RHI*, uint32_t)>;

		RenderGraphPass(const std::string& name, SetupFunc setup, ExecuteFunc execute)
			: RGPassBase(nullptr, name)
			, setupFunc_(std::move(setup))
			, executeFunc_(std::move(execute))
		{}

		void setup(RenderGraphBuilder& builder) override
		{
			if (setupFunc_) {
				setupFunc_(data_, builder);
			}
		}

		void execute(RHI* rhi, uint32_t frameIndex) override
		{
			if (executeFunc_) {
				executeFunc_(data_, rhi, frameIndex);
			}
		}

		PassData& getData() { return data_; }
		const PassData& getData() const { return data_; }

	private:
		PassData data_;
		SetupFunc setupFunc_;
		ExecuteFunc executeFunc_;
	};

	/**
	 * @brief RenderGraph 메인 시스템
	 * 
	 * 렌더 패스와 리소스를 관리하고 자동으로 의존성을 해결하여 실행
	 */
	class RenderGraph
	{
	public:
		RenderGraph(RHI* rhi);
		~RenderGraph();

		// ========================================
		// 패스 추가 (람다 기반 - 하위 호환)
		// ========================================

		/**
		 * @brief 렌더 패스 추가 (람다 기반)
		 * 
		 * @tparam PassData 패스 데이터 구조체
		 * @param name 패스 이름
		 * @param setup Setup 함수 (리소스 선언)
		 * @param execute Execute 함수 (렌더링 실행)
		 */
		template<typename PassData>
		void addPass(const std::string& name,
		  typename RenderGraphPass<PassData>::SetupFunc setup,
		             typename RenderGraphPass<PassData>::ExecuteFunc execute)
		{
			auto pass = std::make_unique<RenderGraphPass<PassData>>(
				name, std::move(setup), std::move(execute));

			// Builder에 현재 패스 설정
			builder_.currentPass_ = pass.get();
			builder_.currentPassIndex_ = static_cast<uint32_t>(passes_.size());

			// Setup 단계 실행 (리소스 선언 및 의존성 등록)
			pass->setup(builder_);

			passes_.push_back(std::move(pass));
		}

		// ========================================
		// 패스 추가 (클래스 기반 - 권장)
		// ========================================

		/**
		 * @brief 렌더 패스 추가 (클래스 기반)
		 * 
		 * RGPass<PassData>를 상속받은 클래스를 추가
		 * 
		 * @param pass RGPassBase 포인터
		 * 
		 * @example
		 * auto gbufferPass = std::make_unique<GBufferPassRG>(rhi);
		 * renderGraph.addPass(std::move(gbufferPass));
		 */
		void addPass(std::unique_ptr<RGPassBase> pass);

		// ========================================
		// 그래프 컴파일 및 실행
		// ========================================

		/**
		 * @brief 그래프 컴파일 (의존성 분석 및 최적화)
		 */
		void compile();

		/**
		 * @brief 렌더 그래프 실행
		 */
		void execute(uint32_t frameIndex);

		/**
		 * @brief 그래프 리셋 (다음 프레임 준비)
		 */
		void reset();

		// ========================================
		// 리소스 접근
		// ========================================

		/**
		 * @brief 최종 출력 이미지 가져오기
		 */
		RHIImage* getFinalOutput() const;

		/**
		 * @brief 텍스처 리소스 가져오기
		 */
		RHIImage* getTexture(RGTextureHandle handle) const;

		/**
		 * @brief 버퍼 리소스 가져오기
		 */
		RHIBuffer* getBuffer(RGBufferHandle handle) const;

		// ========================================
		// 디버그 정보
		// ========================================

		/**
		 * @brief 실행 순서 출력 (디버깅용)
		 */
		void printExecutionOrder() const;

		/**
		 * @brief 리소스 사용량 출력 (디버깅용)
		 */
		void printResourceUsage() const;

		/**
		 * @brief 등록된 패스 개수 반환
		 */
		size_t getPassCount() const { return passes_.size(); }

	private:
		RHI* rhi_;
		RenderGraphBuilder builder_;

		// 패스 관리
		std::vector<std::unique_ptr<RGPassBase>> passes_;  // ✅ RenderGraphPassBase → RGPassBase
		std::vector<RGPassBase*> sortedPasses_; // 실행 순서

		// 실제 할당된 리소스
		std::unordered_map<uint32_t, RHIImage*> allocatedTextures_;
		std::unordered_map<uint32_t, RHIBuffer*> allocatedBuffers_;

		bool compiled_ = false;

		// ========================================
		// 내부 헬퍼 함수
		// ========================================

		/**
		 * @brief Topological Sort로 실행 순서 결정
		 */
		void topologicalSort();

		/**
		 * @brief 리소스 할당
		 */
		void allocateResources();

		/**
		 * @brief 리소스 해제
		 */
		void deallocateResources();

		/**
		 * @brief 사용되지 않는 패스 제거 (Culling)
		 */
		void cullUnusedPasses();
	};

} // namespace BinRenderer
