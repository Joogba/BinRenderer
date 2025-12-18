#pragma once

#include "IWindow.h"
#include <memory>

namespace BinRenderer
{
	/**
	 * @brief Window 생성 방식
	 */
	enum class WindowBackend
	{
		Auto,       // 자동 선택 (기본: GLFW)
		GLFW,     // GLFW (Windows/Linux/macOS)
		Win32,      // Windows 전용 (미구현)
		Cocoa,      // macOS 전용 (미구현)
		X11,        // Linux 전용 (미구현)
		Wayland     // Linux 전용 (미구현)
	};

	/**
	 * @brief Window 팩토리
	 * 
	 * 플랫폼에 맞는 Window 구현을 생성
	 */
	class WindowFactory
	{
	public:
		/**
		 * @brief Window 생성
		 * @param backend Window 백엔드 (기본: Auto)
		 * @return IWindow 인터페이스
		 */
		static std::unique_ptr<IWindow> create(WindowBackend backend = WindowBackend::Auto);

		/**
		 * @brief 현재 플랫폼에서 지원하는 백엔드 확인
		 */
		static bool isSupported(WindowBackend backend);

		/**
		 * @brief 기본 백엔드 가져오기
		 */
		static WindowBackend getDefaultBackend();
	};

} // namespace BinRenderer
