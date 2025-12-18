#pragma once

#include "IWindow.h"

// Forward declaration
struct GLFWwindow;

namespace BinRenderer
{
	/**
	 * @brief GLFW 기반 Window 구현
	 * 
	 * Windows, Linux, macOS에서 모두 동작
	 */
	class GLFWWindow : public IWindow
	{
	public:
		GLFWWindow();
		~GLFWWindow() override;

		// ========================================
		// IWindow 구현
		// ========================================

		bool create(uint32_t width, uint32_t height, const std::string& title) override;
		void destroy() override;

		bool shouldClose() const override;
		void setShouldClose(bool shouldClose) override;

		void pollEvents() override;

		void getSize(uint32_t& width, uint32_t& height) const override;
		void setSize(uint32_t width, uint32_t height) override;
		void setTitle(const std::string& title) override;
		void setFullscreen(bool fullscreen) override;
		void setVisible(bool visible) override;

		void* getNativeHandle() const override;
		const char** getRequiredExtensions(uint32_t& extensionCount) const override;

		// Vulkan Surface 생성
		int createVulkanSurface(void* instance, void** surface) const override;

		// ========================================
		// GLFW 전용
		// ========================================

		/**
		 * @brief GLFWwindow* 직접 가져오기 (레거시 코드 호환)
		 */
		GLFWwindow* getGLFWHandle() const { return window_; }

	private:
		GLFWwindow* window_ = nullptr;
		uint32_t width_ = 0;
		uint32_t height_ = 0;
		std::string title_;
		bool initialized_ = false;
	};

} // namespace BinRenderer
