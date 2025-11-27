#pragma once

#include "RHIType.h"
#include <optional>
#include <vector>

namespace BinRenderer
{
    ////////////////////class////////////////////////
    class RHIBuffer {};
    class RHIBufferView {};
    class RHICommandBuffer {};
    class RHICommandPool {};
    class RHIDescriptorPool {};
    class RHIDescriptorSet {};
    class RHIDescriptorSetLayout {};
    class RHIDevice {};
    class RHIDeviceMemory {};
    class RHIEvent {};
    class RHIFence {};
    class RHIFramebuffer {};
    class RHIImage {};
    class RHIImageView {};
    class RHIInstance {};
    class RHIQueue {};
    class RHIPhysicalDevice {};
    class RHIPipeline {};
    class RHIPipelineCache {};
    class RHIPipelineLayout {};
    class RHIRenderPass {};
    class RHISampler {};
    class RHISemaphore {};
    class RHIShader {};

    ////////////////////struct//////////////////////////
    struct RHIMemoryBarrier;
    struct RHICopyDescriptorSet;
    struct RHIDescriptorImageInfo;
    struct RHIDescriptorBufferInfo;
    struct RHIOffset2D;
    struct RHISpecializationMapEntry;
    struct RHIBufferMemoryBarrier;
    struct RHIImageSubresourceRange;
    struct RHIImageMemoryBarrier;
    struct RHIExtent2D;
    struct RHIExtent3D;
    struct RHIApplicationInfo;
    struct RHIAttachmentDescription;
    struct RHIBufferCopy;
    struct RHIBufferCreateInfo;
    struct RHIBufferImageCopy;
    struct RHICommandBufferAllocateInfo;
    struct RHICommandBufferBeginInfo;
    struct RHICommandBufferInheritanceInfo;
    struct RHICommandPoolCreateInfo;
    struct RHIDescriptorPoolSize;
    struct RHIDescriptorPoolCreateInfo;
    struct RHIDescriptorSetAllocateInfo;
    struct RHIDescriptorSetLayoutBinding;
    struct RHIDescriptorSetLayoutCreateInfo;
    struct RHIDeviceCreateInfo;
    struct RHIDeviceQueueCreateInfo;
    struct RHIExtensionProperties;
    struct RHIFenceCreateInfo;
    struct RHIFormatProperties;
    struct RHIFramebufferCreateInfo;
    struct RHIGraphicsPipelineCreateInfo;
    struct RHIComputePipelineCreateInfo;
    struct RHIImageBlit;
    struct RHIImageCreateInfo;
    struct RHIImageFormatProperties;
    struct RHIImageViewCreateInfo;
    struct RHIInstanceCreateInfo;
    struct RHILayerProperties;
    struct RHIMemoryAllocateInfo;
    struct RHIMemoryHeap;
    struct RHIMemoryRequirements;
    struct RHIMemoryType;
    struct RHIPhysicalDeviceFeatures;
    struct RHIPhysicalDeviceLimits;
    struct RHIPhysicalDeviceMemoryProperties;
    struct RHIPhysicalDeviceProperties;
    struct RHIPhysicalDeviceSparseProperties;
    struct RHIPipelineColorBlendStateCreateInfo;
    struct RHIPipelineDepthStencilStateCreateInfo;
    struct RHIPipelineDynamicStateCreateInfo;
    struct RHIPipelineInputAssemblyStateCreateInfo;
    struct RHIPipelineLayoutCreateInfo;
    struct RHIPipelineMultisampleStateCreateInfo;
    struct RHIPipelineRasterizationStateCreateInfo;
    struct RHIPipelineShaderStageCreateInfo;
    struct RHIPipelineTessellationStateCreateInfo;
    struct RHIPipelineVertexInputStateCreateInfo;
    struct RHIPipelineViewportStateCreateInfo;
    struct RHIPushConstantRange;
    struct RHIQueueFamilyProperties;
    struct RHIRenderPassCreateInfo;
    struct RHISamplerCreateInfo;
    struct RHISemaphoreCreateInfo;
    struct RHIShaderModuleCreateInfo;
    struct RHISubmitInfo;
    struct RHISubpassDependency;
    struct RHISubpassDescription;
    struct RHIWriteDescriptorSet;
    struct RHIOffset3D;
    struct RHIAttachmentReference;
    struct RHIComponentMapping;
    struct RHIImageSubresourceLayers;
    struct RHIPipelineColorBlendAttachmentState;
    struct RHIRect2D;
    struct RHISpecializationInfo;
    struct RHIStencilOpState;
    struct RHIVertexInputAttributeDescription;
    struct RHIVertexInputBindingDescription;
    struct RHIViewport;
    struct RHIRenderPassBeginInfo;
    union RHIClearValue;
    union RHIClearColorValue;
    struct RHIClearDepthStencilValue;


    ///// struct declaration /////

    struct RHIOffset3D
    {
        int32_t x;
        int32_t y;
        int32_t z;
    };

    struct RHIExtent3D {
        uint32_t width;
        uint32_t height;
        uint32_t depth;
    };

    struct RHIOffset2D
    {
        int32_t x;
        int32_t y;
    };

    struct RHIExtent2D {
        uint32_t width;
        uint32_t height;
    };

    struct RHIMemoryBarrier
    {
        RHIStructType structType;
        const void* pNext;
        RHIAccessFlags srcAccessMask;
        RHIAccessFlags dstAccessMask;
    };

    struct RHICopyDescriptorSet
    {
        RHIStructType structType;
        const void* pNext;
        RHIDescriptorSet* srcSet;
        uint32_t srcBinding;
        uint32_t srcArrayElement;
        RHIDescriptorSet* dstSet;
        uint32_t dstBinding;
        uint32_t dstArrayElement;
        uint32_t descriptorCount;
    };

    struct RHiDescriptorImageInfo
    {
        RHISampler* smapler;
        RHIImageView* imageView;
        RHIImageLayout imageLayout;
    };

    struct RHIDescriptorBufferInfo
    {
        RHIBuffer* buffer;
        RHIDeviceSize offset;
        RHIDeviceSize range;
    };

    struct RHIOffset2D
    {
        int32_t x;
        int32_t y;
    };

    struct RHISpecializationMapEntry
    {
        uint32_t constantID;
        uint32_t offset;
        size_t size;
    };

    struct RHIBufferMemoryBarrier
    {
        RHIStructType structType;
        const void* pNext;
        RHIAccessFlags srcAccessMask;
        RHIAccessFlags dstAccessMask;
        uint32_t srcQueueFamilyIndex;
        uint32_t dstQueueFamilyIndex;
        RHIBuffer* buffer;
        RHIDeviceSize offset;
        RHIDeviceSize size;
    };

    struct RHIImageSubresourceRange
    {
        RHIImageAspectFlagBits aspectMask;
        uint32_t baseMipLevel;
        uint32_t levelCount;
        uint32_t baseArrayLayer;
        uint32_t layerCount;
    };

    struct RHIImageMemoryBarrier
    {
        RHIStructType structType;
        const void* pNext;
        RHIAccessFlags srcAccessMask;
        RHIAccessFlags dstAccessMask;
        RHIImageLayout oldLayout;
        RHIImageLayout newLayout;
        uint32_t srcQueueFamilyIndex;
        uint32_t dstQueueFamilyIndex;
        RHIImage* image;
        RHIImageSubresourceRange subresourceRange;
    };

    struct RHIApplicationInfo
    {
        RHIStructType structType;
        const void* pNext;
        const char* pApplicationName;
        uint32_t applicationVersion;
        const char* pEngineName;
        uint32_t engineVersion;
        uint32_t apiVersion;
    };

    struct RHIAttachmentDescription
    {
        RHIStructType structType;
        const void* pNext;
        RHIFormat format;
        RHISampleCountFlagBits samples;
        RHIAttachmentLoadOp loadOp;
        RHIAttachmentStoreOp storeOp;
        RHIAttachmentLoadOp stencilLoadOp;
        RHIAttachmentStoreOp stencilStoreOp;
        RHIImageLayout initialLayout;
        RHIImageLayout finalLayout;
    };

    struct BufferCopy
    {
        RHIDeviceSize srcOffset;
        RHIDeviceSize dstOffset;
        RHIDeviceSize size;
    };

    struct RHIBufferCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIBufferCreateFlags flags;
        RHIDeviceSize size;
        RHIBufferUsageFlags usage;
        RHISharingMode sharingMode;
        uint32_t queueFamilyIndexCount;
        const uint32_t* pQueueFamilyIndices;
    };

    struct RHIImageSubresourceLayers
    {
        RHIImageAspectFlagBits aspectMask;
        uint32_t mipLevel;
        uint32_t baseArrayLayer;
        uint32_t layerCount;
    };

    struct RHIBufferImageCopy
    {
        RHIDeviceSize bufferOffset;
        uint32_t bufferRowLength;
        uint32_t bufferImageHeight;
        RHIImageSubresourceLayers imageSubresource;
        RHIOffset3D imageOffset;
        RHIExtent3D imageExtent;
    };

    struct RHICommandBufferAllocateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHICommandPool* commandPool;
        RHICommandBufferLevel level;
        uint32_t commandBufferCount;
    };

    struct RHICommandBufferBeginInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHICommandBufferUsageFlags flags;
        const RHICommandBufferInheritanceInfo* pInheritanceInfo;
    };

    struct RHICommandBufferInheritanceInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIRenderPass* renderPass;
        uint32_t subpass;
        RHIFramebuffer* framebuffer;
        bool occlusionQueryEnable;
        RHIQueryControlFlags queryFlags;
        RHIQueryPipelineStatisticFlags pipelineStatistics;
    };

    struct RHICommandPoolCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHICommandPoolCreateFlags flags;
        uint32_t queueFamilyIndex;
    };

    struct RHIDescriptorPoolSize
    {
        RHIDescriptorType type;
        uint32_t descriptorCount;
    };

    struct RHIDescriptorPoolCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIDescriptorPoolCreateFlags flags;
        uint32_t maxSets;
        uint32_t poolSizeCount;
        const RHIDescriptorPoolSize* pPoolSizes;
    };

    struct RHIDescriptorSetAllocateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIDescriptorPool* descriptorPool;
        uint32_t descriptorSetCount;
        const RHIDescriptorSetLayout* pSetLayouts;
    };

    struct RHIDescriptorSetLayoutBinding
    {
        uint32_t binding;
        RHIDescriptorType descriptorType;
        uint32_t descriptorCount;
        RHIShaderStageFlags stageFlags;
        const RHISampler* const* pImmutableSamplers = nullptr;
    };

    struct RHIDescriptorSetLayoutCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIDescriptorSetLayoutBindingFlags* pBindingFlags;
        uint32_t bindingCount;
        const RHIDescriptorSetLayoutBinding* pBindings;
    };

    struct RHIDeviceCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIDeviceCreateFlags flags;
        uint32_t queueCreateInfoCount;
        const RHIDeviceQueueCreateInfo* pQueueCreateInfos;
        uint32_t enabledLayerCount;
        const char* const* ppEnabledLayerNames;
        uint32_t enabledExtensionCount;
        const char* const* ppEnabledExtensionNames;
        const RHIPhysicalDeviceFeatures* pEnabledFeatures;
    };

    struct RHIDeviceQueueCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIDeviceQueueCreateFlags flags;
        uint32_t queueFamilyIndex;
        uint32_t queueCount;
        const float* pQueuePriorities;
    };

    struct RHIExtensionProperties
    {
        char extensionName[256]; // TOOD : define RHI_MAX_EXTENSION_NAME_SIZE
        uint32_t specVersion;
    };

    struct RHIFenceCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIFenceCreateFlags flags;
    };

    struct RHIFormatProperties
    {
        RHIFormatFeatureFlags linearTilingFeatures;
        RHIFormatFeatureFlags optimalTilingFeatures;
        RHIFormatFeatureFlags bufferFeatures;
    };

    struct RHIFramebufferCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIFramebufferCreateFlags flags;
        RHIRenderPass* renderPass;
        uint32_t attachmentCount;
        const RHIImageView* pAttachments;
        uint32_t width;
        uint32_t height;
        uint32_t layers;
    };

    struct RHIGraphicsPipelineCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        uint32_t stageCount;
        const RHIPipelineShaderStageCreateInfo* pStages;
        const RHIPipelineVertexInputStateCreateInfo* pVertexInputState;
        const RHIPipelineInputAssemblyStateCreateInfo* pInputAssemblyState;
        const RHIPipelineTessellationStateCreateInfo* pTessellationState;
        const RHIPipelineViewportStateCreateInfo* pViewportState;
        const RHIPipelineRasterizationStateCreateInfo* pRasterizationState;
        const RHIPipelineMultisampleStateCreateInfo* pMultisampleState;
        const RHIPipelineDepthStencilStateCreateInfo* pDepthStencilState;
        const RHIPipelineColorBlendStateCreateInfo* pColorBlendState;
        const RHIPipelineDynamicStateCreateInfo* pDynamicState;
        RHIPipelineLayout* layout;
        RHIRenderPass* renderPass;
        uint32_t subpass;
        RHIPipeline basePipelineHandle;
        int32_t basePipelineIndex;
    };

    struct RHIComputePipelineCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIPipelineCreateFlags flags;
        RHIPipelineShaderStageCreateInfo* stage;
        RHIPipelineLayout* layout;
        RHIPipeline basePipelineHandle;
        int32_t basePipelineIndex;
    };

    struct RHIImageBlit
    {
        RHIImageSubresourceLayers srcSubresource;
        RHIOffset3D srcOffsets[2];
        RHIImageSubresourceLayers dstSubresource;
        RHIOffset3D dstOffsets[2];
    };

    struct RHIImageCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIImageCreateFlags flags;
        RHIImageType imageType;
        RHIFormat format;
        RHIExtent3D extent;
        uint32_t mipLevels;
        uint32_t arrayLayers;
        RHISampleCountFlagBits samples;
        RHIImageTiling tiling;
        RHIImageUsageFlags usage;
        RHISharingMode sharingMode;
        uint32_t queueFamilyIndexCount;
        const uint32_t* pQueueFamilyIndices;
        RHIImageLayout initialLayout;
    };

    struct RHIImageFormatProperties
    {
        RHIExtent3D maxExtent;
        uint32_t maxMipLevels;
        uint32_t maxArrayLayers;
        RHISampleCountFlagBits sampleCounts;
        RHIDeviceSize maxResourceSize;
    };

    struct RHIComponentMapping
    {
        RHIComponentSwizzle r;
        RHIComponentSwizzle g;
        RHIComponentSwizzle b;
        RHIComponentSwizzle a;
    };

    struct RHIImageViewCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIImageViewCreateFlags flags;
        RHIImage* image;
        RHIImageViewType viewType;
        RHIFormat format;
        RHIComponentMapping components;
        RHIImageSubresourceRange subresourceRange;
    };

    struct RHIInstanceCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIInstanceCreateFlags flags;
        const RHIApplicationInfo* pApplicationInfo;
        uint32_t enabledLayerCount;
        const char* const* ppEnabledLayerNames;
        uint32_t enabledExtensionCount;
        const char* const* ppEnabledExtensionNames;
    };

    struct RHILayerProperties
    {
        char layerName[256]; // TOOD : define RHI_MAX_LAYER_NAME_SIZE
        uint32_t specVersion;
        uint32_t implementationVersion;
        char description[256]; // TOOD : define RHI_MAX_DESCRIPTION_SIZE
    };

    struct RHIMemoryAllocateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIDeviceSize allocationSize;
        uint32_t memoryTypeIndex;
    };

    struct RHIMemoryHeap
    {
        RHIDeviceSize size;
        RHIMemoryHeapFlagBits flags;
    };

    struct RHIMemoryRequirements
    {
        RHIDeviceSize size;
        RHIDeviceSize alignment;
        uint32_t memoryTypeBits;
    };

    struct RHIMemoryType
    {
        RHIMemoryPropertyFlags propertyFlags;
        uint32_t heapIndex;
    };

    struct RHIPhysicalDeviceFeatures
    {
        bool robustBufferAccess;
        bool fullDrawIndexUint32;
        bool imageCubeArray;
        bool independentBlend;
        bool geometryShader;
        bool tessellationShader;
        bool sampleRateShading;
        bool dualSrcBlend;
        bool logicOp;
        bool multiDrawIndirect;
        bool drawIndirectFirstInstance;
        bool depthClamp;
        bool depthBiasClamp;
        bool fillModeNonSolid;
        bool wideLines;
        bool largePoints;
        bool alphaToOne;
        bool multiViewport;
        bool samplerAnisotropy;
        bool textureCompressionETC2;
        bool textureCompressionASTC_LDR;
        bool textureCompressionBC;
        bool occlusionQueryPrecise;
        bool pipelineStatisticsQuery;
        bool vertexPipelineStoresAndAtomics;
        bool fragmentStoresAndAtomics;
        bool shaderTessellationAndGeometryPointSize;
        bool shaderImageGatherExtended;
        bool shaderStorageImageExtendedFormats;
        bool shaderStorageImageMultisample;
        bool shaderStorageImageReadWithoutFormat;
        bool shaderStorageImageWriteWithoutFormat;
        bool shaderUniformBufferArrayDynamicIndexing;
        bool shaderSampledImageArrayDynamicIndexing;
        bool shaderStorageBufferArrayDynamicIndexing;
        bool shaderStorageImageArrayDynamicIndexing;
        bool shaderClipDistance;
        bool shaderCullDistance;
        bool shaderFloat64;
        bool shaderInt64;
        bool shaderInt16;
        bool shaderResourceResidency;
        bool shaderResourceMinLod;
        bool sparseBinding;
        bool sparseResidencyBuffer;
        bool sparseResidencyImage2D;
        bool sparseResidencyImage3D;
        bool sparseResidency2Samples;
        bool sparseResidency4Samples;
        bool sparseResidency8Samples;
        bool sparseResidency16Samples;
        bool sparseResidencyAliased;
        bool variableMultisampleRate;
        bool inheritedQueries;
    };

    struct RHIPhysicalDeviceLimits
    {
        uint32_t maxImageDimension1D;
        uint32_t maxImageDimension2D;
        uint32_t maxImageDimension3D;
        uint32_t maxImageDimensionCube;
        uint32_t maxImageArrayLayers;
        uint32_t maxTexelBufferElements;
        uint32_t maxUniformBufferRange;
        uint32_t maxStorageBufferRange;
        uint32_t maxPushConstantsSize;
        uint32_t maxMemoryAllocationCount;
        uint32_t maxSamplerAllocationCount;
        RHIDeviceSize bufferImageGranularity;
        RHIDeviceSize sparseAddressSpaceSize;
        uint32_t maxBoundDescriptorSets;
        uint32_t maxPerStageDescriptorSamplers;
        uint32_t maxPerStageDescriptorUniformBuffers;
        uint32_t maxPerStageDescriptorStorageBuffers;
        uint32_t maxPerStageDescriptorSampledImages;
        uint32_t maxPerStageDescriptorStorageImages;
        uint32_t maxPerStageDescriptorInputAttachments;
        uint32_t maxPerStageResources;
        uint32_t maxDescriptorSetSamplers;
        uint32_t maxDescriptorSetUniformBuffers;
        uint32_t maxDescriptorSetUniformBuffersDynamic;
        uint32_t maxDescriptorSetStorageBuffers;
        uint32_t maxDescriptorSetStorageBuffersDynamic;
        uint32_t maxDescriptorSetSampledImages;
        uint32_t maxDescriptorSetStorageImages;
        uint32_t maxDescriptorSetInputAttachments;
        uint32_t maxVertexInputAttributes;
        uint32_t maxVertexInputBindings;
        uint32_t maxVertexInputAttributeOffset;
        uint32_t maxVertexInputBindingStride;
        uint32_t maxVertexOutputComponents;
        uint32_t maxTessellationGenerationLevel;
        uint32_t maxTessellationPatchSize;
        uint32_t maxTessellationControlPerVertexInputComponents;
        uint32_t maxTessellationControlPerVertexOutputComponents;
        uint32_t maxTessellationControlPerPatchOutputComponents;
        uint32_t maxTessellationControlTotalOutputComponents;
        uint32_t maxTessellationEvaluationInputComponents;
        uint32_t maxTessellationEvaluationOutputComponents;
        uint32_t maxGeometryShaderInvocations;
        uint32_t maxGeometryInputComponents;
        uint32_t maxGeometryOutputComponents;
        uint32_t maxGeometryOutputVertices;
        uint32_t maxGeometryTotalOutputComponents;
        uint32_t maxFragmentInputComponents;
        uint32_t maxFragmentOutputAttachments;
        uint32_t maxFragmentDualSrcAttachments;
        uint32_t maxFragmentCombinedOutputResources;
        uint32_t maxComputeSharedMemorySize;
        uint32_t maxComputeWorkGroupCount[3];
        uint32_t maxComputeWorkGroupInvocations;
        uint32_t maxComputeWorkGroupSize[3];
        uint32_t subPixelPrecisionBits;
        uint32_t subTexelPrecisionBits;
        uint32_t mipmapPrecisionBits;
        uint32_t maxDrawIndexedIndexValue;
        uint32_t maxDrawIndirectCount;
        float maxSamplerLodBias;
        float maxSamplerAnisotropy;
        uint32_t maxViewports;
        uint32_t maxViewportDimensions[2];
        float viewportBoundsRange[2];
        uint32_t viewportSubPixelBits;
        size_t minMemoryMapAlignment;
        RHIDeviceSize minTexelBufferOffsetAlignment;
        RHIDeviceSize minUniformBufferOffsetAlignment;
        RHIDeviceSize minStorageBufferOffsetAlignment;
        int32_t minTexelOffset;
        uint32_t maxTexelOffset;
        int32_t minTexelGatherOffset;
        uint32_t maxTexelGatherOffset;
        float minInterpolationOffset;
        float maxInterpolationOffset;
        uint32_t subPixelInterpolationOffsetBits;
        uint32_t maxFramebufferWidth;
        uint32_t maxFramebufferHeight;
        uint32_t maxFramebufferLayers;
        RHISampleCountFlags framebufferColorSampleCounts;
        RHISampleCountFlags framebufferDepthSampleCounts;
        RHISampleCountFlags framebufferStencilSampleCounts;
        RHISampleCountFlags framebufferNoAttachmentsSampleCounts;
        uint32_t maxColorAttachments;
        RHISampleCountFlags sampledImageColorSampleCounts;
        RHISampleCountFlags sampledImageIntegerSampleCounts;
        RHISampleCountFlags sampledImageDepthSampleCounts;
        RHISampleCountFlags sampledImageStencilSampleCounts;
        RHISampleCountFlags storageImageSampleCounts;
        uint32_t maxSampleMaskWords;
        bool timestampComputeAndGraphics;
        float timestampPeriod;
        uint32_t maxClipDistances;
        uint32_t maxCullDistances;
        uint32_t maxCombinedClipAndCullDistances;
        uint32_t discreteQueuePriorities;
        float pointSizeRange[2];
        float lineWidthRange[2];
        float pointSizeGranularity;
        float lineWidthGranularity;
        bool strictLines;
        bool standardSampleLocations;
        RHIDeviceSize optimalBufferCopyOffsetAlignment;
        RHIDeviceSize optimalBufferCopyRowPitchAlignment;
        RHIDeviceSize nonCoherentAtomSize;
    };

    struct RHIPhysicalDeviceMemoryProperties
    {
        uint32_t memoryTypeCount;
        RHIMemoryType memoryTypes[32]; // TODO: define RHI_MAX_MEMORY_TYPES
        uint32_t memoryHeapCount;
        RHIMemoryHeap memoryHeaps[16]; // TODO: define RHI_MAX_MEMORY_HEAPS
	};

    struct RHIPhysicalDeviceSparseProperties
    {
        bool residencyStandard2DBlockShape;
        bool residencyStandard2DMultisampleBlockShape;
        bool residencyStandard3DBlockShape;
        bool residencyAlignedMipSize;
        bool residencyNonResidentStrict;
    };

    struct RHIPhysicalDeviceProperties
    {
        uint32_t apiVersion;
        uint32_t driverVersion;
        uint32_t vendorID;
        uint32_t deviceID;
        RHIPhysicalDeviceType deviceType;
        char deviceName[256]; // TOOD : define RHI_MAX_DEVICE_NAME_SIZE
        uint8_t pipelineCacheUUID[16];
        RHIPhysicalDeviceLimits limits;
        RHIPhysicalDeviceSparseProperties sparseProperties;
	};

    struct RHIPipelineColorBlendStateCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIPipelineColorBlendStateCreateFlags flags;
        bool logicOpEnable;
        RHILogicOp logicOp;
        uint32_t attachmentCount;
        const RHIPipelineColorBlendAttachmentState* pAttachments;
        float blendConstants[4];
	};

    struct RHIStencilOpState
    {
        RHIStencilOp failOp;
        RHIStencilOp passOp;
        RHIStencilOp depthFailOp;
        RHICompareOp compareOp;
        uint32_t compareMask;
        uint32_t writeMask;
        uint32_t reference;
	};

    struct RHIPipelineDepthStencilStateCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIPipelineDepthStencilStateCreateFlags flags;
        bool depthTestEnable;
        bool depthWriteEnable;
        RHICompareOp depthCompareOp;
        bool depthBoundsTestEnable;
        bool stencilTestEnable;
        RHIStencilOpState front;
        RHIStencilOpState back;
        float minDepthBounds;
        float maxDepthBounds;
    };

    struct RHIPipelineDynamicStateCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIPipelineDynamicStateCreateFlags flags;
        uint32_t dynamicStateCount;
        const RHIPipelineDynamicState* pDynamicStates;
	};

    struct RHIPipelineInputAssemblyStateCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIPipelineInputAssemblyStateCreateFlags flags;
        RHIPrimitiveTopology topology;
        bool primitiveRestartEnable;
    };

    struct RHIPipelineLayoutCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIPipelineLayoutCreateFlags flags;
        uint32_t setLayoutCount;
        const RHIDescriptorSetLayout* pSetLayouts;
        uint32_t pushConstantRangeCount;
        const RHIPushConstantRange* pPushConstantRanges;
    };

    struct RHIPipelineMultisampleStateCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIPipelineMultisampleStateCreateFlags flags;
        RHISampleCountFlagBits rasterizationSamples;
        bool sampleShadingEnable;
        float minSampleShading;
        const RHISampleMask** pSampleMask;
        bool alphaToCoverageEnable;
        bool alphaToOneEnable;
	};

    struct RHIPipelineRasterizationStateCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIPipelineRasterizationStateCreateFlags flags;
        bool depthClampEnable;
        bool rasterizerDiscardEnable;
		RHIPolygonMode polygonMode;
        RHICullModeFlags cullMode;
        RHIFrontFace frontFace;
        bool depthBiasEnable;
        float depthBiasConstantFactor;
        float depthBiasClamp;
        float depthBiasSlopeFactor;
        float lineWidth;
    };

    struct RHIPipelineShaderStageCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIPipelineShaderStageCreateFlags flags;
        RHIShaderStageFlagBits stage;
        RHIShader* module;
        const char* pName;
        const RHISpecializationInfo* pSpecializationInfo;
	};

    struct RHIPipelineTessellationStateCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIPipelineTessellationStateCreateFlags flags;
        uint32_t patchControlPoints;
    };

    struct RHIPipelineVertexInputStateCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIPipelineVertexInputStateCreateFlags flags;
        uint32_t vertexBindingDescriptionCount;
        const RHIVertexInputBindingDescription* pVertexBindingDescriptions;
        uint32_t vertexAttributeDescriptionCount;
        const RHIVertexInputAttributeDescription* pVertexAttributeDescriptions;
    };

    struct RHIPipelineViewportStateCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIPipelineViewportStateCreateFlags flags;
        uint32_t viewportCount;
        const RHIViewport* pViewports;
        uint32_t scissorCount;
        const RHIRect2D* pScissors;
	};

    struct RHIPushConstantRange
    {
        RHIShaderStageFlags stageFlags;
        uint32_t offset;
        uint32_t size;
    };

    struct RHIQueueFamilyProperties
    {
        RHIQueueFlags queueFlags;
        uint32_t queueCount;
        uint32_t timestampValidBits;
        RHIExtent3D minImageTransferGranularity;
    };

    struct RHIRenderPassCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIRenderPassCreateFlags flags;
        uint32_t attachmentCount;
        const RHIAttachmentDescription* pAttachments;
        uint32_t subpassCount;
        const RHISubpassDescription* pSubpasses;
        uint32_t dependencyCount;
        const RHISubpassDependency* pDependencies;
    };

    struct RHISamplerCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHISamplerCreateFlags flags;
        RHIFilter magFilter;
        RHIFilter minFilter;
        RHISamplerMipmapMode mipmapMode;
        RHISamplerAddressMode addressModeU;
        RHISamplerAddressMode addressModeV;
        RHISamplerAddressMode addressModeW;
        float mipLodBias;
        bool anisotropyEnable;
        float maxAnisotropy;
        bool compareEnable;
        RHICompareOp compareOp;
        float minLod;
        float maxLod;
        RHIBorderColor borderColor;
        bool unnormalizedCoordinates;
	};

    struct RHISemaphoreCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHISemaphoreCreateFlags flags;
    };

    struct RHIShaderModuleCreateInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIShaderModuleCreateFlags flags;
		size_t codeSize;
		const uint32_t* pCode;
	};

    struct RHISubmitInfo
    {
        RHIStructType structType;
        const void* pNext;
        uint32_t waitSemaphoreCount;
        const RHISemaphore* const* pWaitSemaphores;
        const RHIPipelineStageFlags* pWaitDstStageMask;
        uint32_t commandBufferCount;
        const RHICommandBuffer* const* pCommandBuffers;
        uint32_t signalSemaphoreCount;
        const RHISemaphore* const* pSignalSemaphores;
	};

    struct RHISubpassDependency
    {
        uint32_t srcSubpass;
        uint32_t dstSubpass;
        RHIPipelineStageFlags srcStageMask;
        RHIPipelineStageFlags dstStageMask;
        RHIAccessFlags srcAccessMask;
        RHIAccessFlags dstAccessMask;
        RHISubpassDependencyFlags dependencyFlags;
    };

    struct RHISubpassDescription
    {
        RHISubpassDescriptionFlags flags;
        RHIPipelineBindPoint pipelineBindPoint;
        uint32_t inputAttachmentCount;
        const RHIAttachmentReference* pInputAttachments;
        uint32_t colorAttachmentCount;
        const RHIAttachmentReference* pColorAttachments;
        const RHIAttachmentReference* pResolveAttachments;
        const RHIAttachmentReference* pDepthStencilAttachment;
        uint32_t preserveAttachmentCount;
        const uint32_t* pPreserveAttachments;
    };

    struct RHIWriteDescriptorSet
    {
        RHIStructType structType;
        const void* pNext;
        RHIDescriptorSet* dstSet;
        uint32_t dstBinding;
        uint32_t dstArrayElement;
        uint32_t descriptorCount;
        RHIDescriptorType descriptorType;
        const RHIDescriptorImageInfo* pImageInfo = nullptr;
        const RHIDescriptorBufferInfo* pBufferInfo = nullptr;
        const RHIBufferView* const* pTexelBufferView = nullptr;
    };

    struct RHIAttachmentReference
    {
        uint32_t attachment;
        RHIImageLayout layout;
	};

    struct RHIPipelineColorBlendAttachmentState
    {
		bool blendEnable;
		RHIBlendFactor srcColorBlendFactor;
		RHIBlendFactor dstColorBlendFactor;
		RHIBlendOp colorBlendOp;
		RHIBlendFactor srcAlphaBlendFactor;
		RHIBlendFactor dstAlphaBlendFactor;
		RHIBlendOp alphaBlendOp;
		RHIColorComponentFlags colorWriteMask;
	};

    struct RHIRect2D
    {
        RHIOffset2D offset;
        RHIExtent2D extent;
	};

    struct RHISpecializationInfo
    {
        uint32_t mapEntryCount;
        const RHISpecializationMapEntry* pMapEntries;
        size_t dataSize;
        const void* pData;
    };
    struct RHIVertexInputAttributeDescription
    {
        uint32_t location;
        uint32_t binding;
        RHIFormat format;
        uint32_t offset;
    };
    struct RHIVertexInputBindingDescription
    {
        uint32_t binding;
        uint32_t stride;
        RHIVertexInputRate inputRate;
    };
    struct RHIViewport
    {
        float x;
        float y;
        float width;
        float height;
        float minDepth;
        float maxDepth;
    };
    struct RHIRenderPassBeginInfo
    {
        RHIStructType structType;
        const void* pNext;
        RHIRenderPass* renderPass;
        RHIFramebuffer* framebuffer;
        RHIRect2D renderArea;
        uint32_t clearValueCount;
        const RHIClearValue* pClearValues;
    };
    union RHIClearValue
    {
        RHIClearColorValue color;
        RHIClearDepthStencilValue depthStencil;
    };
    union RHIClearColorValue
    {
        float float32[4];
        int32_t int32[4];
        uint32_t uint32[4];
    };
    struct RHIClearDepthStencilValue
    {
        float depth;
        uint32_t stencil;
	};

    struct RHIClearRect
    {
        RHIRect2D rect;
        uint32_t baseArrayLayer;
        uint32_t layerCount;
    };

    struct RHIClearAttachment
    {
        RHIImageAspectFlagBits aspectMask;
        uint32_t colorAttachment;
        RHIClearValue clearValue;
	};

    struct RHISwapChainDesc
    {
        RHIExtent2D extent;
        RHIFormat   image_format;
        RHIViewport* viewport;
        RHIRect2D* scissor;
        std::vector<RHIImageView*> imageViews;
    };

    struct RHIDepthImageDesc
    {
		RHIImage* depthImage;
		RHIImageView* depthImageView;
		RHIFormat depthFormat;
    };

	// TODO : move to platform specific file
    /*struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphics_family;
        std::optional<uint32_t> present_family;
        std::optional<uint32_t> m_compute_family;

        bool isComplete() { return graphics_family.has_value() && present_family.has_value() && m_compute_family.has_value();; }
    };

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR        capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR>   presentModes;
    };*/
}