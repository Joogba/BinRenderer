#pragma once

#include <cstdint>

namespace BinRenderer
{
	class RHIScene;
	class RHICamera;
	class RenderGraph;

	/**
	 * @brief 플랫폼 독립적 애플리케이션 라이프사이클 리스너
	 * 
	 * 기존 Vulkan::IApplicationListener를 플랫폼 독립적으로 재설계
	 */
	class IApplicationListener
	{
	public:
		virtual ~IApplicationListener() = default;

		/**
		 * @brief 엔진 초기화 완료 후 호출
		 */
		virtual void onInit(RHIScene& scene, RenderGraph& renderGraph, RHICamera& camera) {}

		/**
		 * @brief 매 프레임 업데이트 (렌더링 전)
		 */
		virtual void onUpdate(float deltaTime, uint32_t frameIndex) {}

		/**
		 * @brief 렌더링 직전 호출
		 */
		virtual void onPreRender(uint32_t frameIndex) {}

		/**
		 * @brief 렌더링 직후 호출
		 */
		virtual void onPostRender(uint32_t frameIndex) {}

		/**
		 * @brief GUI 렌더링 시 호출 (ImGui 사용 가능)
		 */
		virtual void onGui() {}

		/**
		 * @brief 윈도우 리사이즈 시 호출
		 */
		virtual void onResize(uint32_t width, uint32_t height) {}

		/**
		 * @brief 종료 직전 호출 (리소스 정리 등)
		 */
		virtual void onShutdown() {}

		/**
		 * @brief 키보드 입력 이벤트
		 * @return true면 이벤트 소비, false면 기본 처리 수행
		 */
		virtual bool onKeyInput(int key, int scancode, int action, int mods)
		{
			return false;
		}

		/**
		 * @brief 마우스 버튼 이벤트
		 * @return true면 이벤트 소비
		 */
		virtual bool onMouseButton(int button, int action, int mods)
		{
			return false;
		}

		/**
		 * @brief 마우스 이동 이벤트
		 * @return true면 이벤트 소비
		 */
		virtual bool onMouseMove(double xpos, double ypos)
		{
			return false;
		}
	};

} // namespace BinRenderer
