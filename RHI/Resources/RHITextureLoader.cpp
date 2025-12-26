#include "RHITextureLoader.h"
#include "Core/Logger.h"

//  올바른 순서: Vulkan → KTX
#include <vulkan/vulkan.h>  // Vulkan 타입 정의 (VkFormat, VkDevice, etc.)
#include <ktx.h>             // KTX 기본 API
#include <ktxvulkan.h>       // KTX Vulkan 확장 (ktxTexture2_GetVkFormat 등)

// STB 이미지 라이브러리
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <algorithm>
#include <cstring>

namespace BinRenderer
{
	// ========================================
	// Private Helpers
	// ========================================

	std::string RHITextureLoader::fixPath(const std::string& path)
	{
		std::string fixed = path;
		std::replace(fixed.begin(), fixed.end(), '\\', '/');
		return fixed;
	}

	RHIFormat RHITextureLoader::convertVkFormatToRHI(uint32_t vkFormat)
	{
		// Vulkan 포맷 매핑 (ktxvulkan.h의 VkFormat enum 참조)
		switch (vkFormat)
		{
		case 37: // VK_FORMAT_R8G8B8A8_UNORM
			return RHI_FORMAT_R8G8B8A8_UNORM;
		case 43: // VK_FORMAT_R8G8B8A8_SRGB
			return RHI_FORMAT_R8G8B8A8_SRGB;
		case 97: // VK_FORMAT_R16G16B16A16_SFLOAT
			return RHI_FORMAT_R16G16B16A16_SFLOAT;
		case 109: // VK_FORMAT_R32G32B32A32_SFLOAT
			return RHI_FORMAT_R32G32B32A32_SFLOAT;
		case 100: // VK_FORMAT_D32_SFLOAT
			return RHI_FORMAT_D32_SFLOAT;
		case 124: // VK_FORMAT_D24_UNORM_S8_UINT
			return RHI_FORMAT_D24_UNORM_S8_UINT;
		default:
			printLog("[RHITextureLoader] ⚠️  Unknown VkFormat: {}, defaulting to R8G8B8A8_UNORM", vkFormat);
			return RHI_FORMAT_R8G8B8A8_UNORM;
		}
	}

	// ========================================
	// KTX2 Loader
	// ========================================

	RHITextureLoader::LoadedTextureData RHITextureLoader::loadKTX2(const std::string& filename)
	{
		std::string fixedPath = fixPath(filename);
		LoadedTextureData result;

		// 1. 파일 확장자 검증
		size_t extPos = fixedPath.find_last_of('.');
		if (extPos == std::string::npos || fixedPath.substr(extPos) != ".ktx2")
		{
			printLog("[RHITextureLoader] ❌ File must be .ktx2: {}", filename);
			return result;
		}

		// 2. KTX2 텍스처 로드
		ktxTexture2* ktxTexture2 = nullptr;
		ktxResult ktxResult = ktxTexture2_CreateFromNamedFile(
			fixedPath.c_str(),
			KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
			&ktxTexture2
		);

		if (ktxResult != KTX_SUCCESS)
		{
			printLog("[RHITextureLoader] ❌ Failed to load KTX2 file: {} ", filename);
			return result;
		}

		// 3. 기본 정보 추출
		result.width = ktxTexture2->baseWidth;
		result.height = ktxTexture2->baseHeight;
		result.depth = ktxTexture2->baseDepth;
		result.mipLevels = ktxTexture2->numLevels;
		result.isCubemap = (ktxTexture2->numFaces == 6);
		result.arrayLayers = result.isCubemap ? 6 : ktxTexture2->numLayers;

		// 4. 포맷 변환 (Vulkan → RHI)
		uint32_t vkFormat = ktxTexture2_GetVkFormat(ktxTexture2);
		result.format = convertVkFormatToRHI(vkFormat);

		// HDR 큐브맵 기본값 처리
		if (result.format == RHI_FORMAT_UNDEFINED || result.format == RHI_FORMAT_R8G8B8A8_UNORM)
		{
			if (result.isCubemap)
			{
				result.format = RHI_FORMAT_R16G16B16A16_SFLOAT; // HDR 기본값
				printLog("[RHITextureLoader] Using default HDR format for cubemap");
			}
		}

		// 5. 텍스처 데이터 복사
		ktxTexture* baseTexture = ktxTexture(ktxTexture2);
		ktx_uint8_t* ktxData = ktxTexture_GetData(baseTexture);
		ktx_size_t ktxSize = ktxTexture_GetDataSize(baseTexture);

		result.data.resize(ktxSize);
		std::memcpy(result.data.data(), ktxData, ktxSize);

		// 6. Mipmap 오프셋 정보 추출
		result.mipInfos.resize(result.arrayLayers);

		if (result.isCubemap)
		{
			// 큐브맵: 6개 face, 각 face별 mipmap
			for (uint32_t face = 0; face < 6; ++face)
			{
				result.mipInfos[face].resize(result.mipLevels);

				for (uint32_t level = 0; level < result.mipLevels; ++level)
				{
					ktx_size_t offset = 0;
					ktxTexture_GetImageOffset(baseTexture, level, 0, face, &offset);

					result.mipInfos[face][level].offset = offset;
					result.mipInfos[face][level].width = std::max(1u, result.width >> level);
					result.mipInfos[face][level].height = std::max(1u, result.height >> level);
				}
			}
		}
		else
		{
			// 2D 텍스처: 단일 레이어, 여러 mipmap
			result.mipInfos[0].resize(result.mipLevels);

			for (uint32_t level = 0; level < result.mipLevels; ++level)
			{
				ktx_size_t offset = 0;
				ktxTexture_GetImageOffset(baseTexture, level, 0, 0, &offset);

				result.mipInfos[0][level].offset = offset;
				result.mipInfos[0][level].width = std::max(1u, result.width >> level);
				result.mipInfos[0][level].height = std::max(1u, result.height >> level);
			}
		}

		// 7. KTX2 텍스처 정리
		ktxTexture_Destroy(ktxTexture(ktxTexture2));

		printLog("[RHITextureLoader]  Loaded KTX2: {}", filename);
		printLog("    - Size: {}x{}", result.width, result.height);
		printLog("    - Mip levels: {}", result.mipLevels);
		printLog("    - Array layers: {}", result.arrayLayers);
		printLog("    - Cubemap: {}", result.isCubemap ? "YES" : "NO");
		printLog("    - Format: {}", static_cast<int>(result.format));

		return result;
	}

	// ========================================
	// Image Loader (PNG/JPEG)
	// ========================================

	RHITextureLoader::LoadedTextureData RHITextureLoader::loadImage(const std::string& filename, bool sRGB)
	{
		std::string fixedPath = fixPath(filename);
		LoadedTextureData result;

		// 1. 파일 확장자 검증
		size_t extPos = fixedPath.find_last_of('.');
		std::string ext = (extPos != std::string::npos) ? fixedPath.substr(extPos) : "";
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

		if (ext != ".png" && ext != ".jpg" && ext != ".jpeg")
		{
			printLog("[RHITextureLoader] ❌ Unsupported image format: {}", filename);
			return result;
		}

		// 2. STB로 이미지 로드 (항상 RGBA로 변환)
		int width, height, channels;
		unsigned char* pixels = stbi_load(fixedPath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

		if (!pixels)
		{
			printLog("[RHITextureLoader] ❌ Failed to load image: {} ({})", filename, stbi_failure_reason());
			return result;
		}

		// 3. 결과 설정
		result.width = static_cast<uint32_t>(width);
		result.height = static_cast<uint32_t>(height);
		result.depth = 1;
		result.mipLevels = 1;
		result.arrayLayers = 1;
		result.format = sRGB ? RHI_FORMAT_R8G8B8A8_SRGB : RHI_FORMAT_R8G8B8A8_UNORM;
		result.isCubemap = false;

		// 4. 픽셀 데이터 복사
		size_t dataSize = width * height * 4; // RGBA
		result.data.resize(dataSize);
		std::memcpy(result.data.data(), pixels, dataSize);

		// 5. Mipmap 정보 (단일 레벨)
		result.mipInfos.resize(1);
		result.mipInfos[0].resize(1);
		result.mipInfos[0][0].offset = 0;
		result.mipInfos[0][0].width = result.width;
		result.mipInfos[0][0].height = result.height;

		// 6. STB 메모리 해제
		stbi_image_free(pixels);

		printLog("[RHITextureLoader]  Loaded image: {} ({}x{})", filename, width, height);

		return result;
	}

} // namespace BinRenderer
