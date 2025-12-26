#include "VulkanImage.h"
#include "Core/Logger.h"

namespace BinRenderer::Vulkan
{
	// ========================================
	// VulkanImage 구현
	// ========================================

	VulkanImage::VulkanImage(VkDevice device, VkPhysicalDevice physicalDevice)
		: device_(device), physicalDevice_(physicalDevice)
	{
	}

	VulkanImage::~VulkanImage()
	{
		destroy();
	}

	bool VulkanImage::create(const RHIImageCreateInfo& createInfo)
	{
		width_ = createInfo.width;
		height_ = createInfo.height;
		depth_ = createInfo.depth;
		mipLevels_ = createInfo.mipLevels;
		arrayLayers_ = createInfo.arrayLayers;
		format_ = createInfo.format;
		samples_ = createInfo.samples;

		// Vulkan 이미지 생성 정보
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = (depth_ > 1) ? VK_IMAGE_TYPE_3D :
			(height_ > 1) ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_1D;
		imageInfo.extent.width = width_;
		imageInfo.extent.height = height_;
		imageInfo.extent.depth = depth_;
		imageInfo.mipLevels = mipLevels_;
		imageInfo.arrayLayers = arrayLayers_;
		imageInfo.format = static_cast<VkFormat>(format_);
		imageInfo.tiling = static_cast<VkImageTiling>(createInfo.tiling);
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = static_cast<VkImageUsageFlags>(createInfo.usage);
		imageInfo.samples = static_cast<VkSampleCountFlagBits>(samples_);
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.flags = static_cast<VkImageCreateFlags>(createInfo.flags);

		if (vkCreateImage(device_, &imageInfo, nullptr, &image_) != VK_SUCCESS)
		{
			return false;
		}

		// 메모리 요구사항 가져오기
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device_, image_, &memRequirements);

		// 메모리 할당
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(
			memRequirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		if (vkAllocateMemory(device_, &allocInfo, nullptr, &memory_) != VK_SUCCESS)
		{
			vkDestroyImage(device_, image_, nullptr);
			image_ = VK_NULL_HANDLE;
			return false;
		}

		// 이미지에 메모리 바인딩
		vkBindImageMemory(device_, image_, memory_, 0);

		return true;
	}

	void VulkanImage::destroy()
	{
		if (image_ != VK_NULL_HANDLE)
		{
			vkDestroyImage(device_, image_, nullptr);
			image_ = VK_NULL_HANDLE;
		}

		if (memory_ != VK_NULL_HANDLE)
		{
			vkFreeMemory(device_, memory_, nullptr);
			memory_ = VK_NULL_HANDLE;
		}
	}

	uint32_t VulkanImage::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) &&
				(memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		exitWithMessage("Failed to find suitable memory type for image!");
		return 0;
	}

	// ========================================
 // VulkanImageView 구현
	// ========================================

	VulkanImageView::VulkanImageView(VkDevice device, VulkanImage* image)
		: device_(device), image_(image)
	{
	}

	VulkanImageView::~VulkanImageView()
	{
		destroy();
	}

	bool VulkanImageView::create(VkImageViewType viewType, VkImageAspectFlags aspectFlags)
	{
		//  Swapchain image view는 이미 setVkImageView()로 설정되어 있음
		if (imageView_ != VK_NULL_HANDLE)
		{
			//  RHI enum과 Vulkan enum은 값이 일치하므로 직접 캐스팅 안전
			viewType_ = static_cast<RHIImageViewType>(viewType);
			return true;  // 이미 생성됨
		}
		
		//  image_가 nullptr이면 생성 불가
		if (!image_)
		{
			printLog("❌ ERROR: Cannot create VulkanImageView without VulkanImage");
			return false;
		}
		
		//  RHI enum과 Vulkan enum은 값이 일치하므로 직접 캐스팅 안전
		viewType_ = static_cast<RHIImageViewType>(viewType);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image_->getVkImage();
		viewInfo.viewType = viewType;
		viewInfo.format = static_cast<VkFormat>(image_->getFormat());
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = image_->getMipLevels();
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = image_->getArrayLayers();

		if (vkCreateImageView(device_, &viewInfo, nullptr, &imageView_) != VK_SUCCESS)
		{
			printLog("❌ ERROR: Failed to create VkImageView (format={}, viewType={})", 
				static_cast<int>(viewInfo.format), static_cast<int>(viewType));
			return false;
		}

		return true;
	}

	void VulkanImageView::destroy()
	{
		//  소유권이 있을 때만 파괴
		if (imageView_ != VK_NULL_HANDLE && ownsImageView_)
		{
			vkDestroyImageView(device_, imageView_, nullptr);
			imageView_ = VK_NULL_HANDLE;
		}
	}

	RHIFormat VulkanImageView::getFormat() const
	{
		//  Swapchain image view인 경우
		if (swapchainFormat_ != RHI_FORMAT_UNDEFINED)
		{
			return swapchainFormat_;
		}
		
		//  일반 image view인 경우
		return image_ ? image_->getFormat() : RHI_FORMAT_UNDEFINED;
	}

} // namespace BinRenderer::Vulkan
