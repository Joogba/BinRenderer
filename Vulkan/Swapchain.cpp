#include "Swapchain.h"

namespace BinRenderer::Vulkan {

using namespace std;

Swapchain::Swapchain(Context& ctx, VkSurfaceKHR surface, VkExtent2D& windowSize, bool vsync)
    : ctx_(ctx)
{
    initSurface(surface);
    create(windowSize, vsync);
}

void Swapchain::initSurface(VkSurfaceKHR surface)
{
    constexpr uint32_t INVALID_INDEX = UINT32_MAX;

    surface_ = surface;

    // Use Context's existing queue family properties instead of re-querying
    const auto& queueFamilyProps = ctx_.queueFamilyProperties();
    uint32_t queueCount = static_cast<uint32_t>(queueFamilyProps.size());

    // Check surface support for each queue family
    vector<VkBool32> supportsPresent(queueCount);
    for (uint32_t i = 0; i < queueCount; i++) {
        vkGetPhysicalDeviceSurfaceSupportKHR(ctx_.physicalDevice(), i, surface_,
                                             &supportsPresent[i]);
    }

    // Find graphics and present queue families
    // Start with the graphics queue family from Context
    uint32_t graphicsQueueIndex = ctx_.queueFamilyIndices().graphics;
    uint32_t presentQueueIndex = INVALID_INDEX;

    // Check if the graphics queue supports presentation
    if (supportsPresent[graphicsQueueIndex] == VK_TRUE) {
        presentQueueIndex = graphicsQueueIndex;
    } else {
        // Find any queue that supports presentation
        for (uint32_t i = 0; i < queueCount; ++i) {
            if (supportsPresent[i] == VK_TRUE) {
                presentQueueIndex = i;
                break;
            }
        }
    }

    // Validate queue families found
    if (graphicsQueueIndex == INVALID_INDEX || presentQueueIndex == INVALID_INDEX) {
        exitWithMessage("Could not find a graphics and/or presenting queue!");
    }

    if (graphicsQueueIndex != presentQueueIndex) {
        exitWithMessage("Separate graphics and presenting queues are not supported yet!");
    }

    // Surface format selection
    uint32_t formatCount;
    check(
        vkGetPhysicalDeviceSurfaceFormatsKHR(ctx_.physicalDevice(), surface_, &formatCount, NULL));
    assert(formatCount > 0);

    vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    check(vkGetPhysicalDeviceSurfaceFormatsKHR(ctx_.physicalDevice(), surface_, &formatCount,
                                               surfaceFormats.data()));

    vector<VkFormat> preferredImageFormats = {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SRGB,
                                              VK_FORMAT_B8G8R8A8_UNORM,
                                              VK_FORMAT_B8G8R8A8_SRGB, // for linux
                                              VK_FORMAT_A8B8G8R8_UNORM_PACK32};

    VkSurfaceFormatKHR selectedFormat;
    selectedFormat.format = VK_FORMAT_UNDEFINED;
    for (auto& preferredFormat : preferredImageFormats) {
        for (auto& availableFormat : surfaceFormats) {
            if (availableFormat.format == preferredFormat) {
                selectedFormat = availableFormat;
                break;
            }
        }
        if (selectedFormat.format != VK_FORMAT_UNDEFINED) {
            break;
        }
    }

    if (selectedFormat.format == VK_FORMAT_UNDEFINED)
        exitWithMessage(
            "No preferred swapchain image format found! Please check your GPU and driver support.");

    // Print selected format and color space
    printLog("Selected Surface Format: {}", vkFormatToString(selectedFormat.format));
    printLog("Selected Color Space: VK_COLOR_SPACE_{}",
             colorSpaceToString(selectedFormat.colorSpace));

    colorFormat_ = selectedFormat.format;
    colorSpace_ = selectedFormat.colorSpace;
}

const char* presentModeToString(VkPresentModeKHR mode)
{
    switch (mode) {
    case VK_PRESENT_MODE_IMMEDIATE_KHR:
        return "IMMEDIATE_KHR";
    case VK_PRESENT_MODE_MAILBOX_KHR:
        return "MAILBOX_KHR";
    case VK_PRESENT_MODE_FIFO_KHR:
        return "FIFO_KHR";
    case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
        return "FIFO_RELAXED_KHR";
    case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
        return "SHARED_DEMAND_REFRESH_KHR";
    case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
        return "SHARED_CONTINUOUS_REFRESH_KHR";
    default:
        return "UNKNOWN";
    }
}

void Swapchain::create(VkExtent2D& expectedWindowSize, bool vsync)
{
    VkSwapchainKHR oldSwapchain = swapchain_;

    VkSurfaceCapabilitiesKHR surfCaps;
    check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx_.physicalDevice(), surface_, &surfCaps));

    // Check for storage bit support
    bool storageSupported = (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT) != 0;
    printLog("Swapchain Storage Bit Support: {}", storageSupported ? "YES" : "NO");

    VkExtent2D swapchainExtent = {};

    if (surfCaps.currentExtent.width == (uint32_t)-1) {
        swapchainExtent.width = expectedWindowSize.width;
        swapchainExtent.height = expectedWindowSize.height;
    } else {
        swapchainExtent = surfCaps.currentExtent;
        expectedWindowSize.width = surfCaps.currentExtent.width;
        expectedWindowSize.height = surfCaps.currentExtent.height;
    }

    uint32_t presentModeCount;
    check(vkGetPhysicalDeviceSurfacePresentModesKHR(ctx_.physicalDevice(), surface_,
                                                    &presentModeCount, nullptr));
    assert(presentModeCount > 0);

    vector<VkPresentModeKHR> presentModes(presentModeCount);
    check(vkGetPhysicalDeviceSurfacePresentModesKHR(ctx_.physicalDevice(), surface_,
                                                    &presentModeCount, presentModes.data()));

    printLog("Available Present Modes: {}", presentModeCount);
    for (const auto& mode : presentModes) {
        printLog("  VK_PRESENT_MODE_{}", presentModeToString(mode));
    }

    VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

    if (vsync) {
        // When vsync is enabled, prioritize MAILBOX over FIFO for smoother experience
        for (size_t i = 0; i < presentModeCount; i++) {
            if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
                swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }
        }
        // If MAILBOX is not available, FIFO is already set as default
    } else {
        // When vsync is disabled, prioritize non-sync modes for maximum performance
        for (size_t i = 0; i < presentModeCount; i++) {
            if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
                break; // IMMEDIATE is the true non-vsync mode, use it first
            }
            if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
                swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                // Fallback: MAILBOX is still better than FIFO for performance
            }
        }
    }

    printLog("Selected Present Mode: VK_PRESENT_MODE_{}",
             presentModeToString(swapchainPresentMode));

    uint32_t desiredNumberOfSwapchainImages = surfCaps.minImageCount + 1;
    if ((surfCaps.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfCaps.maxImageCount)) {
        desiredNumberOfSwapchainImages = surfCaps.maxImageCount;
    }
    printLog("Designed Num of Swapchain Images: {}", desiredNumberOfSwapchainImages);

    VkSurfaceTransformFlagsKHR preTransform;
    if (surfCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        preTransform = surfCaps.currentTransform;
    }

    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };
    for (auto& compositeAlphaFlag : compositeAlphaFlags) {
        if (surfCaps.supportedCompositeAlpha & compositeAlphaFlag) {
            compositeAlpha = compositeAlphaFlag;
            break;
        }
    }

    VkSwapchainCreateInfoKHR swapchainCI = {};
    swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCI.surface = surface_;
    swapchainCI.minImageCount = desiredNumberOfSwapchainImages;
    swapchainCI.imageFormat = colorFormat_;
    swapchainCI.imageColorSpace = colorSpace_;
    swapchainCI.imageExtent = swapchainExtent;
    swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCI.preTransform = static_cast<VkSurfaceTransformFlagBitsKHR>(preTransform);
    swapchainCI.imageArrayLayers = 1;
    swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCI.queueFamilyIndexCount = 0;
    swapchainCI.presentMode = swapchainPresentMode;

    swapchainCI.oldSwapchain = oldSwapchain;

    swapchainCI.clipped = VK_TRUE;
    swapchainCI.compositeAlpha = compositeAlpha;

    if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
        swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    // ADD STORAGE BIT IF SUPPORTED (Compute shader에서 직접 swapchain으로 출력할때 사용)
    if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT) {
        swapchainCI.imageUsage |= VK_IMAGE_USAGE_STORAGE_BIT;
        printLog("Added VK_IMAGE_USAGE_STORAGE_BIT to swapchain");
    } else {
        printLog("VK_IMAGE_USAGE_STORAGE_BIT not supported for swapchain");
    }

    check(vkCreateSwapchainKHR(ctx_.device(), &swapchainCI, nullptr, &swapchain_));

    if (oldSwapchain != VK_NULL_HANDLE) {
        for (uint32_t i = 0; i < images_.size(); i++) {
            vkDestroyImageView(ctx_.device(), imageViews_[i], nullptr);
        }
        vkDestroySwapchainKHR(ctx_.device(), oldSwapchain, nullptr);
    }
    check(vkGetSwapchainImagesKHR(ctx_.device(), swapchain_, &imageCount_, nullptr));

    images_.resize(imageCount_);
    check(vkGetSwapchainImagesKHR(ctx_.device(), swapchain_, &imageCount_, images_.data()));

    imageViews_.resize(imageCount_);
    for (uint32_t i = 0; i < images_.size(); i++) {
        VkImageViewCreateInfo colorAttachmentView = {};
        colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        colorAttachmentView.pNext = nullptr;
        colorAttachmentView.format = colorFormat_;
        colorAttachmentView.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
                                          VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
        colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        colorAttachmentView.subresourceRange.baseMipLevel = 0;
        colorAttachmentView.subresourceRange.levelCount = 1;
        colorAttachmentView.subresourceRange.baseArrayLayer = 0;
        colorAttachmentView.subresourceRange.layerCount = 1;
        colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
        colorAttachmentView.flags = 0;
        colorAttachmentView.image = images_[i];
        check(vkCreateImageView(ctx_.device(), &colorAttachmentView, nullptr, &imageViews_[i]));
    }

    barrierHelpers.reserve(imageCount_);
    for (uint32_t i = 0; i < imageCount_; i++) {
        barrierHelpers.emplace_back(images_[i]);
        barrierHelpers[i].format() = colorFormat_;
        barrierHelpers[i].mipLevels() = 1;   // Swapchain images have 1 mip level
        barrierHelpers[i].arrayLayers() = 1; // Swapchain images have 1 array layer
        barrierHelpers[i].currentLayout() = VK_IMAGE_LAYOUT_UNDEFINED;
        barrierHelpers[i].currentAccess() = VK_ACCESS_2_NONE;
        barrierHelpers[i].currentStage() = VK_PIPELINE_STAGE_2_NONE;
    }
}

VkResult Swapchain::acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t& imageIndex)
{
    return vkAcquireNextImageKHR(ctx_.device(), swapchain_, UINT64_MAX, presentCompleteSemaphore,
                                 (VkFence) nullptr, &imageIndex);
}

VkResult Swapchain::queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore)
{
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = NULL;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain_;
    presentInfo.pImageIndices = &imageIndex;

    if (waitSemaphore != VK_NULL_HANDLE) {
        presentInfo.pWaitSemaphores = &waitSemaphore;
        presentInfo.waitSemaphoreCount = 1;
    }
    return vkQueuePresentKHR(queue, &presentInfo);
}

void Swapchain::cleanup()
{
    // Wait for device to be idle before cleanup
    if (ctx_.device() != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(ctx_.device());
    }

    // Clean up image views first (they depend on images)
    for (auto& imageView : imageViews_) {
        if (imageView != VK_NULL_HANDLE) {
            vkDestroyImageView(ctx_.device(), imageView, nullptr);
        }
    }
    imageViews_.clear();

    // Clean up swapchain (this will also clean up the images)
    if (swapchain_ != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(ctx_.device(), swapchain_, nullptr);
        swapchain_ = VK_NULL_HANDLE;
    }

    // Clear image handles (they are owned by swapchain, so just clear the vector)
    images_.clear();

    // Clear barrier helpers
    barrierHelpers.clear();

    // Clean up surface (if owned by this swapchain)
    if (surface_ != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(ctx_.instance(), surface_, nullptr);
        surface_ = VK_NULL_HANDLE;
    }

    // Reset member variables to default state
    colorFormat_ = VK_FORMAT_UNDEFINED;
    colorSpace_ = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    imageCount_ = 0;
}

} // namespace BinRenderer::Vulkan