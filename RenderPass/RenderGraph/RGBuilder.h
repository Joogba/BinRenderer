#pragma once

#include "RGTypes.h"
//  전방 선언으로 변경 (순환 참조 방지)
// #include "../RGPassBase.h"  
#include "../../RHI/Core/RHI.h"
#include <vector>
#include <unordered_map>

namespace BinRenderer
{
	//  전방 선언
	class RGPassBase;

	/**
	 * @brief RenderGraph Builder
	 * 
	 * 패스 Setup 단계에서 리소스 생성 및 의존성 선언
	 */
	class RenderGraphBuilder
	{
	public:
		RenderGraphBuilder() = default;

		// ========================================
		// 텍스처 생성 및 관리
		// ========================================
		
		/**
		 * @brief 새로운 텍스처 생성
		 */
		RGTextureHandle createTexture(const RGTextureDesc& desc);

		/**
		 * @brief 외부 텍스처 임포트 (예: 스왑체인 이미지)
		 */
		RGTextureHandle importTexture(const std::string& name, RHIImageHandle image, const RGTextureDesc& desc);

		/**
		 * @brief 텍스처 읽기 선언
		 */
		RGTextureHandle readTexture(RGTextureHandle handle);

		/**
		 * @brief 텍스처 쓰기 선언
		 */
		RGTextureHandle writeTexture(RGTextureHandle handle);

		/**
		 * @brief 텍스처 읽기/쓰기 선언
		 */
		RGTextureHandle readWriteTexture(RGTextureHandle handle);

		// ========================================
		// 버퍼 생성 및 관리
		// ========================================

		/**
		 * @brief 새로운 버퍼 생성
		 */
		RGBufferHandle createBuffer(const RGBufferDesc& desc);

		/**
		 * @brief 외부 버퍼 임포트
		 */
		RGBufferHandle importBuffer(const std::string& name, RHIBufferHandle buffer, const RGBufferDesc& desc);

		/**
		 * @brief 버퍼 읽기 선언
		 */
		RGBufferHandle readBuffer(RGBufferHandle handle);

		/**
		 * @brief 버퍼 쓰기 선언
		 */
		RGBufferHandle writeBuffer(RGBufferHandle handle);

		/**
		 * @brief 버퍼 읽기/쓰기 선언
		 */
		RGBufferHandle readWriteBuffer(RGBufferHandle handle);

		// ========================================
		// 최종 출력 설정
		// ========================================

		/**
		 * @brief 최종 출력 텍스처 설정
		 */
		void setFinalOutput(RGTextureHandle handle);

		RGTextureHandle getFinalOutput() const { return finalOutput_; }

	private:
		friend class RenderGraph;

		// 텍스처 노드
		struct TextureNode
		{
			RGTextureDesc desc;
			RHIImageHandle importedImage;
			uint32_t firstUse = UINT32_MAX;
			uint32_t lastUse = 0;
			bool isRead = false;
			bool isWritten = false;
		};

		// 버퍼 노드
		struct BufferNode
		{
			RGBufferDesc desc;
			RHIBufferHandle importedBuffer;
			uint32_t firstUse = UINT32_MAX;
			uint32_t lastUse = 0;
			bool isRead = false;
			bool isWritten = false;
		};

		std::vector<TextureNode> textures_;
		std::vector<BufferNode> buffers_;
		RGTextureHandle finalOutput_;

		RGPassBase* currentPass_ = nullptr;
		uint32_t currentPassIndex_ = 0;

		// 의존성 추가 헬퍼
		void addTextureDependency(RGTextureHandle handle, RGResourceAccessType accessType);
		void addBufferDependency(RGBufferHandle handle, RGResourceAccessType accessType);
	};

} // namespace BinRenderer
