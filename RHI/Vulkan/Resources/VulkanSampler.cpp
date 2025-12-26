#include "VulkanSampler.h"
#include "Core/Logger.h"

namespace BinRenderer::Vulkan
{
	VulkanSampler::VulkanSampler(VkDevice device)
		: device_(device)
	{
	}

	VulkanSampler::~VulkanSampler()
	{
		destroy();
	}

	bool VulkanSampler::createLinear(VkSamplerAddressMode addressMode)
	{
		return create(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR,
			addressMode, addressMode, addressMode, 1.0f, false);
	}

	bool VulkanSampler::createNearest(VkSamplerAddressMode addressMode)
	{
		return create(VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST,
			addressMode, addressMode, addressMode, 1.0f, false);
	}

	bool VulkanSampler::createAnisotropic(float maxAnisotropy, VkSamplerAddressMode addressMode)
	{
		return create(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR,
			addressMode, addressMode, addressMode, maxAnisotropy, false);
	}

	bool VulkanSampler::createShadow()
	{
		return create(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 1.0f, true);
	}

	bool VulkanSampler::create(VkFilter minFilter, VkFilter magFilter, VkSamplerMipmapMode mipmapMode,
		VkSamplerAddressMode addressModeU, VkSamplerAddressMode addressModeV, VkSamplerAddressMode addressModeW,
		float maxAnisotropy, bool compareEnable)
	{
		minFilter_ = static_cast<RHIFilter>(minFilter);
		magFilter_ = static_cast<RHIFilter>(magFilter);
		mipmapMode_ = static_cast<RHISamplerMipmapMode>(mipmapMode);
		addressModeU_ = static_cast<RHISamplerAddressMode>(addressModeU);
		addressModeV_ = static_cast<RHISamplerAddressMode>(addressModeV);
		addressModeW_ = static_cast<RHISamplerAddressMode>(addressModeW);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = magFilter;
		samplerInfo.minFilter = minFilter;
		samplerInfo.addressModeU = addressModeU;
		samplerInfo.addressModeV = addressModeV;
		samplerInfo.addressModeW = addressModeW;

		// Anisotropic 필터링
		samplerInfo.anisotropyEnable = (maxAnisotropy > 1.0f) ? VK_TRUE : VK_FALSE;
		samplerInfo.maxAnisotropy = maxAnisotropy;

		// 경계 색상
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

		// 텍스처 좌표 정규화
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		// 비교 연산 (그림자 매핑용)
		samplerInfo.compareEnable = compareEnable ? VK_TRUE : VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_LESS;

		// Mipmap
		samplerInfo.mipmapMode = mipmapMode;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = VK_LOD_CLAMP_NONE;

		if (vkCreateSampler(device_, &samplerInfo, nullptr, &sampler_) != VK_SUCCESS)
		{
			return false;
		}

		return true;
	}

	void VulkanSampler::destroy()
	{
		if (sampler_ != VK_NULL_HANDLE)
		{
			vkDestroySampler(device_, sampler_, nullptr);
			sampler_ = VK_NULL_HANDLE;
		}
	}

} // namespace BinRenderer::Vulkan
