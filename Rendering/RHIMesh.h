#pragma once

#include "../RHI/Core/RHI.h"
#include "RHIVertex.h"
#include <vector>
#include <string>

namespace BinRenderer
{
	/**
	 * @brief RHI 기반 메시
	 */
	class RHIMesh
	{
	public:
		RHIMesh(RHI* rhi);
		~RHIMesh();

		// 메시 데이터 설정
		void setVertices(const std::vector<RHIVertex>& vertices);
		void setIndices(const std::vector<uint32_t>& indices);

		// GPU 버퍼 생성
		bool createBuffers();
		void destroyBuffers();

		// 렌더링
		void bind(RHI* rhi);
		void draw(RHI* rhi, uint32_t instanceCount = 1);

		// 정보
		uint32_t getVertexCount() const { return static_cast<uint32_t>(vertices_.size()); }
		uint32_t getIndexCount() const { return static_cast<uint32_t>(indices_.size()); }

		// Material index
		uint32_t getMaterialIndex() const { return materialIndex_; }
		void setMaterialIndex(uint32_t index) { materialIndex_ = index; }

		// Name
		const std::string& getName() const { return name_; }
		void setName(const std::string& name) { name_ = name; }

	private:
		RHI* rhi_;
		
		std::vector<RHIVertex> vertices_;
		std::vector<uint32_t> indices_;

		RHIBufferHandle vertexBuffer_;
		RHIBufferHandle indexBuffer_;

		uint32_t materialIndex_ = 0;
		std::string name_;
	};

} // namespace BinRenderer
