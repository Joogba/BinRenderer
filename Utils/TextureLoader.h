#pragma once

#include "../RHI/Core/RHI.h"
#include "../RHI/Resources/RHITexture.h"
#include "../RHI/Core/RHIHandle.h"
#include <string>
#include <memory>

namespace BinRenderer
{
	/**
	 * @brief API 독립적인 텍스처 로더
	 * 
	 * KTX 라이브러리와 STB를 직접 사용하여 텍스처를 로드하고
	 * RHI 인터페이스를 통해 GPU에 업로드합니다.
	 * 
	 *  RHITextureLoader를 사용하지 않고 직접 KTX 파싱
	 *  큐브맵 지원 (KTX2)
	 *  PNG/JPEG 지원 (STB)
	 */
	class TextureLoader
	{
	public:
		/**
		 * @brief 생성자
		 * @param rhi RHI 인터페이스 (Vulkan/DX12/Metal 구현체)
		 */
		explicit TextureLoader(RHI* rhi);
		~TextureLoader();

		/**
		 * @brief KTX2 파일에서 텍스처 로드 (큐브맵 지원)
		 * 
		 * @param filename KTX2 파일 경로
		 * @return 생성된 RHITexture (실패시 nullptr)
		 * 
		 * @code
		 * auto* loader = new TextureLoader(rhi);
		 * auto* cubemap = loader->loadKTX2("assets/textures/cubemap.ktx2");
		 * if (cubemap) {
		 *     // 큐브맵 사용
		 *     descriptorSet->updateImage(0, cubemap->getImageView(), cubemap->getSampler());
		 * }
		 * @endcode
		 */
		RHITexture* loadKTX2(const std::string& filename);

		/**
		 * @brief KTX2 파일에서 텍스처 로드 (Handle 반환)
		 */
		RHITextureHandle loadKTX2Handle(const std::string& filename);

		/**
		 * @brief PNG/JPEG 파일에서 2D 텍스처 로드
		 * 
		 * @param filename 이미지 파일 경로
		 * @param sRGB sRGB 색상 공간 사용 여부
		 * @return 생성된 RHITexture (실패시 nullptr)
		 */
		RHITexture* loadImage(const std::string& filename, bool sRGB = false);

	private:
		RHI* rhi_;

		// ⚠️ 아래 함수들은 더 이상 사용하지 않음 (레거시)
		struct LoadedTextureData; // Forward declaration for compatibility
		RHITexture* createTextureFromData(const LoadedTextureData& loadedData);
		void uploadTextureData(RHIImage* image, const LoadedTextureData& loadedData);
	};

} // namespace BinRenderer
