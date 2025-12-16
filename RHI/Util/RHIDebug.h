#pragma once

#include "../Core/RHI.h"
#include <string>
#include <functional>

namespace BinRenderer
{
	/**
	 * @brief RHI 디버그 헬퍼
	 */
	class RHIDebug
	{
	public:
		/**
	* @brief 디버그 메시지 콜백 타입
  */
		using DebugMessageCallback = std::function<void(const std::string& message)>;

		/**
	* @brief 디버그 메시지 콜백 설정
		 */
		static void setDebugCallback(DebugMessageCallback callback);

		/**
		   * @brief 디버그 메시지 출력
		   */
		static void logDebug(const std::string& message);
		static void logInfo(const std::string& message);
		static void logWarning(const std::string& message);
		static void logError(const std::string& message);

		/**
			   * @brief API 이름 문자열 반환
		*/
		static const char* getApiName(RHIApiType apiType);

		/**
  * @brief 포맷 이름 문자열 반환
  */
		static const char* getFormatName(RHIFormat format);

		/**
		 * @brief 버퍼 사용 플래그 문자열 반환
	   */
		static std::string getBufferUsageFlagsString(RHIBufferUsageFlags flags);

		/**
		 * @brief 이미지 사용 플래그 문자열 반환
		 */
		static std::string getImageUsageFlagsString(RHIImageUsageFlags flags);

		/**
	   * @brief 셰이더 스테이지 문자열 반환
		*/
		static std::string getShaderStageFlagsString(RHIShaderStageFlags flags);

		/**
		  * @brief RHI 리소스 정보 덤프
			*/
		static void dumpBufferInfo(const RHIBufferCreateInfo& createInfo);
		static void dumpImageInfo(const RHIImageCreateInfo& createInfo);
		static void dumpShaderInfo(const RHIShaderCreateInfo& createInfo);
		static void dumpPipelineInfo(const RHIPipelineCreateInfo& createInfo);

	private:
		static DebugMessageCallback s_debugCallback;

		RHIDebug() = delete;
	};

} // namespace BinRenderer
