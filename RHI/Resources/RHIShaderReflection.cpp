#include "RHIShaderReflection.h"
#include "../../Core/Logger.h"
#include <sstream>

namespace BinRenderer
{
	// ========================================
	// Helper Functions
	// ========================================

	static const char* descriptorTypeToString(RHIDescriptorType type)
	{
		switch (type)
		{
		case RHIDescriptorType::Sampler: return "Sampler";
		case RHIDescriptorType::CombinedImageSampler: return "CombinedImageSampler";
		case RHIDescriptorType::SampledImage: return "SampledImage";
		case RHIDescriptorType::StorageImage: return "StorageImage";
		case RHIDescriptorType::UniformTexelBuffer: return "UniformTexelBuffer";
		case RHIDescriptorType::StorageTexelBuffer: return "StorageTexelBuffer";
		case RHIDescriptorType::UniformBuffer: return "UniformBuffer";
		case RHIDescriptorType::StorageBuffer: return "StorageBuffer";
		case RHIDescriptorType::UniformBufferDynamic: return "UniformBufferDynamic";
		case RHIDescriptorType::StorageBufferDynamic: return "StorageBufferDynamic";
		case RHIDescriptorType::InputAttachment: return "InputAttachment";
		case RHIDescriptorType::AccelerationStructure: return "AccelerationStructure";
		default: return "Unknown";
		}
	}

	static const char* shaderStageToString(RHIShaderStage stage)
	{
		switch (stage)
		{
		case RHIShaderStage::Vertex: return "Vertex";
		case RHIShaderStage::TessellationControl: return "TessControl";
		case RHIShaderStage::TessellationEvaluation: return "TessEval";
		case RHIShaderStage::Geometry: return "Geometry";
		case RHIShaderStage::Fragment: return "Fragment";
		case RHIShaderStage::Compute: return "Compute";
		case RHIShaderStage::AllGraphics: return "AllGraphics";
		case RHIShaderStage::All: return "All";
		default: return "Unknown";
		}
	}

	static std::string stageFlagsToString(RHIShaderStage flags)
	{
		std::stringstream ss;
		bool first = true;

		auto addStage = [&](RHIShaderStage stage, const char* name)
		{
			if ((flags & stage) == stage)
			{
				if (!first) ss << " | ";
				ss << name;
				first = false;
			}
		};

		addStage(RHIShaderStage::Vertex, "Vertex");
		addStage(RHIShaderStage::TessellationControl, "TessControl");
		addStage(RHIShaderStage::TessellationEvaluation, "TessEval");
		addStage(RHIShaderStage::Geometry, "Geometry");
		addStage(RHIShaderStage::Fragment, "Fragment");
		addStage(RHIShaderStage::Compute, "Compute");

		return ss.str();
	}

	static const char* vertexFormatToString(RHIVertexFormat format)
	{
		switch (format)
		{
		case RHIVertexFormat::R32_Float: return "R32_Float";
		case RHIVertexFormat::R32G32_Float: return "R32G32_Float";
		case RHIVertexFormat::R32G32B32_Float: return "R32G32B32_Float";
		case RHIVertexFormat::R32G32B32A32_Float: return "R32G32B32A32_Float";
		case RHIVertexFormat::R32_Sint: return "R32_Sint";
		case RHIVertexFormat::R32G32_Sint: return "R32G32_Sint";
		case RHIVertexFormat::R32G32B32_Sint: return "R32G32B32_Sint";
		case RHIVertexFormat::R32G32B32A32_Sint: return "R32G32B32A32_Sint";
		case RHIVertexFormat::R32_Uint: return "R32_Uint";
		case RHIVertexFormat::R32G32_Uint: return "R32G32_Uint";
		case RHIVertexFormat::R32G32B32_Uint: return "R32G32B32_Uint";
		case RHIVertexFormat::R32G32B32A32_Uint: return "R32G32B32A32_Uint";
		case RHIVertexFormat::R8G8B8A8_Unorm: return "R8G8B8A8_Unorm";
		case RHIVertexFormat::R8G8B8A8_Snorm: return "R8G8B8A8_Snorm";
		default: return "Undefined";
		}
	}

	// ========================================
	// ShaderReflectionData Implementation
	// ========================================

	void ShaderReflectionData::printDebugInfo() const
	{
		printLog("========================================");
		printLog("Shader Reflection Data");
		printLog("========================================");
		printLog("Stage: {}", shaderStageToString(stage));
		printLog("Entry Point: {}", entryPoint);
		printLog("");

		// Descriptor Bindings
		if (!bindings.empty())
		{
			printLog("Descriptor Bindings:");
			for (const auto& [setIdx, setBindings] : bindings)
			{
				printLog("  Set {}:", setIdx);
				for (const auto& binding : setBindings)
				{
					printLog("    Binding {}: {} '{}'", 
						binding.binding,
						descriptorTypeToString(binding.descriptorType),
						binding.name);
					printLog("      Count: {}", binding.descriptorCount);
					printLog("      Stages: {}", stageFlagsToString(binding.stageFlags));
					
					if (binding.bufferSize > 0)
					{
						printLog("      Buffer Size: {} bytes", binding.bufferSize);
					}
					
					if (binding.writeOnly)
					{
						printLog("      Access: Write-Only");
					}
				}
			}
			printLog("");
		}

		// Push Constants
		if (!pushConstants.empty())
		{
			printLog("Push Constants:");
			for (const auto& pc : pushConstants)
			{
				printLog("  '{}' - Offset: {}, Size: {}, Stages: {}",
					pc.name, pc.offset, pc.size, stageFlagsToString(pc.stageFlags));
			}
			printLog("");
		}

		// Vertex Inputs
		if (!vertexInputs.empty())
		{
			printLog("Vertex Inputs:");
			for (const auto& input : vertexInputs)
			{
				printLog("  Location {}: {} '{}' (offset: {})",
					input.location,
					vertexFormatToString(input.format),
					input.name,
					input.offset);
				
				if (!input.semanticName.empty())
				{
					printLog("    Semantic: {}", input.semanticName);
				}
			}
			printLog("");
		}

		// Compute Workgroup Size
		if (stage == RHIShaderStage::Compute)
		{
			printLog("Compute Workgroup Size: ({}, {}, {})",
				workgroupSizeX, workgroupSizeY, workgroupSizeZ);
			printLog("");
		}

		// Resource Usage
		printLog("Resource Usage:");
		printLog("  Uniform Buffers: {}", resourceUsage.numUniformBuffers);
		printLog("  Storage Buffers: {}", resourceUsage.numStorageBuffers);
		printLog("  Sampled Images: {}", resourceUsage.numSampledImages);
		printLog("  Storage Images: {}", resourceUsage.numStorageImages);
		printLog("  Samplers: {}", resourceUsage.numSamplers);
		printLog("  Input Attachments: {}", resourceUsage.numInputAttachments);
		printLog("  Total Descriptors: {}", resourceUsage.totalDescriptors);
		printLog("========================================");
	}

} // namespace BinRenderer
