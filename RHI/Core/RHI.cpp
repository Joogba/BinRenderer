#include "RHI.h"
#include "../Vulkan/VulkanRHI.h"

namespace BinRenderer
{
	RHI* RHIFactory::createRHI(RHIApiType apiType)
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

} // namespace BinRenderer
