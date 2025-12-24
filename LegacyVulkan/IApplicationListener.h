#pragma once

#include <cstdint>

namespace BinRenderer::Vulkan {

	class Scene;
	class Renderer;

	/**
	 * @brief 애플리케이션 라이프사이클 콜백 인터페이스
	 *
	 * 사용자 코드가 렌더링 엔진의 이벤트를 후킹할 수 있도록 합니다.
	 * Open-Closed Principle: 엔진 코드 수정 없이 확장 가능
	 */
	class IApplicationListener
	{
	public:
		virtual ~IApplicationListener() = default;

		/**
		 * @brief 엔진 초기화 완료 후 호출
		 * @param scene 씬 객체 참조 (모델 추가 등 가능)
		 * @param renderer 렌더러 객체 참조 (렌더 그래프 수정 등 가능)
		 */
		virtual void onInit(Scene& scene, Renderer& renderer) {}

		/**
		 * @brief 매 프레임 업데이트 (렌더링 전)
	  * @param deltaTime 이전 프레임과의 시간차 (초)
		 * @param frameIndex 현재 프레임 인덱스
		 */
		virtual void onUpdate(float deltaTime, uint32_t frameIndex) {}

		/**
	 * @brief 렌더링 직전 호출 (커맨드 버퍼 기록 전)
		 * @param frameIndex 현재 프레임 인덱스
		 */
		virtual void onPreRender(uint32_t frameIndex) {}

		/**
		 * @brief 렌더링 직후 호출 (커맨드 버퍼 제출 후)
		 * @param frameIndex 현재 프레임 인덱스
		 */
		virtual void onPostRender(uint32_t frameIndex) {}

		/**
		 * @brief GUI 렌더링 시 호출 (ImGui 사용 가능)
		 */
		virtual void onGui() {}

		/**
		 * @brief 윈도우 리사이즈 시 호출
		 * @param width 새 너비
		 * @param height 새 높이
	  */
		virtual void onResize(uint32_t width, uint32_t height) {}

		/**
		 * @brief 종료 직전 호출 (리소스 정리 등)
		 */
		virtual void onShutdown() {}

		/**
		 * @brief 키보드 입력 이벤트
		 * @param key GLFW 키 코드
	   * @param scancode 시스템 스캔 코드
		 * @param action GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT
		 * @param mods Modifier keys (Shift, Ctrl, Alt 등)
		 * @return true면 이벤트 소비 (기본 처리 스킵), false면 기본 처리 수행
		 */
		virtual bool onKeyInput(int key, int scancode, int action, int mods)
		{
			return false; // Don't consume event by default
		}

		/**
		* @brief 마우스 버튼 이벤트
			* @param button GLFW 마우스 버튼 코드
			* @param action GLFW_PRESS, GLFW_RELEASE
			* @param mods Modifier keys
			* @return true면 이벤트 소비
			*/
		virtual bool onMouseButton(int button, int action, int mods)
		{
			return false;
		}

		/**
	  * @brief 마우스 이동 이벤트
		 * @param xpos 마우스 X 좌표
		 * @param ypos 마우스 Y 좌표
		 * @return true면 이벤트 소비
		 */
		virtual bool onMouseMove(double xpos, double ypos)
		{
			return false;
		}
	};

} // namespace BinRenderer::Vulkan
