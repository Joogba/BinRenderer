#pragma once

#include <atomic>
#include <cstdint>

namespace BinRenderer
{
	/**
	 * @brief RHI 리소스 최상위 추상 클래스
	 * 참조 카운팅 기능을 제공합니다.
	 */
	class RHIResource
	{
	public:
		RHIResource() : refCount_(1) {}
		virtual ~RHIResource() = default;

		/**
		 * @brief 참조 카운트 증가
		 */
		void addRef()
		{
			refCount_.fetch_add(1, std::memory_order_relaxed);
		}

		/**
		 * @brief 참조 카운트 감소 및 0 도달 시 삭제
		 */
		void release()
		{
			if (refCount_.fetch_sub(1, std::memory_order_release) == 1)
			{
				std::atomic_thread_fence(std::memory_order_acquire);
				delete this;
			}
		}

		/**
		 * @brief 현재 참조 카운트 반환
		 */
		uint32_t getRefCount() const
		{
			return refCount_.load(std::memory_order_relaxed);
		}

	private:
		std::atomic<uint32_t> refCount_;
	};
}
