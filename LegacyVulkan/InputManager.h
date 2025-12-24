#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <functional>
#include <unordered_map>

namespace BinRenderer::Vulkan {

	/**
	 * @brief 키 상태를 나타내는 열거형
	 */
	enum class KeyState
	{
		Released = 0,
		Pressed = 1,
		Repeat = 2
	};

	/**
	 * @brief 마우스 버튼 상태를 나타내는 열거형
	 */
	enum class MouseButton
	{
		Left = 0,
		Right = 1,
		Middle = 2
	};

	/**
	 * @brief 입력 이벤트 리스너 인터페이스
	 */
	class IInputListener
	{
	public:
		virtual ~IInputListener() = default;

		// 키보드 이벤트
		virtual void onKeyPressed(int key, int mods) {}
		virtual void onKeyReleased(int key, int mods) {}
		virtual void onKeyRepeat(int key, int mods) {}

		// 마우스 버튼 이벤트
		virtual void onMouseButtonPressed(MouseButton button, double x, double y) {}
		virtual void onMouseButtonReleased(MouseButton button, double x, double y) {}

		// 마우스 이동 이벤트
		virtual void onMouseMoved(double x, double y, double deltaX, double deltaY) {}

		// 마우스 스크롤 이벤트
		virtual void onMouseScrolled(double xOffset, double yOffset) {}
	};

	/**
	 * @brief 입력 관리 클래스
	 * 
	 * GLFW 윈도우의 입력 이벤트를 수집하고 리스너들에게 전달합니다.
	 */
	class InputManager
	{
	public:
		InputManager();
		~InputManager() = default;

		/**
		 * @brief GLFW 윈도우에 입력 콜백 설정
		 */
		void initialize(GLFWwindow* window);

		/**
		 * @brief 입력 리스너 등록
		 */
		void addListener(IInputListener* listener);

		/**
		 * @brief 입력 리스너 제거
		 */
		void removeListener(IInputListener* listener);

		/**
		 * @brief 프레임 업데이트 (폴링 기반 입력 처리)
		 */
		void update();

		// ========================================
		// 상태 쿼리 API
		// ========================================

		/**
		 * @brief 키가 현재 눌려있는지 확인
		 */
		bool isKeyPressed(int key) const;

		/**
		 * @brief 마우스 버튼이 현재 눌려있는지 확인
		 */
		bool isMouseButtonPressed(MouseButton button) const;

		/**
		 * @brief 현재 마우스 위치 가져오기
		 */
		glm::vec2 getMousePosition() const { return mousePosition_; }

		/**
		 * @brief 이전 프레임 대비 마우스 이동량 가져오기
		 */
		glm::vec2 getMouseDelta() const { return mouseDelta_; }

	private:
		// GLFW 콜백 함수들 (static)
		static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
		static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

		// 리스너 관리
		std::vector<IInputListener*> listeners_;

		// 입력 상태
		std::unordered_map<int, bool> keyStates_;
		std::unordered_map<MouseButton, bool> mouseButtonStates_;
		glm::vec2 mousePosition_{ 0.0f, 0.0f };
		glm::vec2 previousMousePosition_{ 0.0f, 0.0f };
		glm::vec2 mouseDelta_{ 0.0f, 0.0f };

		GLFWwindow* window_ = nullptr;
	};

} // namespace BinRenderer::Vulkan
