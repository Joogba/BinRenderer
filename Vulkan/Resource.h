#pragma once

#include "BarrierHelper.h"
#include "ResourceBinding.h"
#include "Context.h"
#include "Logger.h"
#include <vulkan/vulkan.h>

namespace BinRenderer::Vulkan {

class Resource
{
  public:
    enum class Type { Image, Buffer };

    Resource(Context& ctx, Type type);
    virtual ~Resource() = default;

    // Deleted copy operations to enforce move-only semantics
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;

    // Move operations
    Resource(Resource&& other) noexcept;
    Resource& operator=(Resource&& other) noexcept;

    // Common interface
    virtual void cleanup() = 0;

    // Pure virtual methods that must be implemented by derived classes
    // virtual void updateBinding(VkDescriptorSetLayoutBinding& binding) = 0;
    virtual void updateWrite(VkDescriptorSetLayoutBinding expectedBinding,
                             VkWriteDescriptorSet& write) = 0;

    // Resource identification
    Type getType() const
    {
        return type_;
    }
    bool isImage() const
    {
        return type_ == Type::Image;
    }
    bool isBuffer() const
    {
        return type_ == Type::Buffer;
    }

    // Common resource management
    BarrierHelper& barrierHelper()
    {
        return barrierHelper_;
    }
    const BarrierHelper& barrierHelper() const
    {
        return barrierHelper_;
    }

    ResourceBinding& resourceBinding()
    {
        return resourceBinding_;
    }
    const ResourceBinding& resourceBinding() const
    {
        return resourceBinding_;
    }

    // Image-specific methods (only valid for Image resources)
    void transitionTo(VkCommandBuffer cmd, VkAccessFlags2 newAccess, VkImageLayout newLayout,
                      VkPipelineStageFlags2 newStage);
    void transitionToColorAttachment(VkCommandBuffer cmd);
    void transitionToTransferSrc(VkCommandBuffer cmd);
    void transitionToTransferDst(VkCommandBuffer cmd);
    void transitionToShaderRead(VkCommandBuffer cmd);
    void transitionToDepthStencilAttachment(VkCommandBuffer cmd);
    void transitionToGeneral(VkCommandBuffer cmd, VkAccessFlags2 accessFlags,
                             VkPipelineStageFlags2 stageFlags);
    void setSampler(VkSampler sampler);

    // Buffer-specific methods (only valid for Buffer resources)
    void transitionBuffer(VkCommandBuffer cmd, VkAccessFlags2 srcAccess, VkAccessFlags2 dstAccess,
                          VkPipelineStageFlags2 srcStage, VkPipelineStageFlags2 dstStage);

  protected:
    Context& ctx_;
    Type type_;
    BarrierHelper barrierHelper_;
    ResourceBinding resourceBinding_;

    // Helper methods for derived classes
    void initializeImageResource(VkImage image, VkFormat format, uint32_t mipLevels,
                                 uint32_t arrayLayers);
    void initializeBufferResource(VkBuffer buffer, VkDeviceSize size);
    void updateResourceBinding();

  private:
    // Implementation helpers
    void assertImageType() const;
    void assertBufferType() const;
};

} // namespace BinRenderer::Vulkan