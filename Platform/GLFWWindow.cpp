#include "GLFWWindow.h"
#include "../Core/Logger.h"
#include <GLFW/glfw3.h>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#elif __linux__
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>
#elif __APPLE__
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>
#endif

namespace BinRenderer
{
	GLFWWindow::GLFWWindow()
	{
	}

	GLFWWindow::~GLFWWindow()
	{
		destroy();
	}

	bool GLFWWindow::create(uint32_t width, uint32_t height, const std::string& title)
	{
		width_ = width;
		height_ = height;
		title_ = title;

		// GLFW 초기화 (최초 1회만)
		if (!initialized_)
		{
			if (!glfwInit())
			{
				printLog("❌ ERROR: Failed to initialize GLFW");
				return false;
			}
			initialized_ = true;
		}

		// Vulkan 사용 설정 (OpenGL 비활성화)
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		// Window 생성
		window_ = glfwCreateWindow(
			static_cast<int>(width),
			static_cast<int>(height),
			title.c_str(),
			nullptr,
			nullptr
		);

		if (!window_)
		{
			printLog("❌ ERROR: Failed to create GLFW window");
			glfwTerminate();
			initialized_ = false;
			return false;
		}

		printLog("✅ GLFW Window created: {}x{} \"{}\"", width, height, title);
		return true;
	}

	void GLFWWindow::destroy()
	{
		if (window_)
		{
			glfwDestroyWindow(window_);
			window_ = nullptr;
			printLog("✅ GLFW Window destroyed");
		}

		if (initialized_)
		{
			glfwTerminate();
			initialized_ = false;
			printLog("✅ GLFW terminated");
		}
	}

	bool GLFWWindow::shouldClose() const
	{
		if (!window_)
			return true;

		return glfwWindowShouldClose(window_);
	}

	void GLFWWindow::setShouldClose(bool shouldClose)
	{
		if (window_)
		{
			glfwSetWindowShouldClose(window_, shouldClose ? GLFW_TRUE : GLFW_FALSE);
		}
	}

	void GLFWWindow::pollEvents()
	{
		glfwPollEvents();
	}

	void GLFWWindow::getSize(uint32_t& width, uint32_t& height) const
	{
		if (window_)
		{
			int w, h;
			glfwGetWindowSize(window_, &w, &h);
			width = static_cast<uint32_t>(w);
			height = static_cast<uint32_t>(h);
		}
		else
		{
			width = width_;
			height = height_;
		}
	}

	void GLFWWindow::setSize(uint32_t width, uint32_t height)
	{
		width_ = width;
		height_ = height;

		if (window_)
		{
			glfwSetWindowSize(window_, static_cast<int>(width), static_cast<int>(height));
			printLog("🖼️  Window resized: {}x{}", width, height);
		}
	}

	void GLFWWindow::setTitle(const std::string& title)
	{
		title_ = title;

		if (window_)
		{
			glfwSetWindowTitle(window_, title.c_str());
		}
	}

	void GLFWWindow::setFullscreen(bool fullscreen)
	{
		if (!window_)
			return;

		if (fullscreen)
		{
			// 전체화면
			GLFWmonitor* monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			glfwSetWindowMonitor(window_, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
			printLog("🖥️  Fullscreen enabled");
		}
		else
		{
			// 창 모드
			glfwSetWindowMonitor(window_, nullptr, 100, 100, static_cast<int>(width_), static_cast<int>(height_), 0);
			printLog("🪟 Windowed mode");
		}
	}

	void GLFWWindow::setVisible(bool visible)
	{
		if (!window_)
			return;

		if (visible)
		{
			glfwShowWindow(window_);
		}
		else
		{
			glfwHideWindow(window_);
		}
	}

	void* GLFWWindow::getNativeHandle() const
	{
		if (!window_)
			return nullptr;

#ifdef _WIN32
		// Windows: HWND
		return glfwGetWin32Window(window_);
#elif __linux__
		// Linux: X11 Window
		return reinterpret_cast<void*>(glfwGetX11Window(window_));
#elif __APPLE__
		// macOS: NSWindow*
		return glfwGetCocoaWindow(window_);
#else
		return nullptr;
#endif
	}

	const char** GLFWWindow::getRequiredExtensions(uint32_t& extensionCount) const
	{
		return glfwGetRequiredInstanceExtensions(&extensionCount);
	}

	int GLFWWindow::createVulkanSurface(void* instance, void** surface) const
	{
		if (!window_)
		{
			printLog("❌ ERROR: Window is null in createVulkanSurface");
			return -1;
		}

		// ✅ GLFW의 Surface 생성 함수 사용 (GLFWwindow* 필요)
		VkResult result = static_cast<VkResult>(
			glfwCreateWindowSurface(
				reinterpret_cast<VkInstance>(instance),
				window_,
				nullptr,
				reinterpret_cast<VkSurfaceKHR*>(surface)
			)
		);
		
		if (result == 0) // VK_SUCCESS
		{
			printLog("✅ Vulkan Surface created via GLFW");
		}
		else
		{
			printLog("❌ ERROR: Failed to create Vulkan Surface: {}", static_cast<int>(result));
		}

		return static_cast<int>(result);
	}

} // namespace BinRenderer
