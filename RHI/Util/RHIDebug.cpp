#include "RHIDebug.h"
#include "../Structs/RHIStructs.h"
#include <iostream>
#include <sstream>

namespace BinRenderer
{
	RHIDebug::DebugMessageCallback RHIDebug::s_debugCallback = nullptr;

	void RHIDebug::setDebugCallback(DebugMessageCallback callback)
	{
		s_debugCallback = callback;
	}

	void RHIDebug::logDebug(const std::string& message)
	{
		if (s_debugCallback)
		{
			s_debugCallback("[DEBUG] " + message);
		}
		else
		{
			std::cout << "[DEBUG] " << message << std::endl;
		}
	}

	void RHIDebug::logInfo(const std::string& message)
	{
		if (s_debugCallback)
		{
			s_debugCallback("[INFO] " + message);
		}
		else
		{
			std::cout << "[INFO] " << message << std::endl;
		}
	}

	void RHIDebug::logWarning(const std::string& message)
	{
		if (s_debugCallback)
		{
			s_debugCallback("[WARNING] " + message);
		}
		else
		{
			std::cout << "[WARNING] " << message << std::endl;
		}
	}

	void RHIDebug::logError(const std::string& message)
	{
		if (s_debugCallback)
		{
			s_debugCallback("[ERROR] " + message);
		}
		else
		{
			std::cerr << "[ERROR] " << message << std::endl;
		}
	}

	const char* RHIDebug::getApiName(RHIApiType apiType)
	{
		switch (apiType)
		{
		case RHIApiType::Vulkan:  return "Vulkan";
		case RHIApiType::D3D12:   return "Direct3D 12";
		case RHIApiType::Metal:   return "Metal";
		case RHIApiType::OpenGL:  return "OpenGL";
		default:       return "Unknown";
		}
	}

	const char* RHIDebug::getFormatName(RHIFormat format)
	{
		switch (format)
		{
		case RHI_FORMAT_UNDEFINED: return "UNDEFINED";
		case RHI_FORMAT_R8G8B8A8_UNORM:   return "R8G8B8A8_UNORM";
		case RHI_FORMAT_B8G8R8A8_UNORM:   return "B8G8R8A8_UNORM";
		case RHI_FORMAT_D32_SFLOAT:   return "D32_SFLOAT";
		default:      return "UNKNOWN_FORMAT";
		}
	}

	std::string RHIDebug::getBufferUsageFlagsString(RHIBufferUsageFlags flags)
	{
		std::stringstream ss;
		bool first = true;

		auto addFlag = [&](RHIBufferUsageFlags flag, const char* name) {
			if (flags & flag)
			{
				if (!first) ss << " | ";
				ss << name;
				first = false;
			}
			};

		addFlag(RHI_BUFFER_USAGE_VERTEX_BUFFER_BIT, "VERTEX_BUFFER");
		addFlag(RHI_BUFFER_USAGE_INDEX_BUFFER_BIT, "INDEX_BUFFER");
		addFlag(RHI_BUFFER_USAGE_UNIFORM_BUFFER_BIT, "UNIFORM_BUFFER");
		addFlag(RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT, "STORAGE_BUFFER");
		addFlag(RHI_BUFFER_USAGE_TRANSFER_SRC_BIT, "TRANSFER_SRC");
		addFlag(RHI_BUFFER_USAGE_TRANSFER_DST_BIT, "TRANSFER_DST");

		return ss.str().empty() ? "NONE" : ss.str();
	}

	std::string RHIDebug::getImageUsageFlagsString(RHIImageUsageFlags flags)
	{
		std::stringstream ss;
		bool first = true;

		auto addFlag = [&](RHIImageUsageFlags flag, const char* name) {
			if (flags & flag)
			{
				if (!first) ss << " | ";
				ss << name;
				first = false;
			}
			};

		addFlag(RHI_IMAGE_USAGE_TRANSFER_SRC_BIT, "TRANSFER_SRC");
		addFlag(RHI_IMAGE_USAGE_TRANSFER_DST_BIT, "TRANSFER_DST");
		addFlag(RHI_IMAGE_USAGE_SAMPLED_BIT, "SAMPLED");
		addFlag(RHI_IMAGE_USAGE_STORAGE_BIT, "STORAGE");
		addFlag(RHI_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, "COLOR_ATTACHMENT");
		addFlag(RHI_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, "DEPTH_STENCIL_ATTACHMENT");

		return ss.str().empty() ? "NONE" : ss.str();
	}

	std::string RHIDebug::getShaderStageFlagsString(RHIShaderStageFlags flags)
	{
		std::stringstream ss;
		bool first = true;

		auto addFlag = [&](RHIShaderStageFlags flag, const char* name) {
			if (flags & flag)
			{
				if (!first) ss << " | ";
				ss << name;
				first = false;
			}
			};

		addFlag(RHI_SHADER_STAGE_VERTEX_BIT, "VERTEX");
		addFlag(RHI_SHADER_STAGE_FRAGMENT_BIT, "FRAGMENT");
		addFlag(RHI_SHADER_STAGE_COMPUTE_BIT, "COMPUTE");
		addFlag(RHI_SHADER_STAGE_GEOMETRY_BIT, "GEOMETRY");
		addFlag(RHI_SHADER_STAGE_TESSELLATION_CONTROL_BIT, "TESS_CONTROL");
		addFlag(RHI_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, "TESS_EVAL");

		return ss.str().empty() ? "NONE" : ss.str();
	}

	void RHIDebug::dumpBufferInfo(const RHIBufferCreateInfo& createInfo)
	{
		std::stringstream ss;
		ss << "Buffer CreateInfo:\n";
		ss << "  Size: " << createInfo.size << " bytes\n";
		ss << "  Usage: " << getBufferUsageFlagsString(createInfo.usage) << "\n";
		ss << "  Initial Data: " << (createInfo.initialData ? "Yes" : "No") << "\n";
		logInfo(ss.str());
	}

	void RHIDebug::dumpImageInfo(const RHIImageCreateInfo& createInfo)
	{
		std::stringstream ss;
		ss << "Image CreateInfo:\n";
		ss << "  Dimensions: " << createInfo.width << "x" << createInfo.height << "x" << createInfo.depth << "\n";
		ss << "  Format: " << getFormatName(createInfo.format) << "\n";
		ss << "  Mip Levels: " << createInfo.mipLevels << "\n";
		ss << "  Array Layers: " << createInfo.arrayLayers << "\n";
		ss << "  Samples: " << createInfo.samples << "\n";
		ss << "  Usage: " << getImageUsageFlagsString(createInfo.usage) << "\n";
		logInfo(ss.str());
	}

	void RHIDebug::dumpShaderInfo(const RHIShaderCreateInfo& createInfo)
	{
		std::stringstream ss;
		ss << "Shader CreateInfo:\n";
		ss << "  Stage: " << getShaderStageFlagsString(createInfo.stage) << "\n";
		ss << "  Entry Point: " << createInfo.entryPoint << "\n";
		ss << "  Code Size: " << createInfo.code.size() * sizeof(uint32_t) << " bytes\n";
		ss << "  Name: " << (createInfo.name.empty() ? "Unnamed" : createInfo.name) << "\n";
		logInfo(ss.str());
	}

	void RHIDebug::dumpPipelineInfo(const RHIPipelineCreateInfo& createInfo)
	{
		std::stringstream ss;
		ss << "Pipeline CreateInfo:\n";
		ss << "  Shader Stages: " << createInfo.shaderStages.size() << "\n";
		ss << "  Vertex Bindings: " << createInfo.vertexInputState.bindings.size() << "\n";
		ss << "  Vertex Attributes: " << createInfo.vertexInputState.attributes.size() << "\n";
		ss << "  Topology: " << static_cast<int>(createInfo.inputAssemblyState.topology) << "\n";
		ss << "  Cull Mode: " << createInfo.rasterizationState.cullMode << "\n";
		ss << "  Depth Test: " << (createInfo.depthStencilState.depthTestEnable ? "Enabled" : "Disabled") << "\n";
		logInfo(ss.str());
	}

} // namespace BinRenderer
