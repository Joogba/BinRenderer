#pragma once

#include <glm/glm.hpp>
#include <functional>
#include <unordered_map>
#include <vector>

namespace BinRenderer
{
	/**
	 * @brief 키 상태
	 */
	enum class KeyState
	{
		Released = 0,
		Pressed = 1,
		Repeat = 2
	};

	/**
	 * @brief 마우스 버튼
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
	 * @brief 플랫폼 독립적 입력 관리 클래스
	 * 
	 * 플랫폼별 윈도우 시스템(GLFW, SDL 등)으로부터 입력을 받아
	 * 리스너들에게 전달합니다.
	 */
	class InputManager
	{
	public:
		InputManager() = default;
		~InputManager() = default;

		/**
		 * @brief 입력 리스너 등록
		 */
		void addListener(IInputListener* listener);

		/**
		 * @brief 입력 리스너 제거
		 */
		void removeListener(IInputListener* listener);

		/**
		 * @brief 프레임 업데이트
		 */
		void update();

		// ========================================
		// 이벤트 발생 (플랫폼별 구현에서 호출)
		// ========================================

		void notifyKeyPressed(int key, int mods);
		void notifyKeyReleased(int key, int mods);
		void notifyKeyRepeat(int key, int mods);

		void notifyMouseButtonPressed(MouseButton button, double x, double y);
		void notifyMouseButtonReleased(MouseButton button, double x, double y);

		void notifyMouseMoved(double x, double y, double deltaX, double deltaY);
		void notifyMouseScrolled(double xOffset, double yOffset);

		// ========================================
		// 상태 쿼리 API
		// ========================================

		bool isKeyPressed(int key) const;
		bool isMouseButtonPressed(MouseButton button) const;

		glm::vec2 getMousePosition() const { return mousePosition_; }
		glm::vec2 getMouseDelta() const { return mouseDelta_; }

	private:
		std::vector<IInputListener*> listeners_;

		// 입력 상태
		std::unordered_map<int, bool> keyStates_;
		std::unordered_map<int, bool> mouseButtonStates_;

		glm::vec2 mousePosition_ = glm::vec2(0.0f);
		glm::vec2 mouseDelta_ = glm::vec2(0.0f);
	};

} // namespace BinRenderer
