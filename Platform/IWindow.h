#pragma once

#include <cstdint>
#include <string>

namespace BinRenderer
{
	// Forward declarations (Vulkan 타입은 사용하는 쪽에서 include)
	class IWindow;

	/**
	 * @brief 플랫폼 독립적 Window 인터페이스
	 * 
	 * 다양한 플랫폼(Windows, Linux, macOS)에서 Window를 추상화
	 */
	class IWindow
	{
	public:
		virtual ~IWindow() = default;

		// ========================================
		// 초기화 및 종료
		// ========================================

		/**
		 * @brief Window 생성
		 * @param width Window 너비
		 * @param height Window 높이
		 * @param title Window 제목
		 * @return 성공 여부
		 */
		virtual bool create(uint32_t width, uint32_t height, const std::string& title) = 0;

		/**
		 * @brief Window 종료
		 */
		virtual void destroy() = 0;

		// ========================================
		// Window 상태
		// ========================================

		/**
		 * @brief Window가 닫혀야 하는지 확인
		 */
		virtual bool shouldClose() const = 0;

		/**
		 * @brief Window 닫기 요청
		 */
		virtual void setShouldClose(bool shouldClose) = 0;

		// ========================================
		// 이벤트 처리
		// ========================================

		/**
		 * @brief 이벤트 폴링 (매 프레임 호출)
		 */
		virtual void pollEvents() = 0;

		// ========================================
		// Window 속성
		// ========================================

		/**
		 * @brief Window 크기 가져오기
		 */
		virtual void getSize(uint32_t& width, uint32_t& height) const = 0;

		/**
		 * @brief Window 크기 설정
		 */
		virtual void setSize(uint32_t width, uint32_t height) = 0;

		/**
		 * @brief Window 제목 설정
		 */
		virtual void setTitle(const std::string& title) = 0;

		/**
		 * @brief 전체화면 전환
		 */
		virtual void setFullscreen(bool fullscreen) = 0;

		/**
		 * @brief Window 표시/숨기기
		 */
		virtual void setVisible(bool visible) = 0;

		// ========================================
		// 플랫폼별 핸들 (Vulkan, DirectX 등에서 필요)
		// ========================================

		/**
		 * @brief 네이티브 Window 핸들 가져오기
		 * - Windows: HWND
		 * - Linux: xcb_window_t or Window (X11)
		 * - macOS: NSWindow*
		 */
		virtual void* getNativeHandle() const = 0;

		/**
		 * @brief 플랫폼별 확장 정보 가져오기
		 * @param extensionCount 확장 개수 출력
		 * @return 확장 이름 배열 (Vulkan Instance Extensions)
		 */
		virtual const char** getRequiredExtensions(uint32_t& extensionCount) const = 0;

		// ========================================
		// Vulkan Surface 생성 (플랫폼별로 다름)
		// ========================================

		/**
		 * @brief Vulkan Surface 생성
		 * @param instance Vulkan Instance (void* 타입으로 전달)
		 * @param surface 생성된 Surface 포인터 (void** 타입으로 전달)
		 * @return 0 = 성공, 그 외 = 에러 코드
		 * 
		 * @note Vulkan 타입을 헤더에 노출하지 않기 위해 void*를 사용
		 *    구현체에서 적절히 캐스팅하여 사용
		 */
		virtual int createVulkanSurface(void* instance, void** surface) const = 0;
	};

} // namespace BinRenderer
