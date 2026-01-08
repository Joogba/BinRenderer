#include "VulkanRHI.h"
#include "Resources/VulkanBuffer.h"
#include "Resources/VulkanImage.h"
#include "Resources/VulkanSampler.h"
#include "Resources/VulkanTexture.h"
#include "Conversion/TypeConversion.h"
#include "Core/Logger.h"

namespace BinRenderer::Vulkan
{
	RHIBufferHandle VulkanRHI::createBuffer(const RHIBufferCreateInfo& createInfo)
	{
		auto* vulkanBuffer = new VulkanBuffer(context_->getDevice(), context_->getPhysicalDevice());
		if (!vulkanBuffer->create(createInfo))
		{
			delete vulkanBuffer;
			return {};
		}
		return bufferPool.insert(vulkanBuffer);
	}

	RHIImageHandle VulkanRHI::createImage(const RHIImageCreateInfo& createInfo)
	{
		auto* vulkanImage = new VulkanImage(context_->getDevice(), context_->getPhysicalDevice());
		if (!vulkanImage->create(createInfo))
		{
			delete vulkanImage;
			return {};
		}
		return imagePool.insert(vulkanImage);
	}

	RHIImageViewHandle VulkanRHI::createImageView(RHIImageHandle imageHandle, const RHIImageViewCreateInfo& createInfo)
	{
		RHIImage* image = imagePool.get(imageHandle);
		if (!image)
		{
			return {};
		}

		auto* vulkanImage = static_cast<VulkanImage*>(image);
		auto* imageView = new VulkanImageView(context_->getDevice(), vulkanImage);
  
		// createInfo를 Vulkan 타입으로 변환 (TypeConversion 이용)
		VkImageViewType viewType = TypeConversion::toVkImageViewType(createInfo.viewType);
		VkImageAspectFlags aspectFlags = TypeConversion::toVkImageAspectFlags(createInfo.aspectMask);

		if (!imageView->create(viewType, aspectFlags))
		{
			delete imageView;
			return {};
		}
    
		return imageViewPool.insert(imageView);
	}

	RHISamplerHandle VulkanRHI::createSampler(const RHISamplerCreateInfo& createInfo)
	{
		auto* sampler = new VulkanSampler(context_->getDevice());
		
		// TODO: createInfo 파라미터 사용하여 Sampler 생성
		// 현재는 기본 Linear 설정
		if (!sampler->createLinear())
		{
			delete sampler;
			return {};
		}
	 
		return samplerPool.insert(sampler);
	}

	RHITextureHandle VulkanRHI::createTexture(RHIImageHandle imageHandle, RHIImageViewHandle viewHandle, RHISamplerHandle samplerHandle)
	{
		RHIImage* image = imagePool.get(imageHandle);
		if (!image) return {};

		// 이미지 정보 가져오기
		uint32_t width = image->getWidth();
		uint32_t height = image->getHeight();
		uint32_t mipLevels = image->getMipLevels();

		auto* texture = new VulkanTexture(imageHandle, viewHandle, samplerHandle, width, height, mipLevels);
		return texturePool.insert(texture);
	}

	void VulkanRHI::destroyBuffer(RHIBufferHandle buffer) { bufferPool.remove(buffer); }
	void VulkanRHI::destroyImage(RHIImageHandle image) { imagePool.remove(image); }
	void VulkanRHI::destroyImageView(RHIImageViewHandle imageView) { imageViewPool.remove(imageView); }
	void VulkanRHI::destroySampler(RHISamplerHandle sampler) { samplerPool.remove(sampler); }
	void VulkanRHI::destroyTexture(RHITextureHandle texture) { texturePool.remove(texture); }

	void* VulkanRHI::mapBuffer(RHIBufferHandle bufferHandle)
	{
		RHIBuffer* buffer = bufferPool.get(bufferHandle);
		if (!buffer)
		{
			return nullptr;
		}

		auto* vulkanBuffer = static_cast<VulkanBuffer*>(buffer);
		return vulkanBuffer->map();
	}

	void VulkanRHI::unmapBuffer(RHIBufferHandle bufferHandle)
	{
		RHIBuffer* buffer = bufferPool.get(bufferHandle);
		if (!buffer)
		{
			return;
		}

		auto* vulkanBuffer = static_cast<VulkanBuffer*>(buffer);
		vulkanBuffer->unmap();
	}

	void VulkanRHI::flushBuffer(RHIBufferHandle bufferHandle, RHIDeviceSize offset, RHIDeviceSize size)
	{
		RHIBuffer* buffer = bufferPool.get(bufferHandle);
		if (!buffer)
		{
			return;
		}

		auto* vulkanBuffer = static_cast<VulkanBuffer*>(buffer);
		vulkanBuffer->flush(offset, size);
	}

} // namespace BinRenderer::Vulkan
