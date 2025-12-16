#pragma once

#include "../Core/RHIType.h"
#include "../Structs/RHIStructs.h"
#include <string>
#include <cassert>

namespace BinRenderer
{
	/**
	 * @brief RHI 검증 유틸리티
	 */
	class RHIValidation
	{
	public:
		/**
		 * @brief 버퍼 생성 정보 검증
   */
		static bool validateBufferCreateInfo(const RHIBufferCreateInfo& createInfo, std::string* outError = nullptr);

		/**
	  * @brief 이미지 생성 정보 검증
	   */
		static bool validateImageCreateInfo(const RHIImageCreateInfo& createInfo, std::string* outError = nullptr);

		/**
		* @brief 셰이더 생성 정보 검증
		*/
		static bool validateShaderCreateInfo(const RHIShaderCreateInfo& createInfo, std::string* outError = nullptr);

		/**
		 * @brief 파이프라인 생성 정보 검증
   */
		static bool validatePipelineCreateInfo(const RHIPipelineCreateInfo& createInfo, std::string* outError = nullptr);

		/**
		 * @brief 포맷이 유효한지 확인
		  */
		static bool isValidFormat(RHIFormat format);

		/**
				 * @brief 포맷이 깊이 포맷인지 확인
			*/
		static bool isDepthFormat(RHIFormat format);

		/**
				* @brief 포맷이 스텐실 포맷인지 확인
				*/
		static bool isStencilFormat(RHIFormat format);

		/**
			* @brief 사용 플래그가 유효한지 확인
		  */
		static bool isValidBufferUsage(RHIBufferUsageFlags usage);
		static bool isValidImageUsage(RHIImageUsageFlags usage);

	private:
		RHIValidation() = delete;
	};

} // namespace BinRenderer
