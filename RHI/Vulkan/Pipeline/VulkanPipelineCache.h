#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace BinRenderer::Vulkan
{
	/**
	 * @brief Vulkan Pipeline Cache
	 * 
	 * Pipeline 생성 속도 향상을 위한 캐싱 시스템
	 * - 파일로 저장/로드
	 * - 자동 관리
	 */
	class VulkanPipelineCache
	{
	public:
		VulkanPipelineCache(VkDevice device);
		~VulkanPipelineCache();

		/**
		 * @brief Pipeline cache 생성
		 * @param initialData 초기 데이터 (nullptr 가능)
		 * @param initialDataSize 초기 데이터 크기
		 * @return 성공 여부
		 */
		bool create(const void* initialData = nullptr, size_t initialDataSize = 0);

		/**
		 * @brief 파일에서 로드
		 * @param filename 파일 경로
		 * @return 성공 여부
		 */
		bool loadFromFile(const std::string& filename);

		/**
		 * @brief 파일로 저장
		 * @param filename 파일 경로
		 * @return 성공 여부
		 */
		bool saveToFile(const std::string& filename);

		/**
		 * @brief Cache 데이터 가져오기
		 * @return Cache 데이터 (벡터)
		 */
		std::vector<uint8_t> getCacheData() const;

		/**
		 * @brief Cache 크기
		 * @return Cache 데이터 크기 (bytes)
		 */
		size_t getCacheSize() const;

		/**
		 * @brief VkPipelineCache 접근
		 */
		VkPipelineCache getVkPipelineCache() const { return cache_; }

		/**
		 * @brief Cache 유효성 확인
		 */
		bool isValid() const { return cache_ != VK_NULL_HANDLE; }

		/**
		 * @brief Cache 제거
		 */
		void destroy();

	private:
		VkDevice device_;
		VkPipelineCache cache_ = VK_NULL_HANDLE;

		/**
		 * @brief 파일 읽기
		 */
		std::vector<uint8_t> readFile(const std::string& filename);

		/**
		 * @brief 파일 쓰기
		 */
		bool writeFile(const std::string& filename, const void* data, size_t size);
	};

} // namespace BinRenderer::Vulkan
