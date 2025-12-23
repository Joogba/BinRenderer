#pragma once

#include "../RHI/Core/RHI.h"
#include <memory>

namespace BinRenderer
{
	/**
	 * @brief 기본 리소스 생성 팩토리
	 * 
	 * Dummy/Default 리소스를 생성하여 Pass 초기화 시 사용
	 * - 모든 Pass에서 재사용 가능
	 * - RHI 레이어만 사용 (플랫폼 독립적)
	 */
	class ResourceFactory
	{
	public:
		explicit ResourceFactory(RHI* rhi);
		~ResourceFactory();

		/**
		 * @brief 기본 2D 텍스처 생성 (흰색)
		 * @param size 텍스처 크기 (NxN)
		 * @return RHIImageView 포인터
		 */
		RHIImageView* createDefaultTexture2D(uint32_t size = 4);

		/**
		 * @brief 기본 Cubemap 생성 (검은색, 6 faces)
		 * @param size Cubemap 크기 (NxN per face)
		 * @return RHIImageView 포인터 (CUBE view type)
		 */
		RHIImageView* createDefaultCubemap(uint32_t size = 4);

		/**
		 * @brief 기본 Depth 텍스처 생성
		 * @param size 텍스처 크기 (NxN)
		 * @return RHIImageView 포인터
		 */
		RHIImageView* createDefaultDepthTexture(uint32_t size = 4);

		/**
		 * @brief 기본 Sampler 생성
		 * @return RHISampler 포인터
		 */
		RHISampler* createDefaultSampler();

		/**
		 * @brief 모든 생성된 리소스 정리
		 */
		void cleanup();

	private:
		RHI* rhi_;

		// 생성된 리소스 추적 (자동 정리)
		std::vector<RHIImage*> images_;
		std::vector<RHIImageView*> imageViews_;
		std::vector<RHISampler*> samplers_;
	};

} // namespace BinRenderer
