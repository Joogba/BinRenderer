#include "Image2D.h"
#include "Context.h"
#include "Logger.h"
#include "MappedBuffer.h"
#include <algorithm>
#include <ktx.h>
#include <ktxvulkan.h>
#include <stb_image.h>
#include <filesystem>

// STB implementation - define once per library
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

namespace BinRenderer::Vulkan {

    Image2D::Image2D(Context& ctx) : Resource(ctx, Type::Image)
    {
    }

    Image2D::~Image2D()
    {
        cleanup();
    }

    auto Image2D::image() const -> VkImage
    {
        return image_;
    }

    VkImageView Image2D::view() const
    {
        return imageView_;
    }

    VkImageView Image2D::attachmentView() const
    {
        // Return depth-stencil view if available, otherwise regular view
        return (depthStencilView_ != VK_NULL_HANDLE) ? depthStencilView_ : imageView_;
    }

    auto Image2D::width() const -> uint32_t
    {
        return width_;
    }

    auto Image2D::height() const -> uint32_t
    {
        return height_;
    }

    void Image2D::createFromPixelData(unsigned char* pixelData, int width, int height, int channels,
        bool sRGB)
    {
        if (pixelData == nullptr) {
            exitWithMessage("Pixel data must not be nullptr for Image creation.");
        }

        if (channels != 4) {
            exitWithMessage("Unsupported number of channels: {}", std::to_string(channels));
        }

        // Determine format based on sRGB flag
        VkFormat format = sRGB ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;

        // Create Vulkan image
        createImage(format, static_cast<uint32_t>(width), static_cast<uint32_t>(height),
            VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 0, VK_IMAGE_VIEW_TYPE_2D);

        VkDeviceSize uploadSize = width * height * channels * sizeof(unsigned char);

        MappedBuffer stagingBuffer(ctx_);
        stagingBuffer.createStagingBuffer(uploadSize, pixelData);

        // Create command buffer for the copy operation
        CommandBuffer copyCmd = ctx_.createTransferCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

        // Transition image layout to transfer destination optimal
        barrierHelper().transitionTo(copyCmd.handle(), VK_ACCESS_2_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_2_TRANSFER_BIT);

        // Copy data from staging buffer to GPU image
        VkBufferImageCopy bufferCopyRegion = {};
        bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        bufferCopyRegion.imageSubresource.layerCount = 1;
        bufferCopyRegion.imageExtent.width = width;
        bufferCopyRegion.imageExtent.height = height;
        bufferCopyRegion.imageExtent.depth = 1;

        vkCmdCopyBufferToImage(copyCmd.handle(), stagingBuffer.buffer(), image_,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopyRegion);

        barrierHelper().transitionTo(copyCmd.handle(), VK_ACCESS_2_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);

        copyCmd.submitAndWait();
    }

    void Image2D::createSolid(int width, int height, uint8_t rgba[4])
    {
        if (width <= 0 || height <= 0) {
            exitWithMessage("Solid texture dimensions must be greater than zero: {}x{}", width, height);
        }

        // Calculate total pixels and allocate pixel data
        const int totalPixels = width * height;
        const int channels = 4; // RGBA
        const size_t dataSize = totalPixels * channels;

        // Create pixel data array filled with the specified RGBA color
        std::vector<unsigned char> pixelData(dataSize);

        // Fill all pixels with the specified RGBA color
        for (int i = 0; i < totalPixels; ++i) {
            int pixelOffset = i * 4;
            pixelData[pixelOffset + 0] = rgba[0]; // Red
            pixelData[pixelOffset + 1] = rgba[1]; // Green
            pixelData[pixelOffset + 2] = rgba[2]; // Blue
            pixelData[pixelOffset + 3] = rgba[3]; // Alpha
        }

        // Use the existing createFromPixelData method
        // false = not sRGB (solid colors are typically linear)
        createFromPixelData(pixelData.data(), width, height, channels, false);
    }

    std::string fixPath(const std::string& path) // for linux path
    {
        std::string fixed = path;
        std::replace(fixed.begin(), fixed.end(), '\\', '/');
        return fixed;
    }

    void Image2D::createTextureFromKtx2(string filename, bool isCubemap)
    {
        filename = fixPath(filename);

        // Validate file extension
        size_t extensionPos = filename.find_last_of('.');
        if (extensionPos == string::npos || filename.substr(extensionPos) != ".ktx2") {
            exitWithMessage("File extension must be .ktx2 for createTextureFromKtx2: {}", filename);
        }

        // Load KTX2 texture
        ktxTexture2* ktxTexture2;
        ktxResult result = ktxTexture2_CreateFromNamedFile(
            filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture2);

        if (result != KTX_SUCCESS) {
            exitWithMessage("Failed to load KTX2 texture: {}", filename);
        }

        // Get texture properties
        uint32_t mipLevels = ktxTexture2->numLevels;
        uint32_t layerCount = isCubemap ? 6 : 1;

        // Determine format from KTX2 file
        VkFormat vkFormat = ktxTexture2_GetVkFormat(ktxTexture2);
        if (vkFormat == VK_FORMAT_UNDEFINED) {
            vkFormat = isCubemap ? VK_FORMAT_R16G16B16A16_SFLOAT : VK_FORMAT_R16G16_SFLOAT;
        }

        // Get texture data
        ktxTexture* baseTexture = ktxTexture(ktxTexture2);
        ktx_uint8_t* ktxTextureData = ktxTexture_GetData(baseTexture);
        ktx_size_t ktxTextureSize = ktxTexture_GetDataSize(baseTexture);

        // Create staging buffer
        MappedBuffer stagingBuffer(ctx_);
        stagingBuffer.createStagingBuffer(ktxTextureSize, ktxTextureData);

        // Create Vulkan image
        VkImageCreateFlags flags = isCubemap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
        VkImageViewType viewType = isCubemap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;

        createImage(vkFormat, ktxTexture2->baseWidth, ktxTexture2->baseHeight, VK_SAMPLE_COUNT_1_BIT,
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, layerCount, flags, viewType);

        // Create command buffer for copy operations
        CommandBuffer copyCmd = ctx_.createTransferCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

        // Prepare buffer copy regions for all mip levels and array layers
        vector<VkBufferImageCopy> bufferCopyRegions;

        if (isCubemap) {
            // For cubemaps: iterate through faces and mip levels
            for (uint32_t face = 0; face < 6; face++) {
                for (uint32_t level = 0; level < mipLevels; level++) {
                    ktx_size_t offset;
                    KTX_error_code ktxResult =
                        ktxTexture_GetImageOffset(baseTexture, level, 0, face, &offset);
                    if (ktxResult != KTX_SUCCESS) {
                        offset = 0;
                    }

                    VkBufferImageCopy bufferCopyRegion{};
                    bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    bufferCopyRegion.imageSubresource.mipLevel = level;
                    bufferCopyRegion.imageSubresource.baseArrayLayer = face;
                    bufferCopyRegion.imageSubresource.layerCount = 1;
                    bufferCopyRegion.imageExtent.width = max(1u, width_ >> level);
                    bufferCopyRegion.imageExtent.height = max(1u, height_ >> level);
                    bufferCopyRegion.imageExtent.depth = 1;
                    bufferCopyRegion.bufferOffset = offset;

                    bufferCopyRegions.push_back(bufferCopyRegion);
                }
            }
        }
        else {
            // For 2D textures: iterate through mip levels only
            for (uint32_t level = 0; level < mipLevels; level++) {
                ktx_size_t offset;
                KTX_error_code ktxResult = ktxTexture_GetImageOffset(baseTexture, level, 0, 0, &offset);
                if (ktxResult != KTX_SUCCESS) {
                    offset = 0;
                }

                VkBufferImageCopy bufferCopyRegion{};
                bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                bufferCopyRegion.imageSubresource.mipLevel = level;
                bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
                bufferCopyRegion.imageSubresource.layerCount = 1;
                bufferCopyRegion.imageExtent.width = max(1u, width_ >> level);
                bufferCopyRegion.imageExtent.height = max(1u, height_ >> level);
                bufferCopyRegion.imageExtent.depth = 1;
                bufferCopyRegion.bufferOffset = offset;

                bufferCopyRegions.push_back(bufferCopyRegion);
            }
        }

        // Transition image layout to transfer destination optimal
        barrierHelper().transitionTo(copyCmd.handle(), VK_ACCESS_2_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_2_TRANSFER_BIT);

        // Copy buffer to image
        vkCmdCopyBufferToImage(
            copyCmd.handle(), stagingBuffer.buffer(), image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            static_cast<uint32_t>(bufferCopyRegions.size()), bufferCopyRegions.data());

        // Transition image layout to shader read-only optimal
        barrierHelper().transitionTo(copyCmd.handle(), VK_ACCESS_2_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);

        copyCmd.submitAndWait();

        // Clean up KTX2 texture
        ktxTexture_Destroy(ktxTexture(ktxTexture2));
    }

    void Image2D::createTextureFromImage(string filename, bool isCubemap, bool sRGB)
    {
        filename = fixPath(filename);

        // Validate file extension
        size_t extensionPos = filename.find_last_of('.');
        string extension = (extensionPos != string::npos) ? filename.substr(extensionPos) : "";
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        if (extensionPos == string::npos ||
            (extension != ".png" && extension != ".jpg" && extension != ".jpeg")) {
            exitWithMessage("File extension must be .png, .jpg, or .jpeg for createFromImage: {}",
                filename);
        }

        if (isCubemap) {
            exitWithMessage("PNG/JPEG format does not support cubemaps: {}", filename);
        }

        // Load image using stb_image
        int width, height, channels;
        unsigned char* pixelData =
            stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);

        if (!pixelData) {
            exitWithMessage("Failed to load image texture: {} ({})", filename,
                string(stbi_failure_reason()));
        }

        createFromPixelData(pixelData, width, height, 4, sRGB);
        stbi_image_free(pixelData);
    }

    void Image2D::createRGBA32F(uint32_t width, uint32_t height)
    {
        createImage(VK_FORMAT_R32G32B32A32_SFLOAT, width, height, VK_SAMPLE_COUNT_1_BIT,
            VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 0, VK_IMAGE_VIEW_TYPE_2D);
    }

    void Image2D::createRGBA16F(uint16_t width, uint32_t height)
    {
        createImage(VK_FORMAT_R16G16B16A16_SFLOAT, width, height, VK_SAMPLE_COUNT_1_BIT,
            VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 0, VK_IMAGE_VIEW_TYPE_2D);
    }

    void Image2D::createGeneralStorage(uint16_t width, uint32_t height)
    {
        VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
            VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
            VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        createImage(VK_FORMAT_R16G16B16A16_SFLOAT, static_cast<uint32_t>(width),
            static_cast<uint32_t>(height), VK_SAMPLE_COUNT_1_BIT, usage,
            VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 0, VK_IMAGE_VIEW_TYPE_2D);
    }

    void Image2D::createShadow(uint32_t width, uint32_t height)
    {
        // Create shadow map with appropriate format and usage flags for depth testing and sampling
        createImage(VK_FORMAT_D16_UNORM,   // 16-bit depth format, suitable for shadow maps
            width,                 // Width
            height,                // Height
            VK_SAMPLE_COUNT_1_BIT, // No MSAA for shadow maps
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | // Can be used as depth attachment
            VK_IMAGE_USAGE_SAMPLED_BIT |              // Can be sampled in shaders
            VK_IMAGE_USAGE_TRANSFER_DST_BIT |
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT, // Can be copied to/from
            VK_IMAGE_ASPECT_DEPTH_BIT,           // Depth aspect only
            1,                                   // Single mip level
            1,                                   // Single array layer
            0,                                   // No special flags
            VK_IMAGE_VIEW_TYPE_2D                // 2D image view
        );
    }

    void Image2D::createImage(VkFormat format, uint32_t width, uint32_t height,
        VkSampleCountFlagBits sampleCount, VkImageUsageFlags usage,
        VkImageAspectFlags aspectMask, uint32_t mipLevels, uint32_t arrayLayers,
        VkImageCreateFlags flags, VkImageViewType viewType)
    {
        if (width == 0 || height == 0) {
            exitWithMessage("Image dimensions must be greater than zero");
        }

        cleanup();

        format_ = format;
        width_ = width;
        height_ = height;
        usageFlags_ |= usage;

        // Create image
        VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = format_;
        imageInfo.extent.width = width_;
        imageInfo.extent.height = height_;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = arrayLayers;
        imageInfo.samples = sampleCount;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = usageFlags_;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.flags = flags;

        check(vkCreateImage(ctx_.device(), &imageInfo, nullptr, &image_));

        // Allocate memory
        VkMemoryRequirements memReqs;
        vkGetImageMemoryRequirements(ctx_.device(), image_, &memReqs);

        VkMemoryAllocateInfo memAllocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        memAllocInfo.allocationSize = memReqs.size;
        memAllocInfo.memoryTypeIndex =
            ctx_.getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        check(vkAllocateMemory(ctx_.device(), &memAllocInfo, nullptr, &memory_));
        check(vkBindImageMemory(ctx_.device(), image_, memory_, 0));

        // Create image view
        VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        viewInfo.image = image_;
        viewInfo.viewType = viewType;
        viewInfo.format = format_;
        viewInfo.subresourceRange.aspectMask = aspectMask;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = arrayLayers;

        check(vkCreateImageView(ctx_.device(), &viewInfo, nullptr, &imageView_));

        // Initialize the resource with image data
        resourceBinding().image_ = image_;
        resourceBinding().imageView_ = imageView_;
        resourceBinding().descriptorCount_ = 1;
        initializeImageResource(image_, format_, mipLevels, arrayLayers);
        updateResourceBinding();
    }

    void Image2D::cleanup()
    {
        if (depthStencilView_ != VK_NULL_HANDLE) {
            vkDestroyImageView(ctx_.device(), depthStencilView_, nullptr);
            depthStencilView_ = VK_NULL_HANDLE;
        }
        if (imageView_ != VK_NULL_HANDLE) {
            vkDestroyImageView(ctx_.device(), imageView_, nullptr);
            imageView_ = VK_NULL_HANDLE;
        }
        if (image_ != VK_NULL_HANDLE) {
            // printLog("Cleaning image {:#016x}", reinterpret_cast<uintptr_t>(image_));
            vkDestroyImage(ctx_.device(), image_, nullptr);
            image_ = VK_NULL_HANDLE;
        }
        if (memory_ != VK_NULL_HANDLE) {
            vkFreeMemory(ctx_.device(), memory_, nullptr);
            memory_ = VK_NULL_HANDLE;
        }

        format_ = VK_FORMAT_UNDEFINED;
        width_ = 0;
        height_ = 0;
    }

    void Image2D::createDepthBuffer(uint32_t width, uint32_t height)
    {
        // Create depth buffer with appropriate format and usage flags for depth-stencil operations
        createImage(
            ctx_.depthFormat(),                           // Use context's preferred depth format
            width,                                        // Width
            height,                                       // Height
            VK_SAMPLE_COUNT_1_BIT,                        // Always use 1x samples (no MSAA)
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | // Can be used as depth-stencil attachment
            VK_IMAGE_USAGE_SAMPLED_BIT, // Can be sampled in shaders (depth-only aspect)
            VK_IMAGE_ASPECT_DEPTH_BIT,      // Start with depth-only aspect for the primary view
            1,                              // Single mip level
            1,                              // Single array layer
            0,                              // No special flags
            VK_IMAGE_VIEW_TYPE_2D           // 2D image view
        );

        // Create additional depth-stencil view for attachment usage
        createDepthStencilAttachmentView();
    }

    void Image2D::updateResourceBindingAfterTransition()
    {
        VkImageLayout currentLayout = barrierHelper().currentLayout();

        if (currentLayout == VK_IMAGE_LAYOUT_GENERAL) {
            // General layout is used for storage images
            resourceBinding().descriptorType_ = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            resourceBinding().imageInfo_.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        }
        else if (currentLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            // Shader read-only layout is used for sampled images
            if (resourceBinding().sampler_ != VK_NULL_HANDLE) {
                resourceBinding().descriptorType_ = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            }
            else {
                resourceBinding().descriptorType_ = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            }
            resourceBinding().imageInfo_.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
        else if (currentLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ||
            currentLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            // Attachment layouts are typically used for input attachments when used in descriptors
            resourceBinding().descriptorType_ = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            resourceBinding().imageInfo_.imageLayout = currentLayout;
        }
        else {
            // For other layouts, default to storage image with general layout capability
            resourceBinding().descriptorType_ = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            resourceBinding().imageInfo_.imageLayout = currentLayout;
        }

        // Update the image info
        resourceBinding().imageInfo_.imageView = imageView_;
        resourceBinding().imageInfo_.sampler = resourceBinding().sampler_;
    }

    void Image2D::createDepthStencilAttachmentView()
    {
        // Only create if we don't already have one and this is a depth format
        if (depthStencilView_ != VK_NULL_HANDLE) {
            vkDestroyImageView(ctx_.device(), depthStencilView_, nullptr);
            depthStencilView_ = VK_NULL_HANDLE;
        }

        // Only create depth-stencil view for depth formats
        if (format_ < VK_FORMAT_D16_UNORM || format_ > VK_FORMAT_D32_SFLOAT_S8_UINT) {
            return; // Not a depth format
        }

        VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        viewInfo.image = image_;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format_;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        // Include stencil aspect if format supports it
        if (format_ >= VK_FORMAT_D16_UNORM_S8_UINT) {
            viewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }

        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        check(vkCreateImageView(ctx_.device(), &viewInfo, nullptr, &depthStencilView_));
    }

    auto Image2D::format() const -> VkFormat
    {
        return format_;
    }

    // void Image2D::updateBinding(VkDescriptorSetLayoutBinding& binding)
    //{
    //     const ResourceBinding& rb = resourceBinding();
    //
    //     // Set descriptor type based on current usage and state
    //     binding.descriptorType = rb.descriptorType_;
    //     binding.descriptorCount = rb.descriptorCount_;
    //     binding.pImmutableSamplers = nullptr;
    //     binding.stageFlags = 0; // Will be set by shader reflection
    // }

    void Image2D::updateWrite(VkDescriptorSetLayoutBinding expectedBinding, VkWriteDescriptorSet& write)
    {
        const ResourceBinding& rb = resourceBinding();

        imageInfo_ = rb.imageInfo_;

        if (expectedBinding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
            if (!(this->usageFlags_ & VK_IMAGE_USAGE_STORAGE_BIT)) {
                exitWithMessage("Image2D was not created with VK_IMAGE_USAGE_STORAGE_BIT flag for "
                    "VK_DESCRIPTOR_TYPE_STORAGE_IMAGE descriptorType.");
            }
            imageInfo_.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        }

        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.pNext = nullptr;
        write.dstSet = VK_NULL_HANDLE; // Will be set by DescriptorSet::create()
        write.dstBinding = 0;          // Will be set by DescriptorSet::create()
        write.dstArrayElement = 0;
        write.descriptorType = expectedBinding.descriptorType;
        write.descriptorCount = expectedBinding.descriptorCount;
        write.pImageInfo = &imageInfo_;
        write.pBufferInfo = nullptr;
        write.pTexelBufferView = nullptr;
    }

} // namespace BinRenderer::Vulkan