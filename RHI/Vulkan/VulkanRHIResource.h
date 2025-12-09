#pragma once

#include "RHI/Core/RHI.h"

#include <vulkan/vulkan.h>
#include <optional>

namespace BinRenderer::Vulkan
{
    // ========================================
    // Buffer
    // ========================================
    
    class VulkanBuffer : public RHIBuffer
    {
    public:
        void setResource(VkBuffer res)
        {
            m_resource = res;
        }
        VkBuffer getResource() const
        {
            return m_resource;
        }

    private:
        VkBuffer m_resource = VK_NULL_HANDLE;
    };

    // ========================================
    // BufferView
    // ========================================
    
    class VulkanBufferView : public RHIBufferView
    {
    public:
        void setResource(VkBufferView res)
        {
            m_resource = res;
        }
        VkBufferView getResource() const
        {
            return m_resource;
        }

    private:
        VkBufferView m_resource = VK_NULL_HANDLE;
    };

    // ========================================
    // CommandBuffer
    // ========================================
    
    class VulkanCommandBuffer : public RHICommandBuffer
    {
    public:
        void setResource(VkCommandBuffer res)
        {
            m_resource = res;
        }
        VkCommandBuffer getResource() const
        {
            return m_resource;
        }

    private:
        VkCommandBuffer m_resource = VK_NULL_HANDLE;
    };

    // ========================================
    // CommandPool
    // ========================================
    
    class VulkanCommandPool : public RHICommandPool
    {
    public:
        void setResource(VkCommandPool res)
        {
            m_resource = res;
        }
        VkCommandPool getResource() const
        {
            return m_resource;
        }

    private:
        VkCommandPool m_resource = VK_NULL_HANDLE;
    };

    // ========================================
    // DescriptorPool
    // ========================================
    
    class VulkanDescriptorPool : public RHIDescriptorPool
    {
    public:
        void setResource(VkDescriptorPool res)
        {
            m_resource = res;
        }
        VkDescriptorPool getResource() const
        {
            return m_resource;
        }

    private:
        VkDescriptorPool m_resource = VK_NULL_HANDLE;
    };

    // ========================================
    // DescriptorSet
    // ========================================
    
    class VulkanDescriptorSet : public RHIDescriptorSet
    {
    public:
        void setResource(VkDescriptorSet res)
        {
            m_resource = res;
        }
        VkDescriptorSet getResource() const
        {
            return m_resource;
        }

    private:
        VkDescriptorSet m_resource = VK_NULL_HANDLE;
    };

    // ========================================
    // DescriptorSetLayout
    // ========================================
    
    class VulkanDescriptorSetLayout : public RHIDescriptorSetLayout
    {
    public:
        void setResource(VkDescriptorSetLayout res)
        {
            m_resource = res;
        }
        VkDescriptorSetLayout getResource() const
        {
            return m_resource;
        }

    private:
        VkDescriptorSetLayout m_resource = VK_NULL_HANDLE;
    };

    // ========================================
    // Device
    // ========================================
    
    class VulkanDevice : public RHIDevice
    {
    public:
        void setResource(VkDevice res)
        {
            m_resource = res;
        }
        VkDevice getResource() const
        {
            return m_resource;
        }

    private:
        VkDevice m_resource = VK_NULL_HANDLE;
    };

    // ========================================
    // DeviceMemory
    // ========================================
    
    class VulkanDeviceMemory : public RHIDeviceMemory
    {
    public:
        void setResource(VkDeviceMemory res)
        {
            m_resource = res;
        }
        VkDeviceMemory getResource() const
        {
            return m_resource;
        }

    private:
        VkDeviceMemory m_resource = VK_NULL_HANDLE;
    };

    // ========================================
    // Event
    // ========================================
    
    class VulkanEvent : public RHIEvent
    {
    public:
        void setResource(VkEvent res)
        {
            m_resource = res;
        }
        VkEvent getResource() const
        {
            return m_resource;
        }

    private:
        VkEvent m_resource = VK_NULL_HANDLE;
    };

    // ========================================
    // Fence
    // ========================================
    
    class VulkanFence : public RHIFence
    {
    public:
        void setResource(VkFence res)
        {
            m_resource = res;
        }
        VkFence getResource() const
        {
            return m_resource;
        }

    private:
        VkFence m_resource = VK_NULL_HANDLE;
    };

    // ========================================
    // Framebuffer
    // ========================================
    
    class VulkanFramebuffer : public RHIFramebuffer
    {
    public:
        void setResource(VkFramebuffer res)
        {
            m_resource = res;
        }
        VkFramebuffer getResource() const
        {
            return m_resource;
        }

    private:
        VkFramebuffer m_resource = VK_NULL_HANDLE;
    };

    // ========================================
    // Image
    // ========================================
    
    class VulkanImage : public RHIImage
    {
    public:
        void setResource(VkImage res)
        {
            m_resource = res;
        }
        VkImage getResource() const
        {
            return m_resource;
        }

    private:
        VkImage m_resource = VK_NULL_HANDLE;
    };

    // ========================================
    // ImageView
    // ========================================
    
    class VulkanImageView : public RHIImageView
    {
    public:
        void setResource(VkImageView res)
        {
            m_resource = res;
        }
        VkImageView getResource() const
        {
            return m_resource;
        }

    private:
        VkImageView m_resource = VK_NULL_HANDLE;
    };

    // ========================================
    // Instance
    // ========================================
    
    class VulkanInstance : public RHIInstance
    {
    public:
        void setResource(VkInstance res)
        {
            m_resource = res;
        }
        VkInstance getResource() const
        {
            return m_resource;
        }

    private:
        VkInstance m_resource = VK_NULL_HANDLE;
    };

    // ========================================
    // Queue
    // ========================================
    
    class VulkanQueue : public RHIQueue
    {
    public:
        void setResource(VkQueue res)
        {
            m_resource = res;
        }
        VkQueue getResource() const
        {
            return m_resource;
        }

    private:
        VkQueue m_resource = VK_NULL_HANDLE;
    };

    // ========================================
    // PhysicalDevice
    // ========================================
    
    class VulkanPhysicalDevice : public RHIPhysicalDevice
    {
    public:
        void setResource(VkPhysicalDevice res)
        {
            m_resource = res;
        }
        VkPhysicalDevice getResource() const
        {
            return m_resource;
        }

    private:
        VkPhysicalDevice m_resource = VK_NULL_HANDLE;
    };

    // ========================================
    // Pipeline
    // ========================================
    
    class VulkanPipeline : public RHIPipeline
    {
    public:
        void setResource(VkPipeline res)
        {
            m_resource = res;
        }
        VkPipeline getResource() const
        {
            return m_resource;
        }

    private:
        VkPipeline m_resource = VK_NULL_HANDLE;
    };

    // ========================================
    // PipelineCache
    // ========================================
    
    class VulkanPipelineCache : public RHIPipelineCache
    {
    public:
        void setResource(VkPipelineCache res)
        {
            m_resource = res;
        }
        VkPipelineCache getResource() const
        {
            return m_resource;
        }

    private:
        VkPipelineCache m_resource = VK_NULL_HANDLE;
    };

    // ========================================
    // PipelineLayout
    // ========================================
    
    class VulkanPipelineLayout : public RHIPipelineLayout
    {
    public:
        void setResource(VkPipelineLayout res)
        {
            m_resource = res;
        }
        VkPipelineLayout getResource() const
        {
            return m_resource;
        }

    private:
        VkPipelineLayout m_resource = VK_NULL_HANDLE;
    };

    // ========================================
    // RenderPass
    // ========================================
    
    class VulkanRenderPass : public RHIRenderPass
    {
    public:
        void setResource(VkRenderPass res)
        {
            m_resource = res;
        }
        VkRenderPass getResource() const
        {
            return m_resource;
        }

    private:
        VkRenderPass m_resource = VK_NULL_HANDLE;
    };

    // ========================================
    // Sampler
    // ========================================
    
    class VulkanSampler : public RHISampler
    {
    public:
        void setResource(VkSampler res)
        {
            m_resource = res;
        }
        VkSampler getResource() const
        {
            return m_resource;
        }

    private:
        VkSampler m_resource = VK_NULL_HANDLE;
    };

    // ========================================
    // Semaphore
    // ========================================
    
    class VulkanSemaphore : public RHISemaphore
    {
    public:
        void setResource(VkSemaphore res)
        {
            m_resource = res;
        }
        VkSemaphore getResource() const
        {
            return m_resource;
        }

    private:
        VkSemaphore m_resource = VK_NULL_HANDLE;
    };

    // ========================================
    // Shader (ShaderModule)
    // ========================================
    
    class VulkanShader : public RHIShader
    {
    public:
        void setResource(VkShaderModule res)
        {
            m_resource = res;
        }
        VkShaderModule getResource() const
        {
            return m_resource;
        }

    private:
        VkShaderModule m_resource = VK_NULL_HANDLE;
    };

} // namespace BinRenderer::Vulkan