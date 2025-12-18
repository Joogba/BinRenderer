#include "WindowFactory.h"
#include "GLFWWindow.h"
#include "../Core/Logger.h"

namespace BinRenderer
{
	std::unique_ptr<IWindow> WindowFactory::create(WindowBackend backend)
	{
		// Auto: 플랫폼에 맞게 자동 선택
		if (backend == WindowBackend::Auto)
		{
			backend = getDefaultBackend();
		}

		switch (backend)
		{
		case WindowBackend::GLFW:
			printLog("🪟 Creating GLFW Window");
			return std::make_unique<GLFWWindow>();

		case WindowBackend::Win32:
			printLog("⚠️  Win32 Window not implemented yet, falling back to GLFW");
			return std::make_unique<GLFWWindow>();

		case WindowBackend::Cocoa:
			printLog("⚠️  Cocoa Window not implemented yet, falling back to GLFW");
			return std::make_unique<GLFWWindow>();

		case WindowBackend::X11:
		case WindowBackend::Wayland:
			printLog("⚠️  X11/Wayland Window not implemented yet, falling back to GLFW");
			return std::make_unique<GLFWWindow>();

		default:
			printLog("❌ ERROR: Unknown window backend");
			return nullptr;
		}
	}

	bool WindowFactory::isSupported(WindowBackend backend)
	{
		switch (backend)
		{
		case WindowBackend::Auto:
		case WindowBackend::GLFW:
			return true;  // GLFW는 모든 플랫폼에서 지원

		case WindowBackend::Win32:
#ifdef _WIN32
			return false;  // 미구현
#else
			return false;
#endif

		case WindowBackend::Cocoa:
#ifdef __APPLE__
			return false;  // 미구현
#else
			return false;
#endif

		case WindowBackend::X11:
		case WindowBackend::Wayland:
#ifdef __linux__
			return false;  // 미구현
#else
			return false;
#endif

		default:
			return false;
		}
	}

	WindowBackend WindowFactory::getDefaultBackend()
	{
		// 현재는 모든 플랫폼에서 GLFW 사용
		return WindowBackend::GLFW;

		// 향후 플랫폼별 기본값:
		// #ifdef _WIN32
		//     return WindowBackend::Win32;  // Windows: Win32 API
		// #elif __APPLE__
		//     return WindowBackend::Cocoa;  // macOS: Cocoa
		// #elif __linux__
		//     return WindowBackend::Wayland;  // Linux: Wayland (X11보다 현대적)
		// #else
		//     return WindowBackend::GLFW;  // 기타: GLFW
		// #endif
	}

} // namespace BinRenderer
