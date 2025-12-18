#include "RGBuilder.h"
#include "../RGPassBase.h"
#include <algorithm>

namespace BinRenderer
{
	// ========================================
	// 텍스처 관리
	// ========================================

	RGTextureHandle RenderGraphBuilder::createTexture(const RGTextureDesc& desc)
	{
		RGTextureHandle handle;
		handle.index = static_cast<uint32_t>(textures_.size());

		TextureNode node;
		node.desc = desc;
		textures_.push_back(node);

		return handle;
	}

	RGTextureHandle RenderGraphBuilder::importTexture(const std::string& name, RHIImage* image, const RGTextureDesc& desc)
	{
		RGTextureHandle handle;
		handle.index = static_cast<uint32_t>(textures_.size());

		TextureNode node;
		node.desc = desc;
		node.desc.name = name;
		node.desc.isImported = true;
		node.importedImage = image;
		textures_.push_back(node);

		return handle;
	}

	RGTextureHandle RenderGraphBuilder::readTexture(RGTextureHandle handle)
	{
		if (!handle.isValid() || handle.index >= textures_.size()) {
			return handle;
		}

		addTextureDependency(handle, RGResourceAccessType::Read);
		
		auto& node = textures_[handle.index];
		node.isRead = true;
		node.firstUse = std::min(node.firstUse, currentPassIndex_);
		node.lastUse = std::max(node.lastUse, currentPassIndex_);

		return handle;
	}

	RGTextureHandle RenderGraphBuilder::writeTexture(RGTextureHandle handle)
	{
		if (!handle.isValid() || handle.index >= textures_.size()) {
			return handle;
		}

		addTextureDependency(handle, RGResourceAccessType::Write);
		
		auto& node = textures_[handle.index];
		node.isWritten = true;
		node.firstUse = std::min(node.firstUse, currentPassIndex_);
		node.lastUse = std::max(node.lastUse, currentPassIndex_);

		return handle;
	}

	RGTextureHandle RenderGraphBuilder::readWriteTexture(RGTextureHandle handle)
	{
		if (!handle.isValid() || handle.index >= textures_.size()) {
			return handle;
		}

		addTextureDependency(handle, RGResourceAccessType::ReadWrite);
		
		auto& node = textures_[handle.index];
		node.isRead = true;
		node.isWritten = true;
		node.firstUse = std::min(node.firstUse, currentPassIndex_);
		node.lastUse = std::max(node.lastUse, currentPassIndex_);

		return handle;
	}

	// ========================================
	// 버퍼 관리
	// ========================================

	RGBufferHandle RenderGraphBuilder::createBuffer(const RGBufferDesc& desc)
	{
		RGBufferHandle handle;
		handle.index = static_cast<uint32_t>(buffers_.size());

		BufferNode node;
		node.desc = desc;
		buffers_.push_back(node);

		return handle;
	}

	RGBufferHandle RenderGraphBuilder::importBuffer(const std::string& name, RHIBuffer* buffer, const RGBufferDesc& desc)
	{
		RGBufferHandle handle;
		handle.index = static_cast<uint32_t>(buffers_.size());

		BufferNode node;
		node.desc = desc;
		node.desc.name = name;
		node.desc.isImported = true;
		node.importedBuffer = buffer;
		buffers_.push_back(node);

		return handle;
	}

	RGBufferHandle RenderGraphBuilder::readBuffer(RGBufferHandle handle)
	{
		if (!handle.isValid() || handle.index >= buffers_.size()) {
			return handle;
		}

		addBufferDependency(handle, RGResourceAccessType::Read);
		
		auto& node = buffers_[handle.index];
		node.isRead = true;
		node.firstUse = std::min(node.firstUse, currentPassIndex_);
		node.lastUse = std::max(node.lastUse, currentPassIndex_);

		return handle;
	}

	RGBufferHandle RenderGraphBuilder::writeBuffer(RGBufferHandle handle)
	{
		if (!handle.isValid() || handle.index >= buffers_.size()) {
			return handle;
		}

		addBufferDependency(handle, RGResourceAccessType::Write);
		
		auto& node = buffers_[handle.index];
		node.isWritten = true;
		node.firstUse = std::min(node.firstUse, currentPassIndex_);
		node.lastUse = std::max(node.lastUse, currentPassIndex_);

		return handle;
	}

	RGBufferHandle RenderGraphBuilder::readWriteBuffer(RGBufferHandle handle)
	{
		if (!handle.isValid() || handle.index >= buffers_.size()) {
			return handle;
		}

		addBufferDependency(handle, RGResourceAccessType::ReadWrite);
		
		auto& node = buffers_[handle.index];
		node.isRead = true;
		node.isWritten = true;
		node.firstUse = std::min(node.firstUse, currentPassIndex_);
		node.lastUse = std::max(node.lastUse, currentPassIndex_);

		return handle;
	}

	// ========================================
	// 최종 출력
	// ========================================

	void RenderGraphBuilder::setFinalOutput(RGTextureHandle handle)
	{
		finalOutput_ = handle;
	}

	// ========================================
	// 헬퍼 함수
	// ========================================

	void RenderGraphBuilder::addTextureDependency(RGTextureHandle handle, RGResourceAccessType accessType)
	{
		if (!currentPass_) {
			return;
		}

		RGResourceDependency dep;
		dep.texture = handle;
		dep.accessType = accessType;
		dep.isTexture = true;
		
		// ✅ public 메서드 사용
		currentPass_->addDependency(dep);
	}

	void RenderGraphBuilder::addBufferDependency(RGBufferHandle handle, RGResourceAccessType accessType)
	{
		if (!currentPass_) {
			return;
		}

		RGResourceDependency dep;
		dep.buffer = handle;
		dep.accessType = accessType;
		dep.isTexture = false;
		
		// ✅ public 메서드 사용
		currentPass_->addDependency(dep);
	}

} // namespace BinRenderer
