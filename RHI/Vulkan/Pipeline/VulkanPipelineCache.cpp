#include "VulkanPipelineCache.h"
#include "Core/Logger.h"
#include <fstream>

namespace BinRenderer::Vulkan
{
	VulkanPipelineCache::VulkanPipelineCache(VkDevice device)
		: device_(device)
	{
	}

	VulkanPipelineCache::~VulkanPipelineCache()
	{
		destroy();
	}

	bool VulkanPipelineCache::create(const void* initialData, size_t initialDataSize)
	{
		VkPipelineCacheCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		createInfo.initialDataSize = initialDataSize;
		createInfo.pInitialData = initialData;

		if (vkCreatePipelineCache(device_, &createInfo, nullptr, &cache_) != VK_SUCCESS)
		{
			printLog("ERROR: Failed to create pipeline cache");
			return false;
		}

		printLog(" Pipeline cache created (initial size: {} bytes)", initialDataSize);
		return true;
	}

	bool VulkanPipelineCache::loadFromFile(const std::string& filename)
	{
		// 파일 읽기
		auto data = readFile(filename);

		if (data.empty())
		{
			printLog("⚠️ Pipeline cache file not found: {}, creating new cache", filename);
			return create();  // 파일 없으면 빈 캐시 생성
		}

		// 파일 데이터로 캐시 생성
		if (!create(data.data(), data.size()))
		{
			printLog("ERROR: Failed to create pipeline cache from file: {}", filename);
			return false;
		}

		printLog(" Pipeline cache loaded from file: {} ({} bytes)", filename, data.size());
		return true;
	}

	bool VulkanPipelineCache::saveToFile(const std::string& filename)
	{
		if (cache_ == VK_NULL_HANDLE)
		{
			printLog("ERROR: Cannot save invalid pipeline cache");
			return false;
		}

		// Cache 데이터 가져오기
		auto data = getCacheData();

		if (data.empty())
		{
			printLog("WARNING: Pipeline cache is empty, skipping save");
			return false;
		}

		// 파일로 저장
		if (!writeFile(filename, data.data(), data.size()))
		{
			printLog("ERROR: Failed to write pipeline cache to file: {}", filename);
			return false;
		}

		printLog(" Pipeline cache saved to file: {} ({} bytes)", filename, data.size());
		return true;
	}

	std::vector<uint8_t> VulkanPipelineCache::getCacheData() const
	{
		if (cache_ == VK_NULL_HANDLE)
		{
			return {};
		}

		// Cache 크기 확인
		size_t dataSize = 0;
		vkGetPipelineCacheData(device_, cache_, &dataSize, nullptr);

		if (dataSize == 0)
		{
			return {};
		}

		// Cache 데이터 가져오기
		std::vector<uint8_t> data(dataSize);
		VkResult result = vkGetPipelineCacheData(device_, cache_, &dataSize, data.data());

		if (result != VK_SUCCESS)
		{
			printLog("ERROR: Failed to get pipeline cache data");
			return {};
		}

		return data;
	}

	size_t VulkanPipelineCache::getCacheSize() const
	{
		if (cache_ == VK_NULL_HANDLE)
		{
			return 0;
		}

		size_t dataSize = 0;
		vkGetPipelineCacheData(device_, cache_, &dataSize, nullptr);
		return dataSize;
	}

	void VulkanPipelineCache::destroy()
	{
		if (cache_ != VK_NULL_HANDLE)
		{
			vkDestroyPipelineCache(device_, cache_, nullptr);
			cache_ = VK_NULL_HANDLE;
		}
	}

	// ========================================
	// Private 헬퍼 메서드
	// ========================================

	std::vector<uint8_t> VulkanPipelineCache::readFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::binary | std::ios::ate);

		if (!file.is_open())
		{
			return {};
		}

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<uint8_t> buffer(fileSize);

		file.seekg(0);
		file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
		file.close();

		return buffer;
	}

	bool VulkanPipelineCache::writeFile(const std::string& filename, const void* data, size_t size)
	{
		std::ofstream file(filename, std::ios::binary);

		if (!file.is_open())
		{
			return false;
		}

		file.write(reinterpret_cast<const char*>(data), size);
		file.close();

		return true;
	}

} // namespace BinRenderer::Vulkan
