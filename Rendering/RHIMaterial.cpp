#include "RHIMaterial.h"
#include "../Core/Logger.h"
#include <fstream>

namespace BinRenderer
{
	void RHIMaterial::loadFromCache(const std::string& cachePath)
	{
		std::ifstream file(cachePath, std::ios::binary);
		if (!file.is_open())
		{
			printLog("Failed to open material cache: {}", cachePath);
			return;
		}

		// TODO: 실제 캐시 로딩 구현
		file.close();
	}

	void RHIMaterial::writeToCache(const std::string& cachePath)
	{
		std::ofstream file(cachePath, std::ios::binary);
		if (!file.is_open())
		{
			printLog("Failed to create material cache: {}", cachePath);
			return;
		}

		// TODO: 실제 캐시 저장 구현
		file.close();
	}

} // namespace BinRenderer
