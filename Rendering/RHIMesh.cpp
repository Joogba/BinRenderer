#include "RHIMesh.h"
#include "../Core/Logger.h"

namespace BinRenderer
{
	RHIMesh::RHIMesh(RHI* rhi)
		: rhi_(rhi)
	{
	}

	RHIMesh::~RHIMesh()
	{
		destroyBuffers();
	}

	void RHIMesh::setVertices(const std::vector<RHIVertex>& vertices)
	{
		vertices_ = vertices;
	}

	void RHIMesh::setIndices(const std::vector<uint32_t>& indices)
	{
		indices_ = indices;
	}

	bool RHIMesh::createBuffers()
	{
		if (vertices_.empty() || indices_.empty())
		{
			printLog("Cannot create buffers: vertices or indices are empty");
			return false;
		}

		// Vertex Buffer
		RHIBufferCreateInfo vertexBufferInfo{};
		vertexBufferInfo.size = vertices_.size() * sizeof(RHIVertex);
		vertexBufferInfo.usage = RHI_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		vertexBufferInfo.memoryProperties = RHI_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

		vertexBuffer_ = rhi_->createBuffer(vertexBufferInfo);
		if (vertexBuffer_.isValid())
		{
			void* data = rhi_->mapBuffer(vertexBuffer_);
			memcpy(data, vertices_.data(), vertexBufferInfo.size);
			rhi_->unmapBuffer(vertexBuffer_);
		}
		else
		{
			printLog("Failed to create vertex buffer");
			return false;
		}

		// Index Buffer
		RHIBufferCreateInfo indexBufferInfo{};
		indexBufferInfo.size = indices_.size() * sizeof(uint32_t);
		indexBufferInfo.usage = RHI_BUFFER_USAGE_INDEX_BUFFER_BIT;
		indexBufferInfo.memoryProperties = RHI_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

		indexBuffer_ = rhi_->createBuffer(indexBufferInfo);
		if (indexBuffer_.isValid())
		{
			void* data = rhi_->mapBuffer(indexBuffer_);
			memcpy(data, indices_.data(), indexBufferInfo.size);
			rhi_->unmapBuffer(indexBuffer_);
		}
		else
		{
			printLog("Failed to create index buffer");
			destroyBuffers();
			return false;
		}

		return true;
	}

	void RHIMesh::destroyBuffers()
	{
		if (vertexBuffer_.isValid())
		{
			rhi_->destroyBuffer(vertexBuffer_);
			vertexBuffer_ = {};
		}

		if (indexBuffer_.isValid())
		{
			rhi_->destroyBuffer(indexBuffer_);
			indexBuffer_ = {};
		}
	}

	void RHIMesh::bind(RHI* rhi)
	{
		if (vertexBuffer_.isValid() && indexBuffer_.isValid())
		{
			rhi->cmdBindVertexBuffer(vertexBuffer_);
			rhi->cmdBindIndexBuffer(indexBuffer_);
		}
	}

	void RHIMesh::draw(RHI* rhi, uint32_t instanceCount)
	{
		if (indexBuffer_.isValid() && !indices_.empty())
		{
			rhi->cmdDrawIndexed(static_cast<uint32_t>(indices_.size()), instanceCount, 0, 0, 0);
		}
	}

} // namespace BinRenderer
