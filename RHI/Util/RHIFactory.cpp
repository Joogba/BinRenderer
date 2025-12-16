#include "RHIFactory.h"
#include "../Vulkan/VulkanRHI.h"

#ifdef _WIN32
#define PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define PLATFORM_MACOS
#elif defined(__linux__)
#define PLATFORM_LINUX
#endif

namespace BinRenderer
{
	RHI* RHIFactory::create(RHIApiType apiType)
	{
		switch (apiType)
		{
		case RHIApiType::Vulkan:
			return new Vulkan::VulkanRHI();

		case RHIApiType::D3D12:
			// TODO: D3D12 구현
			return nullptr;

		case RHIApiType::Metal:
			// TODO: Metal 구현
			return nullptr;

		case RHIApiType::OpenGL:
			// TODO: OpenGL 구현
			return nullptr;

		default:
			return nullptr;
		}
	}

	std::unique_ptr<RHI> RHIFactory::createUnique(RHIApiType apiType)
	{
		return std::unique_ptr<RHI>(create(apiType));
	}

	bool RHIFactory::isApiSupported(RHIApiType apiType)
	{
		switch (apiType)
		{
		case RHIApiType::Vulkan:
			return true; // Vulkan은 현재 지원됨

		case RHIApiType::D3D12:
#ifdef PLATFORM_WINDOWS
			return false; // TODO: D3D12 구현 후 true
#else
			return false;
#endif

		case RHIApiType::Metal:
#ifdef PLATFORM_MACOS
			return false; // TODO: Metal 구현 후 true
#else
			return false;
#endif

		case RHIApiType::OpenGL:
			return false; // TODO: OpenGL 구현 후 true

		default:
			return false;
		}
	}

	RHIApiType RHIFactory::getRecommendedApi()
	{
#ifdef PLATFORM_WINDOWS
		// Windows: Vulkan 우선, D3D12 대체
		if (isApiSupported(RHIApiType::Vulkan))
			return RHIApiType::Vulkan;
		if (isApiSupported(RHIApiType::D3D12))
			return RHIApiType::D3D12;
#elif defined(PLATFORM_MACOS)
		// macOS: Metal 우선, Vulkan 대체
		if (isApiSupported(RHIApiType::Metal))
			return RHIApiType::Metal;
		if (isApiSupported(RHIApiType::Vulkan))
			return RHIApiType::Vulkan;
#else
		// Linux: Vulkan 우선
		if (isApiSupported(RHIApiType::Vulkan))
			return RHIApiType::Vulkan;
#endif

		// 기본값
		return RHIApiType::Vulkan;
	}

} // namespace BinRenderer
