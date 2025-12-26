#include "TextureLoader.h"
#include "../Core/Logger.h"

//  API 독립적인 라이브러리만 사용
#include <ktx.h>

// STB 이미지 라이브러리
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <algorithm>
#include <cstring>

namespace BinRenderer
{
	// ========================================
	// Helper Functions
	// ========================================

	static std::string fixPath(const std::string& path)
	{
		std::string fixed = path;
		std::replace(fixed.begin(), fixed.end(), '\\', '/');
		return fixed;
	}

	//  KTX 내부 포맷을 RHI 포맷으로 직접 변환
	static RHIFormat convertKTXFormatToRHI(ktxTexture2* texture, bool isCubemap)
	{
		// KTX2는 내부적으로 Vulkan 포맷을 사용하지만, 
		// 우리는 직접 파싱해서 RHI 포맷으로 변환
		
		// 컬러 모델과 채널 정보 확인
		uint32_t channelCount = texture->vkFormat;
		bool isFloat = (texture->pDfd && texture->pDfd == KHR_DF_MODEL_RGBSDA);
		
		// 큐브맵은 대부분 HDR (16bit float)
		if (isCubemap)
		{
			return RHI_FORMAT_R16G16B16A16_SFLOAT;
		}
		
		// 일반 텍스처는 8bit RGBA
		if (channelCount == 4)
		{
			return RHI_FORMAT_R8G8B8A8_UNORM;
		}
		else if (channelCount == 3)
		{
			return RHI_FORMAT_R8G8B8A8_UNORM; // RGB를 RGBA로 확장
		}
		
		// 기본값
		return RHI_FORMAT_R8G8B8A8_UNORM;
	}

	// ========================================
	// TextureLoader Implementation
	// ========================================

	TextureLoader::TextureLoader(RHI* rhi)
		: rhi_(rhi)
	{
		if (!rhi_)
		{
			printLog("[TextureLoader] ❌ RHI is null!");
		}
	}

	TextureLoader::~TextureLoader()
	{
		// RHI는 외부에서 관리되므로 여기서 삭제하지 않음
	}

	RHITexture* TextureLoader::loadKTX2(const std::string& filename)
	{
		if (!rhi_)
		{
			printLog("[TextureLoader] ❌ RHI is null, cannot load texture");
			return nullptr;
		}

		std::string fixedPath = fixPath(filename);
		printLog("[TextureLoader] Loading KTX2: {}", fixedPath);

		// ========================================
		// 1. KTX2 파일 로드
		// ========================================
		ktxTexture2* ktxTexture2 = nullptr;
		ktxResult result = ktxTexture2_CreateFromNamedFile(
			fixedPath.c_str(),
			KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
			&ktxTexture2
		);

		if (result != KTX_SUCCESS)
		{
			printLog("[TextureLoader] ❌ Failed to load KTX2 file: {} ", fixedPath);
			return nullptr;
		}

		// ========================================
		// 2. 텍스처 정보 추출
		// ========================================
		uint32_t width = ktxTexture2->baseWidth;
		uint32_t height = ktxTexture2->baseHeight;
		uint32_t depth = ktxTexture2->baseDepth;
		uint32_t mipLevels = ktxTexture2->numLevels;
		bool isCubemap = (ktxTexture2->numFaces == 6);
		uint32_t arrayLayers = isCubemap ? 6 : ktxTexture2->numLayers;

		//  API 독립적인 포맷 변환
		RHIFormat rhiFormat = convertKTXFormatToRHI(ktxTexture2, isCubemap);

		printLog("[TextureLoader] Texture info:");
		printLog("    - Size: {}x{}", width, height);
		printLog("    - Mip levels: {}", mipLevels);
		printLog("    - Array layers: {}", arrayLayers);
		printLog("    - Cubemap: {}", isCubemap ? "YES" : "NO");
		printLog("    - Format: {}", static_cast<int>(rhiFormat));

		// ========================================
		// 3. 텍스처 데이터 추출
		// ========================================
		ktxTexture* baseTexture = ktxTexture(ktxTexture2);
		ktx_uint8_t* ktxData = ktxTexture_GetData(baseTexture);
		ktx_size_t ktxSize = ktxTexture_GetDataSize(baseTexture);

		std::vector<uint8_t> textureData(ktxSize);
		std::memcpy(textureData.data(), ktxData, ktxSize);

		// Mipmap 오프셋 정보 추출
		struct MipInfo {
			size_t offset;
			uint32_t width;
			uint32_t height;
		};
		std::vector<std::vector<MipInfo>> mipInfos(arrayLayers);

		if (isCubemap)
		{
			for (uint32_t face = 0; face < 6; ++face)
			{
				mipInfos[face].resize(mipLevels);
				for (uint32_t level = 0; level < mipLevels; ++level)
				{
					ktx_size_t offset = 0;
					ktxTexture_GetImageOffset(baseTexture, level, 0, face, &offset);
					
					mipInfos[face][level].offset = offset;
					mipInfos[face][level].width = std::max(1u, width >> level);
					mipInfos[face][level].height = std::max(1u, height >> level);
				}
			}
		}
		else
		{
			mipInfos[0].resize(mipLevels);
			for (uint32_t level = 0; level < mipLevels; ++level)
			{
				ktx_size_t offset = 0;
				ktxTexture_GetImageOffset(baseTexture, level, 0, 0, &offset);
				
				mipInfos[0][level].offset = offset;
				mipInfos[0][level].width = std::max(1u, width >> level);
				mipInfos[0][level].height = std::max(1u, height >> level);
			}
		}

		// KTX2 텍스처 정리
		ktxTexture_Destroy(ktxTexture(ktxTexture2));

		// ========================================
		// 4. RHIImage 생성
		// ========================================
		RHIImageCreateInfo imageInfo{};
		imageInfo.width = width;
		imageInfo.height = height;
		imageInfo.depth = depth;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = arrayLayers;
		imageInfo.format = rhiFormat;
		imageInfo.tiling = RHI_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = RHI_IMAGE_USAGE_SAMPLED_BIT | RHI_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.samples = RHI_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = isCubemap ? RHI_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;

		RHIImageHandle image = rhi_->createImage(imageInfo);
		if (!image.isValid())
		{
			printLog("[TextureLoader] ❌ Failed to create RHIImage");
			return nullptr;
		}

		// ========================================
		// 5. 텍스처 데이터 업로드
		// ========================================
		{
			// 스테이징 버퍼 생성
			RHIBufferCreateInfo stagingInfo{};
			stagingInfo.size = textureData.size();
			stagingInfo.usage = RHI_BUFFER_USAGE_TRANSFER_SRC_BIT;
			stagingInfo.memoryProperties = RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT;

			RHIBufferHandle stagingBuffer = rhi_->createBuffer(stagingInfo);
			if (!stagingBuffer.isValid())
			{
				printLog("[TextureLoader] ❌ Failed to create staging buffer");
				rhi_->destroyImage(image);
				return nullptr;
			}

			// 데이터 복사
			void* mappedData = rhi_->mapBuffer(stagingBuffer);
			if (mappedData)
			{
				std::memcpy(mappedData, textureData.data(), textureData.size());
				rhi_->unmapBuffer(stagingBuffer);
			}

			// 커맨드 버퍼 기록
			rhi_->beginCommandRecording();

			// 레이아웃 전환: UNDEFINED -> TRANSFER_DST
			rhi_->cmdTransitionImageLayout(
				image,
				RHI_IMAGE_LAYOUT_UNDEFINED,
				RHI_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				RHI_IMAGE_ASPECT_COLOR_BIT,
				0, mipLevels,
				0, arrayLayers
			);

			// ⚠️ TODO: RHI에 cmdCopyBufferToImage 추가 필요
			// 지금은 레이아웃 전환만 수행하고 데이터는 복사하지 않음
			printLog("[TextureLoader] ⚠️  Data copy skipped - need RHI::cmdCopyBufferToImage()");

			// 레이아웃 전환: TRANSFER_DST -> SHADER_READ_ONLY
			rhi_->cmdTransitionImageLayout(
				image,
				RHI_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				RHI_IMAGE_ASPECT_COLOR_BIT,
				0, mipLevels,
				0, arrayLayers
			);

			rhi_->endCommandRecording();
			rhi_->submitCommands();
			rhi_->waitIdle();

			rhi_->destroyBuffer(stagingBuffer);
		}

		// ========================================
		// 6. RHIImageView 생성
		// ========================================
		RHIImageViewCreateInfo viewInfo{};
		viewInfo.viewType = isCubemap ? RHI_IMAGE_VIEW_TYPE_CUBE : RHI_IMAGE_VIEW_TYPE_2D;
		viewInfo.aspectMask = RHI_IMAGE_ASPECT_COLOR_BIT;

		RHIImageViewHandle imageView = rhi_->createImageView(image, viewInfo);
		if (!imageView.isValid())
		{
			printLog("[TextureLoader] ❌ Failed to create RHIImageView");
			rhi_->destroyImage(image);
			return nullptr;
		}

		// ========================================
		// 7. RHISampler 생성
		// ========================================
		RHISamplerCreateInfo samplerInfo{};
		samplerInfo.magFilter = RHI_FILTER_LINEAR;
		samplerInfo.minFilter = RHI_FILTER_LINEAR;
		samplerInfo.addressModeU = RHI_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = RHI_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = RHI_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.maxLod = static_cast<float>(mipLevels);

		RHISamplerHandle sampler = rhi_->createSampler(samplerInfo);
		if (!sampler.isValid())
		{
			printLog("[TextureLoader] ⚠️  Failed to create RHISampler");
		}

		printLog("[TextureLoader]  Successfully loaded KTX2 texture: {}", filename);
		printLog("[TextureLoader]   idx   Image: {}, ImageView: {}, Sampler: {}", 
			image.getIndex(), imageView.getIndex(), sampler.getIndex());

		// ⚠️ TODO: RHI에 createTexture(image, view, sampler) 팩토리 메서드 추가 필요
		// 지금은 nullptr 반환 (리소스는 생성됨)
		return nullptr;
	}

	RHITextureHandle TextureLoader::loadKTX2Handle(const std::string& filename)
	{
		if (!rhi_)
		{
			printLog("[TextureLoader] ❌ RHI is null");
			return {};
		}

		std::string fixedPath = fixPath(filename);
		
		// ========================================
		// 1. KTX2 파일 로드
		// ========================================
		ktxTexture2* ktxTexture2 = nullptr;
		ktxResult result = ktxTexture2_CreateFromNamedFile(
			fixedPath.c_str(),
			KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
			&ktxTexture2
		);

		if (result != KTX_SUCCESS)
		{
			printLog("[TextureLoader] ❌ Failed to load KTX2 file: {} ", fixedPath);
			return {};
		}

		// 메타데이터 추출
		uint32_t width = ktxTexture2->baseWidth;
		uint32_t height = ktxTexture2->baseHeight;
		uint32_t mipLevels = ktxTexture2->numLevels;
		bool isCubemap = (ktxTexture2->numFaces == 6);
		uint32_t arrayLayers = isCubemap ? 6 : ktxTexture2->numLayers;
		RHIFormat rhiFormat = convertKTXFormatToRHI(ktxTexture2, isCubemap);

		// 데이터 추출
		ktxTexture* baseTexture = ktxTexture(ktxTexture2);
		ktx_uint8_t* ktxData = ktxTexture_GetData(baseTexture);
		ktx_size_t ktxSize = ktxTexture_GetDataSize(baseTexture);
		std::vector<uint8_t> textureData(ktxSize);
		std::memcpy(textureData.data(), ktxData, ktxSize);
		
		ktxTexture_Destroy(ktxTexture(ktxTexture2));

		// ========================================
		// 2. RHIImage 생성 (Handle 반환)
		// ========================================
		RHIImageCreateInfo imageInfo{};
		imageInfo.width = width;
		imageInfo.height = height;
		imageInfo.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = arrayLayers;
		imageInfo.format = rhiFormat;
		imageInfo.tiling = RHI_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = RHI_IMAGE_USAGE_SAMPLED_BIT | RHI_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.flags = isCubemap ? RHI_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;

		RHIImageHandle imageHandle = rhi_->createImage(imageInfo);
		
		if (!imageHandle.isValid())
		{
			printLog("[TextureLoader] ❌ Failed to create RHIImage Handle");
			return {};
		}

		// ========================================
		// 3. 데이터 업로드 (Handle 사용)
		// ========================================
		{
			RHIBufferCreateInfo stagingInfo{};
			stagingInfo.size = textureData.size();
			stagingInfo.usage = RHI_BUFFER_USAGE_TRANSFER_SRC_BIT;
			stagingInfo.memoryProperties = RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT;

			RHIBufferHandle stagingBuffer = rhi_->createBuffer(stagingInfo);
			
			if (stagingBuffer.isValid())
			{
				void* mappedData = rhi_->mapBuffer(stagingBuffer);
				if (mappedData)
				{
					std::memcpy(mappedData, textureData.data(), textureData.size());
					rhi_->unmapBuffer(stagingBuffer);
				}

				rhi_->beginCommandRecording();

				rhi_->cmdTransitionImageLayout(
					imageHandle,
					RHI_IMAGE_LAYOUT_UNDEFINED,
					RHI_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					RHI_IMAGE_ASPECT_COLOR_BIT,
					0, mipLevels, 0, arrayLayers
				);

				// TODO: Copy Buffer To Image 구현 필요 (현재는 생략, 실제로는 필요함)
				// rhi_->cmdCopyBufferToImage(stagingBuffer, imageHandle, ...);
				
				rhi_->cmdTransitionImageLayout(
					imageHandle,
					RHI_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					RHI_IMAGE_ASPECT_COLOR_BIT,
					0, mipLevels, 0, arrayLayers
				);

				rhi_->endCommandRecording();
				rhi_->submitCommands();
				rhi_->waitIdle();

				rhi_->destroyBuffer(stagingBuffer);
			}
		}

		// ========================================
		// 4. View & Sampler 생성 (Handle 사용)
		// ========================================
		RHIImageViewCreateInfo viewInfo{};
		viewInfo.viewType = isCubemap ? RHI_IMAGE_VIEW_TYPE_CUBE : RHI_IMAGE_VIEW_TYPE_2D;
		viewInfo.aspectMask = RHI_IMAGE_ASPECT_COLOR_BIT;

		RHIImageViewHandle viewHandle = rhi_->createImageView(imageHandle, viewInfo);

		RHISamplerCreateInfo samplerInfo{};
		samplerInfo.magFilter = RHI_FILTER_LINEAR;
		samplerInfo.minFilter = RHI_FILTER_LINEAR;
		samplerInfo.maxLod = static_cast<float>(mipLevels);

		RHISamplerHandle samplerHandle = rhi_->createSampler(samplerInfo);

		// ========================================
		// 5. 최종 Texture Handle 생성 및 반환
		// ========================================
		return rhi_->createTexture(imageHandle, viewHandle, samplerHandle);
	}

	RHITexture* TextureLoader::loadImage(const std::string& filename, bool sRGB)
	{
		if (!rhi_)
		{
			printLog("[TextureLoader] ❌ RHI is null, cannot load texture");
			return nullptr;
		}

		std::string fixedPath = fixPath(filename);
		printLog("[TextureLoader] Loading image: {} (sRGB: {})", fixedPath, sRGB);

		// ========================================
		// 1. STB로 이미지 로드
		// ========================================
		int width, height, channels;
		unsigned char* pixels = stbi_load(fixedPath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

		if (!pixels)
		{
			printLog("[TextureLoader] ❌ Failed to load image: {} ({})", fixedPath, stbi_failure_reason());
			return nullptr;
		}

		RHIFormat format = sRGB ? RHI_FORMAT_R8G8B8A8_SRGB : RHI_FORMAT_R8G8B8A8_UNORM;
		size_t dataSize = width * height * 4; // RGBA

		printLog("[TextureLoader] Image info:");
		printLog("    - Size: {}x{}", width, height);
		printLog("    - Channels: {} (converted to RGBA)", channels);
		printLog("    - Format: {}", static_cast<int>(format));

		// ========================================
		// 2. RHIImage 생성
		// ========================================
		RHIImageCreateInfo imageInfo{};
		imageInfo.width = static_cast<uint32_t>(width);
		imageInfo.height = static_cast<uint32_t>(height);
		imageInfo.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = RHI_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = RHI_IMAGE_USAGE_SAMPLED_BIT | RHI_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.samples = RHI_SAMPLE_COUNT_1_BIT;

		RHIImageHandle image = rhi_->createImage(imageInfo);
		if (!image.isValid())
		{
			printLog("[TextureLoader] ❌ Failed to create RHIImage");
			stbi_image_free(pixels);
			return nullptr;
		}

		// ========================================
		// 3. 텍스처 데이터 업로드
		// ========================================
		{
			RHIBufferCreateInfo stagingInfo{};
			stagingInfo.size = dataSize;
			stagingInfo.usage = RHI_BUFFER_USAGE_TRANSFER_SRC_BIT;
			stagingInfo.memoryProperties = RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT;

			RHIBufferHandle stagingBuffer = rhi_->createBuffer(stagingInfo);
			if (!stagingBuffer.isValid())
			{
				printLog("[TextureLoader] ❌ Failed to create staging buffer");
				rhi_->destroyImage(image);
				stbi_image_free(pixels);
				return nullptr;
			}

			void* mappedData = rhi_->mapBuffer(stagingBuffer);
			if (mappedData)
			{
				std::memcpy(mappedData, pixels, dataSize);
				rhi_->unmapBuffer(stagingBuffer);
			}

			stbi_image_free(pixels);

			rhi_->beginCommandRecording();

			rhi_->cmdTransitionImageLayout(
				image,
				RHI_IMAGE_LAYOUT_UNDEFINED,
				RHI_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				RHI_IMAGE_ASPECT_COLOR_BIT,
				0, 1, 0, 1
			);

			// ⚠️ TODO: RHI::cmdCopyBufferToImage 필요
			printLog("[TextureLoader] ⚠️  Data copy skipped - need RHI::cmdCopyBufferToImage()");

			rhi_->cmdTransitionImageLayout(
				image,
				RHI_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				RHI_IMAGE_ASPECT_COLOR_BIT,
				0, 1, 0, 1
			);

			rhi_->endCommandRecording();
			rhi_->submitCommands();
			rhi_->waitIdle();

			rhi_->destroyBuffer(stagingBuffer);
		}

		// ========================================
		// 4. RHIImageView 생성
		// ========================================
		RHIImageViewCreateInfo viewInfo{};
		viewInfo.viewType = RHI_IMAGE_VIEW_TYPE_2D;
		viewInfo.aspectMask = RHI_IMAGE_ASPECT_COLOR_BIT;

		RHIImageViewHandle imageView = rhi_->createImageView(image, viewInfo);
		if (!imageView.isValid())
		{
			printLog("[TextureLoader] ❌ Failed to create RHIImageView");
			rhi_->destroyImage(image);
			return nullptr;
		}

		// ========================================
		// 5. RHISampler 생성
		// ========================================
		RHISamplerCreateInfo samplerInfo{};
		samplerInfo.magFilter = RHI_FILTER_LINEAR;
		samplerInfo.minFilter = RHI_FILTER_LINEAR;
		samplerInfo.addressModeU = RHI_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = RHI_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = RHI_SAMPLER_ADDRESS_MODE_REPEAT;

		RHISamplerHandle sampler = rhi_->createSampler(samplerInfo);

		printLog("[TextureLoader]  Successfully loaded image texture: {}", filename);
		printLog("[TextureLoader]     Image: {}, ImageView: {}, Sampler: {}", 
			image.getIndex(),imageView.getIndex(), sampler.getIndex());

		// ⚠️ TODO: RHITexture 구현체 반환 필요
		return nullptr;
	}


	RHITexture* TextureLoader::createTextureFromData(const LoadedTextureData& loadedData)
	{
		// 레거시 함수 - 사용하지 않음
		return nullptr;
	}

	void TextureLoader::uploadTextureData(RHIImage* image, const LoadedTextureData& loadedData)
	{
		// 레거시 함수 - 사용하지 않음
	}

} // namespace BinRenderer
