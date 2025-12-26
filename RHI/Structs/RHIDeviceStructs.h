#pragma once

#include "../Core/RHIType.h"
#include "RHICommonStructs.h"

namespace BinRenderer
{
	struct RHIApplicationInfo
	{
		const char* pApplicationName;
		uint32_t applicationVersion;
		const char* pEngineName;
		uint32_t engineVersion;
		uint32_t apiVersion;
	};

	struct RHIInstanceCreateInfo
	{
		RHIInstanceCreateFlags flags;
		const RHIApplicationInfo* pApplicationInfo;
		uint32_t enabledLayerCount;
		const char* const* ppEnabledLayerNames;
		uint32_t enabledExtensionCount;
		const char* const* ppEnabledExtensionNames;
	};

	struct RHILayerProperties
	{
		char layerName[256];
		uint32_t specVersion;
		uint32_t implementationVersion;
		char description[256];
	};

	struct RHIExtensionProperties
	{
		char extensionName[256];
		uint32_t specVersion;
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

	struct RHIMemoryType
    {
        RHIMemoryPropertyFlags propertyFlags;
        uint32_t heapIndex;
    };

	struct RHIMemoryHeap
    {
        RHIDeviceSize size;
        RHIMemoryHeapFlagBits flags;
    };

	struct RHIPhysicalDeviceMemoryProperties
    {
        uint32_t memoryTypeCount;
        RHIMemoryType memoryTypes[32];
        uint32_t memoryHeapCount;
        RHIMemoryHeap memoryHeaps[16];
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
        char deviceName[256];
        uint8_t pipelineCacheUUID[16];
        RHIPhysicalDeviceLimits limits;
        RHIPhysicalDeviceSparseProperties sparseProperties;
	};

	struct RHIDeviceQueueCreateInfo
    {
        RHIDeviceQueueCreateFlags flags;
        uint32_t queueFamilyIndex;
        uint32_t queueCount;
        const float* pQueuePriorities;
    };

	struct RHIDeviceCreateInfo
    {
        RHIDeviceCreateFlags flags;
        uint32_t queueCreateInfoCount;
        const RHIDeviceQueueCreateInfo* pQueueCreateInfos;
        uint32_t enabledLayerCount;
        const char* const* ppEnabledLayerNames;
        uint32_t enabledExtensionCount;
        const char* const* ppEnabledExtensionNames;
        const RHIPhysicalDeviceFeatures* pEnabledFeatures;
    };

	struct RHIMemoryAllocateInfo
    {
        RHIDeviceSize allocationSize;
        uint32_t memoryTypeIndex;
    };

	struct RHIMemoryRequirements
    {
        RHIDeviceSize size;
        RHIDeviceSize alignment;
        uint32_t memoryTypeBits;
    };

	struct RHIQueueFamilyProperties
    {
        RHIQueueFlags queueFlags;
        uint32_t queueCount;
        uint32_t timestampValidBits;
        RHIExtent3D minImageTransferGranularity;
    };

	struct RHIFormatProperties
    {
        RHIFormatFeatureFlags linearTilingFeatures;
        RHIFormatFeatureFlags optimalTilingFeatures;
        RHIFormatFeatureFlags bufferFeatures;
    };

} // namespace BinRenderer
