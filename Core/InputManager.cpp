#include "InputManager.h"

namespace BinRenderer
{
	void InputManager::addListener(IInputListener* listener)
	{
		if (listener)
		{
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
		// 매 프레임 델타 리셋
		mouseDelta_ = glm::vec2(0.0f);
	}

	void InputManager::notifyKeyPressed(int key, int mods)
	{
		keyStates_[key] = true;
		for (auto* listener : listeners_)
		{
			listener->onKeyPressed(key, mods);
		}
	}

	void InputManager::notifyKeyReleased(int key, int mods)
	{
		keyStates_[key] = false;
		for (auto* listener : listeners_)
		{
			listener->onKeyReleased(key, mods);
		}
	}

	void InputManager::notifyKeyRepeat(int key, int mods)
	{
		for (auto* listener : listeners_)
		{
			listener->onKeyRepeat(key, mods);
		}
	}

	void InputManager::notifyMouseButtonPressed(MouseButton button, double x, double y)
	{
		mouseButtonStates_[static_cast<int>(button)] = true;
		for (auto* listener : listeners_)
		{
			listener->onMouseButtonPressed(button, x, y);
		}
	}

	void InputManager::notifyMouseButtonReleased(MouseButton button, double x, double y)
	{
		mouseButtonStates_[static_cast<int>(button)] = false;
		for (auto* listener : listeners_)
		{
			listener->onMouseButtonReleased(button, x, y);
		}
	}

	void InputManager::notifyMouseMoved(double x, double y, double deltaX, double deltaY)
	{
		mousePosition_ = glm::vec2(static_cast<float>(x), static_cast<float>(y));
		mouseDelta_ = glm::vec2(static_cast<float>(deltaX), static_cast<float>(deltaY));

		for (auto* listener : listeners_)
		{
			listener->onMouseMoved(x, y, deltaX, deltaY);
		}
	}

	void InputManager::notifyMouseScrolled(double xOffset, double yOffset)
	{
		for (auto* listener : listeners_)
		{
			listener->onMouseScrolled(xOffset, yOffset);
		}
	}

	bool InputManager::isKeyPressed(int key) const
	{
		auto it = keyStates_.find(key);
		return it != keyStates_.end() && it->second;
	}

	bool InputManager::isMouseButtonPressed(MouseButton button) const
	{
		auto it = mouseButtonStates_.find(static_cast<int>(button));
		return it != mouseButtonStates_.end() && it->second;
	}

} // namespace BinRenderer
