#pragma once

#include "../RHI/Core/RHI.h"
#include "../RHI/Structs/RHIStructs.h"
#include "../Scene/Animation.h"
#include "../Rendering/RHIMesh.h"
#include "../Rendering/RHIMaterial.h"
#include "../Rendering/RHIVertex.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

namespace BinRenderer
{
	/**
	 * @brief 플랫폼 독립적 RHI Model
	 */
	class RHIModel
	{
	public:
		RHIModel(RHI* rhi);
		~RHIModel();

		// 모델 로딩
		bool loadFromFile(const std::string& filePath);

		// 렌더링
		void draw(RHI* rhi, uint32_t instanceCount = 1);

		// Animation 지원
		void setAnimation(std::unique_ptr<Animation> animation);
		Animation* getAnimation() const { return animation_.get(); }
		bool hasAnimation() const { return animation_ != nullptr; }

		// Transform
		void setTransform(const glm::mat4& transform) { transform_ = transform; }
		const glm::mat4& getTransform() const { return transform_; }

		// 메시/머티리얼 접근
		const std::vector<std::unique_ptr<RHIMesh>>& getMeshes() const { return meshes_; }
		const std::vector<RHIMaterial>& getMaterials() const { return materials_; }

		// Name
		const std::string& getName() const { return name_; }
		void setName(const std::string& name) { name_ = name; }

		// ========================================
		//  GPU Instancing 지원
		// ========================================

		/**
		 * @brief 인스턴스 추가
		 * @param instanceData Per-instance data (transform, material offset)
		 */
		void addInstance(const InstanceData& instanceData);

		/**
		 * @brief 인스턴스 업데이트
		 * @param index Instance index
		 * @param instanceData Updated data
		 */
		void updateInstance(uint32_t index, const InstanceData& instanceData);

		/**
		 * @brief 인스턴스 제거
		 */
		void removeInstance(uint32_t index);

		/**
		 * @brief 모든 인스턴스 제거
		 */
		void clearInstances();

		/**
		 * @brief 인스턴스 개수 반환
		 */
		uint32_t getInstanceCount() const { return static_cast<uint32_t>(instances_.size()); }

		/**
		 * @brief 인스턴스 데이터 반환
		 */
		const std::vector<InstanceData>& getInstances() const { return instances_; }

		/**
		 * @brief Instancing 활성화 여부 (1개 이상이면 활성화)
		 */
		bool isInstanced() const { return instances_.size() >= 1; }

		/**
		 * @brief Instance buffer 반환
		 */
		RHIBufferHandle getInstanceBuffer() const { return instanceBuffer_; }

		/**
		 * @brief Instance buffer 업데이트 (GPU로 전송)
		 */
		void updateInstanceBuffer();

	private:
		void createBuffers();
		void destroyBuffers();
		void createInstanceBuffer();
		void destroyInstanceBuffer();

		RHI* rhi_;
		std::string filePath_;
		std::string name_;
		
		std::vector<std::unique_ptr<RHIMesh>> meshes_;
		std::vector<RHIMaterial> materials_;
		
		std::unique_ptr<Animation> animation_;
		glm::mat4 transform_ = glm::mat4(1.0f);

		//  GPU Instancing
		std::vector<InstanceData> instances_;
		RHIBufferHandle instanceBuffer_;
	};

} // namespace BinRenderer
