#include "VulkanResourceCache.h"
#include "Vulkan/Logger.h"
#include <fstream>

namespace BinRenderer::Vulkan
{
	const std::vector<uint32_t>& ShaderCache::loadOrGet(const std::string& filepath)
	{
		// 캐시에 있으면 반환
		auto it = cache_.find(filepath);
		if (it != cache_.end())
		{
			printLog("📦 Shader loaded from cache: {}", filepath);
			return it->second;
		}

		// 파일에서 로드
		auto spirv = loadSPIRV(filepath);
		
		if (spirv.empty())
		{
			printLog("ERROR: Failed to load SPIRV: {}", filepath);
			static std::vector<uint32_t> empty;
			return empty;
		}

		// 캐시에 추가
		cache_[filepath] = std::move(spirv);
		printLog("✅ Shader loaded and cached: {} ({} bytes)", filepath, cache_[filepath].size() * 4);

		return cache_[filepath];
	}

	void ShaderCache::remove(const std::string& filepath)
	{
		cache_.erase(filepath);
	}

	void ShaderCache::clear()
	{
		cache_.clear();
		printLog("🗑️ Shader cache cleared");
	}

	bool ShaderCache::contains(const std::string& filepath) const
	{
		return cache_.find(filepath) != cache_.end();
	}

	std::vector<uint32_t> ShaderCache::loadSPIRV(const std::string& filepath)
	{
		std::ifstream file(filepath, std::ios::binary | std::ios::ate);

		if (!file.is_open())
		{
			printLog("ERROR: Failed to open SPIRV file: {}", filepath);
			return {};
		}

		size_t fileSize = static_cast<size_t>(file.tellg());
		
		if (fileSize % 4 != 0)
		{
			printLog("ERROR: Invalid SPIRV file size: {} (not multiple of 4)", filepath);
			return {};
		}

		std::vector<uint32_t> buffer(fileSize / 4);

		file.seekg(0);
		file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
		file.close();

		return buffer;
	}

} // namespace BinRenderer::Vulkan
