#pragma once

#include "../Resources/ResourceManager.h"
#include <memory>
#include <string>
#include <unordered_map>

// Forward declarations
namespace BinRenderer::Vulkan {
	class Model;
	class Context;
	class ResourceRegistry;
	class Image2D;  // ✅ Image2D forward declaration 추가
}

namespace BinRenderer::Vulkan {

	/**
	 * @brief Vulkan API 전용 리소스 매니저
	 * 
	 * - GPU 리소스 (VkImage, VkBuffer) 관리 (ResourceRegistry)
	 * - Model 캐싱 및 로딩
	 * - 플랫폼 독립적 ResourceManager와 연동
	 */
	class VulkanResourceManager
	{
	public:
		/**
		 * @brief 생성자
		 * @param baseResourceManager 플랫폼 독립적 리소스 매니저 (메타데이터)
		 * @param ctx Vulkan Context
		 */
		VulkanResourceManager(BinRenderer::ResourceManager& baseResourceManager, Context& ctx);
		~VulkanResourceManager();

		// ========================================
		// 플랫폼 독립적 리소스 접근
		// ========================================

		/**
		 * @brief 기본 ResourceManager 접근 (메타데이터)
		 */
		BinRenderer::ResourceManager& GetBaseResourceManager()
		{
			return baseResourceManager_;
		}

		// ========================================
		// Vulkan GPU 리소스 관리
		// ========================================

		/**
		 * @brief Vulkan GPU 리소스 레지스트리 접근
		 */
		ResourceRegistry& GetGpuResources();

		// ========================================
		// Model 캐싱 (Vulkan 전용)
		// ========================================

		/**
		 * @brief 모델 로딩 또는 캐시된 모델 반환
		 * @param resourcePath 모델 파일 경로 (캐시 키)
		 * @return 캐시된 또는 새로 로드된 모델의 shared_ptr
		 */
		std::shared_ptr<Model> LoadOrGetModel(const std::string& resourcePath);

		/**
		 * @brief 모델 캐시에서 제거
		 */
		void UnloadModel(const std::string& resourcePath);

		/**
		 * @brief 모든 모델 캐시 초기화
		 */
		void ClearModelCache();

		/**
		 * @brief 캐시된 모델 개수 반환
		 */
		size_t GetModelCacheSize() const
		{
			return modelCache_.size();
		}

		// ========================================
		// Texture Loading Helpers
		// ========================================

		/**
		 * @brief 텍스처 로드 또는 캐시된 텍스처 반환
		 * @param texturePath 텍스처 파일 경로
		 * @param sRGB sRGB 포맷 사용 여부
		 * @return 텍스처의 Image2D shared_ptr
		 */
		std::shared_ptr<Image2D> LoadOrGetTexture(const std::string& texturePath, bool sRGB = false);

		/**
		 * @brief 로드된 텍스처를 플랫폼 독립적 Registry에 등록
		 */
		void RegisterTextureMetadata(const std::string& name, const std::shared_ptr<Image2D>& texture);

	private:
		// 플랫폼 독립적 리소스 매니저 (참조)
		BinRenderer::ResourceManager& baseResourceManager_;

		// Vulkan Context (참조)
		Context& ctx_;

		// GPU 리소스 레지스트리
		std::unique_ptr<ResourceRegistry> gpuResourceRegistry_;

		// Model 캐시 (파일 경로 → Model)
		std::unordered_map<std::string, std::shared_ptr<Model>> modelCache_;

		// Texture 캐시 (파일 경로 → Image2D)
		std::unordered_map<std::string, std::shared_ptr<Image2D>> textureCache_;
	};

} // namespace BinRenderer::Vulkan
