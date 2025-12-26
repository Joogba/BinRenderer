#pragma once

#include "RHIType.h"
#include "RHIHandle.h"
#include "../Resources/RHIImage.h"

namespace BinRenderer
{
	/**
	 * @brief 스왑체인 생성 정보
	 */
	struct RHISwapchainCreateInfo
	{
		void* windowHandle = nullptr;
		uint32_t width = 1280;
		uint32_t height = 720;
		RHIFormat format = RHI_FORMAT_B8G8R8A8_UNORM;
		RHIPresentMode presentMode = RHI_PRESENT_MODE_FIFO_KHR;
		uint32_t imageCount = 2;
		bool enableVSync = true;
	};

	/**
	 * @brief 스왑체인 추상 클래스
	 */
	class RHISwapchain
	{
	public:
		virtual ~RHISwapchain() = default;

		// 스왑체인 재생성
		virtual bool recreate(uint32_t width, uint32_t height) = 0;

		// 다음 이미지 획득
		virtual bool acquireNextImage(uint32_t& imageIndex, class RHISemaphore* semaphore = nullptr, class RHIFence* fence = nullptr) = 0;

		// 이미지 제시
		virtual bool present(uint32_t imageIndex, class RHISemaphore* waitSemaphore = nullptr) = 0;

		// 스왑체인 정보
		virtual uint32_t getImageCount() const = 0;
		virtual RHIFormat getFormat() const = 0;
		virtual uint32_t getWidth() const = 0;
		virtual uint32_t getHeight() const = 0;

		// 스왑체인 이미지 접근
		virtual RHIImageHandle getImage(uint32_t index) const = 0;
		virtual RHIImageViewHandle getImageView(uint32_t index) const = 0;

		// Present 모드
		virtual RHIPresentMode getPresentMode() const = 0;
		virtual void setPresentMode(RHIPresentMode mode) = 0;
	};

} // namespace BinRenderer