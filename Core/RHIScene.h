#pragma once

#include "../RHI/Core/RHI.h"
#include "RHIModel.h"
#include "../Scene/RHICamera.h"
#include "../Scene/Animation.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace BinRenderer
{
	/**
	 * @brief Scene 내 모델 인스턴스 (레거시 SceneNode와 호환)
	 */
	struct RHISceneNode
	{
		std::shared_ptr<RHIModel> model = nullptr;
		std::string name = "Unnamed";
		glm::mat4 transform = glm::mat4(1.0f);
		bool visible = true;

		RHISceneNode() = default;
		RHISceneNode(std::shared_ptr<RHIModel> m, const std::string& n = "Unnamed")
			: model(std::move(m)), name(n)
		{
		}
	};

	/**
	 * @brief RHI 기반 Scene 관리 (레거시 Vulkan/Scene 기능 통합)
	 */
	class RHIScene
	{
	public:
		RHIScene(RHI* rhi);
		~RHIScene();

		// ========================================
		// 모델 관리 (레거시 기능 통합)
		// ========================================

		/**
		 * @brief 씬에 모델 추가 (새 로드)
		 */
		void addModel(std::shared_ptr<RHIModel> model, const std::string& name);

		/**
		 * @brief 모델 파일에서 로드하여 추가
		 */
		void addModel(const std::string& resourcePath, const std::string& name, const glm::mat4& transform);
		
		/**
		 * @brief 캐시된 모델의 인스턴스 추가 (GPU 인스턴싱)
		 * @param resourcePath 모델 파일 경로 (캐시 키)
		 * @param instanceName 인스턴스 이름
		 * @param transform 인스턴스별 Transform
		 * @return 성공 여부
		 */
		bool addModelInstance(const std::string& resourcePath, const std::string& instanceName, const glm::mat4& transform);

		/**
		 * @brief 모델 리소스 미리 로드 (캐싱)
		 */
		std::shared_ptr<RHIModel> loadOrGetModel(const std::string& resourcePath);

		// ========================================
		// 노드 접근
		// ========================================

		/**
		 * @brief 인덱스로 노드 접근
		 */
		RHISceneNode& getNode(size_t index) { return nodes_[index]; }
		const RHISceneNode& getNode(size_t index) const { return nodes_[index]; }

		/**
		 * @brief 모든 노드 접근
		 */
		std::vector<RHISceneNode>& getNodes() { return nodes_; }
		const std::vector<RHISceneNode>& getNodes() const { return nodes_; }

		/**
		 * @brief RHIRenderer와 호환 (Model* 포인터 벡터)
		 */
		std::vector<RHIModel*> getModels();

		/**
		 * @brief 노드 개수 반환
		 */
		size_t getNodeCount() const { return nodes_.size(); }

		/**
		 * @brief 모든 노드 제거
		 */
		void clear();

		// ========================================
		// 카메라 관리
		// ========================================

		/**
		 * @brief 씬의 메인 카메라 설정
		 */
		void setCamera(const RHICamera& camera) { camera_ = camera; }
		RHICamera& getCamera() { return camera_; }
		const RHICamera& getCamera() const { return camera_; }

		// ========================================
		// 업데이트
		// ========================================

		/**
		 * @brief 씬 업데이트 (애니메이션 등)
		 */
		void update(float deltaTime);

	private:
		RHI* rhi_;
		std::vector<RHISceneNode> nodes_;
		std::unordered_map<std::string, std::shared_ptr<RHIModel>> modelCache_;
		RHICamera camera_;
	};

} // namespace BinRenderer
