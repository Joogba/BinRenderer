#pragma once

#include "../../Resources/RHISampler.h"
#include <vulkan/vulkan.h>

namespace BinRenderer::Vulkan
{
	/**
	 * @brief Vulkan 샘플러 구현
	 */
	class VulkanSampler : public RHISampler
	{
	public:
		VulkanSampler(VkDevice device);
		~VulkanSampler() override;

		// 샘플러 생성 메서드들
		bool createLinear(VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
		bool createNearest(VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
		bool createAnisotropic(float maxAnisotropy = 16.0f, VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
		bool createShadow();

		// 커스텀 생성
		bool create(VkFilter minFilter, VkFilter magFilter, VkSamplerMipmapMode mipmapMode,
			VkSamplerAddressMode addressModeU, VkSamplerAddressMode addressModeV, VkSamplerAddressMode addressModeW,
			float maxAnisotropy = 1.0f, bool compareEnable = false);

		void destroy();

		// RHISampler 인터페이스 구현
		RHIFilter getMinFilter() const override { return minFilter_; }
		RHIFilter getMagFilter() const override { return magFilter_; }
		RHISamplerMipmapMode getMipmapMode() const override { return mipmapMode_; }
		RHISamplerAddressMode getAddressModeU() const override { return addressModeU_; }
		RHISamplerAddressMode getAddressModeV() const override { return addressModeV_; }
		RHISamplerAddressMode getAddressModeW() const override { return addressModeW_; }

		// Vulkan 네이티브 접근
		VkSampler getVkSampler() const { return sampler_; }

	private:
		VkDevice device_;
		VkSampler sampler_ = VK_NULL_HANDLE;

		RHIFilter minFilter_ = RHI_FILTER_LINEAR;
		RHIFilter magFilter_ = RHI_FILTER_LINEAR;
		RHISamplerMipmapMode mipmapMode_ = RHI_SAMPLER_MIPMAP_MODE_LINEAR;
		RHISamplerAddressMode addressModeU_ = RHI_SAMPLER_ADDRESS_MODE_REPEAT;
		RHISamplerAddressMode addressModeV_ = RHI_SAMPLER_ADDRESS_MODE_REPEAT;
		RHISamplerAddressMode addressModeW_ = RHI_SAMPLER_ADDRESS_MODE_REPEAT;
	};

} // namespace BinRenderer::Vulkan
