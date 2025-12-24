#include "InputManager.h"
#include "Logger.h"

namespace BinRenderer::Vulkan {

	InputManager::InputManager()
	{
	}

	void InputManager::initialize(GLFWwindow* window)
	{
		window_ = window;

		// InputManager를 user pointer로 설정
		glfwSetWindowUserPointer(window, this);

		// GLFW 콜백 설정
		glfwSetKeyCallback(window, keyCallback);
		glfwSetMouseButtonCallback(window, mouseButtonCallback);
		glfwSetCursorPosCallback(window, cursorPosCallback);
		glfwSetScrollCallback(window, scrollCallback);

		// 초기 마우스 위치 가져오기
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		mousePosition_ = glm::vec2(static_cast<float>(x), static_cast<float>(y));
		previousMousePosition_ = mousePosition_;

		printLog("InputManager initialized");
	}

	void InputManager::addListener(IInputListener* listener)
	{
		if (listener) {
			listeners_.push_back(listener);
		}
	}

	void InputManager::removeListener(IInputListener* listener)
	{
		listeners_.erase(
			std::remove(listeners_.begin(), listeners_.end(), listener),
			listeners_.end()
		);
	}

	void InputManager::update()
	{
		// 마우스 델타 계산
		mouseDelta_ = mousePosition_ - previousMousePosition_;
		previousMousePosition_ = mousePosition_;
	}

	bool InputManager::isKeyPressed(int key) const
	{
		auto it = keyStates_.find(key);
		return it != keyStates_.end() && it->second;
	}

	bool InputManager::isMouseButtonPressed(MouseButton button) const
	{
		auto it = mouseButtonStates_.find(button);
		return it != mouseButtonStates_.end() && it->second;
	}

	// ========================================
	// GLFW 콜백 구현
	// ========================================

	void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		auto* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
		if (!inputManager) return;

		// 키 상태 업데이트
		if (action == GLFW_PRESS) {
			inputManager->keyStates_[key] = true;
			
			// 리스너들에게 알림
			for (auto* listener : inputManager->listeners_) {
				listener->onKeyPressed(key, mods);
			}
		}
		else if (action == GLFW_RELEASE) {
			inputManager->keyStates_[key] = false;
			
			// 리스너들에게 알림
			for (auto* listener : inputManager->listeners_) {
				listener->onKeyReleased(key, mods);
			}
		}
		else if (action == GLFW_REPEAT) {
			// 리스너들에게 알림
			for (auto* listener : inputManager->listeners_) {
				listener->onKeyRepeat(key, mods);
			}
		}
	}

	void InputManager::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
	{
		auto* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
		if (!inputManager) return;

		MouseButton mouseButton;
		switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			mouseButton = MouseButton::Left;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			mouseButton = MouseButton::Right;
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			mouseButton = MouseButton::Middle;
			break;
		default:
			return; // 지원하지 않는 버튼
		}

		double x, y;
		glfwGetCursorPos(window, &x, &y);

		if (action == GLFW_PRESS) {
			inputManager->mouseButtonStates_[mouseButton] = true;
			
			// 리스너들에게 알림
			for (auto* listener : inputManager->listeners_) {
				listener->onMouseButtonPressed(mouseButton, x, y);
			}
		}
		else if (action == GLFW_RELEASE) {
			inputManager->mouseButtonStates_[mouseButton] = false;
			
			// 리스너들에게 알림
			for (auto* listener : inputManager->listeners_) {
				listener->onMouseButtonReleased(mouseButton, x, y);
			}
		}
	}

	void InputManager::cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
	{
		auto* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
		if (!inputManager) return;

		glm::vec2 newPosition(static_cast<float>(xpos), static_cast<float>(ypos));
		glm::vec2 delta = newPosition - inputManager->mousePosition_;
		
		inputManager->mousePosition_ = newPosition;

		// 리스너들에게 알림
		for (auto* listener : inputManager->listeners_) {
			listener->onMouseMoved(xpos, ypos, delta.x, delta.y);
		}
	}

	void InputManager::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
	{
		auto* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
		if (!inputManager) return;

		// 리스너들에게 알림
		for (auto* listener : inputManager->listeners_) {
			listener->onMouseScrolled(xoffset, yoffset);
		}
	}

} // namespace BinRenderer::Vulkan
