#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

namespace BinRenderer::Vulkan
{
	/**
	 * @brief 간단한 SPIRV 셰이더 캐시
	 * 
	 * SPIRV 바이너리를 메모리에 캐싱하여 중복 로딩 방지
	 */
	class ShaderCache
	{
	public:
		/**
		 * @brief SPIRV 바이너리 로드 또는 캐시에서 가져오기
		 * @param filepath SPIRV 파일 경로
		 * @return SPIRV 바이너리 (참조)
		 */
		const std::vector<uint32_t>& loadOrGet(const std::string& filepath);

		/**
		 * @brief 캐시에서 제거
		 * @param filepath SPIRV 파일 경로
		 */
		void remove(const std::string& filepath);

		/**
		 * @brief 모든 캐시 제거
		 */
		void clear();

		/**
		 * @brief 캐시 크기
		 */
		size_t size() const { return cache_.size(); }

		/**
		 * @brief 캐시에 있는지 확인
		 */
		bool contains(const std::string& filepath) const;

	private:
		std::unordered_map<std::string, std::vector<uint32_t>> cache_;

		/**
		 * @brief SPIRV 파일 로드
		 */
		std::vector<uint32_t> loadSPIRV(const std::string& filepath);
	};

	/**
	 * @brief 간단한 리소스 캐시 (템플릿)
	 * 
	 * 일반적인 리소스 캐싱용
	 */
	template<typename T>
	class ResourceCache
	{
	public:
		/**
		 * @brief 리소스 추가
		 */
		void add(const std::string& key, std::shared_ptr<T> resource)
		{
			cache_[key] = resource;
		}

		/**
		 * @brief 리소스 가져오기
		 */
		std::shared_ptr<T> get(const std::string& key)
		{
			auto it = cache_.find(key);
			if (it != cache_.end())
			{
				return it->second;
			}
			return nullptr;
		}

		/**
		 * @brief 캐시에서 제거
		 */
		void remove(const std::string& key)
		{
			cache_.erase(key);
		}

		/**
		 * @brief 모든 캐시 제거
		 */
		void clear()
		{
			cache_.clear();
		}

		/**
		 * @brief 캐시 크기
		 */
		size_t size() const { return cache_.size(); }

		/**
		 * @brief 캐시에 있는지 확인
		 */
		bool contains(const std::string& key) const
		{
			return cache_.find(key) != cache_.end();
		}

	private:
		std::unordered_map<std::string, std::shared_ptr<T>> cache_;
	};

} // namespace BinRenderer::Vulkan
