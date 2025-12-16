#pragma once

#include "../Core/RHI.h"
#include "../Core/RHIDefinitions.h"
#include <memory>

namespace BinRenderer
{
	/**
	 * @brief RHI 생성 팩토리
 */
	class RHIFactory
	{
	public:
		/**
		 * @brief RHI 인스턴스 생성
  */
		static RHI* create(RHIApiType apiType);

		/**
		 * @brief RHI 인스턴스 생성 (스마트 포인터)
		 */
		static std::unique_ptr<RHI> createUnique(RHIApiType apiType);

		/**
		 * @brief 사용 가능한 API 확인
	   */
		static bool isApiSupported(RHIApiType apiType);

		/**
		 * @brief 현재 플랫폼에서 권장하는 API 반환
	*/
		static RHIApiType getRecommendedApi();

	private:
		RHIFactory() = delete;
	};

} // namespace BinRenderer
