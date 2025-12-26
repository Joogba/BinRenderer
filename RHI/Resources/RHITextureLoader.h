#pragma once

#include "../Core/RHIType.h"
#include <string>
#include <vector>

namespace BinRenderer
{
	/**
	 * @brief 텍스처 로딩 유틸리티 (플랫폼 독립적)
	 * 
	 * KTX2 큐브맵 및 일반 이미지 파일을 로드하여
	 * RHI 레이어에서 사용 가능한 형태로 변환
	 */
	class RHITextureLoader
	{
	public:
		/**
		 * @brief 로드된 텍스처 데이터 구조체
		 */
		struct LoadedTextureData
		{
			std::vector<uint8_t> data;          // 픽셀 데이터 (모든 레이어/mipmap 포함)
			uint32_t width = 0;
			uint32_t height = 0;
			uint32_t depth = 1;
			uint32_t mipLevels = 1;
			uint32_t arrayLayers = 1;           // 큐브맵: 6
			RHIFormat format = RHI_FORMAT_UNDEFINED;
			bool isCubemap = false;
			
			/**
			 * @brief Mipmap별 오프셋 및 크기 정보
			 */
			struct MipInfo
			{
				size_t offset;      // data 버퍼 내 오프셋
				uint32_t width;     // 해당 mip level의 너비
				uint32_t height;    // 해당 mip level의 높이
			};
			
			// mipInfos[arrayLayer][mipLevel]
			std::vector<std::vector<MipInfo>> mipInfos;
		};

		/**
		 * @brief KTX2 파일 로드 (큐브맵 지원)
		 * 
		 * @param filename KTX2 파일 경로 (.ktx2)
		 * @return 로드된 텍스처 데이터 (실패시 data.empty() == true)
		 */
		static LoadedTextureData loadKTX2(const std::string& filename);

		/**
		 * @brief PNG/JPEG 파일 로드 (2D 텍스처)
		 * 
		 * @param filename 이미지 파일 경로 (.png, .jpg, .jpeg)
		 * @param sRGB sRGB 색상 공간 사용 여부
		 * @return 로드된 텍스처 데이터 (실패시 data.empty() == true)
		 */
		static LoadedTextureData loadImage(const std::string& filename, bool sRGB = false);

	private:
		// 경로 수정 (Windows/Linux 호환)
		static std::string fixPath(const std::string& path);
		
		// Vulkan 포맷 → RHI 포맷 변환
		static RHIFormat convertVkFormatToRHI(uint32_t vkFormat);
	};

} // namespace BinRenderer
