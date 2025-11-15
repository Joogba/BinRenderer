#pragma once

#include "Camera.h"
#include "Model.h"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>

namespace BinRenderer::Vulkan {

	using namespace std;

	/**
	 * @brief 씬 내 모델 인스턴스 정보
	 */
	struct SceneNode
	{
		shared_ptr<Model> model = nullptr;  // ✅ unique_ptr → shared_ptr
		glm::mat4 transform = glm::mat4(1.0f);
		string name = "Unnamed";
		bool visible = true;

		SceneNode() = default;
		SceneNode(shared_ptr<Model> m, const string& n = "Unnamed")
			: model(std::move(m)), name(n)
		{
		}
	};

	/**
	 * @brief 씬 관리 클래스
	 *
	 * 모델, 카메라, 라이트 등 씬 데이터를 관리합니다.
	 * Application에서 모델 관리 책임을 분리합니다.
	 */
	class Scene
	{
	public:
		Scene() = default;
		~Scene() = default;

		// ========================================
		// Model Management
		// ========================================

		/**
		 * @brief 씬에 모델 추가 (새 로드)
		 */
		void addModel(shared_ptr<Model> model, const string& name);

		/**
		 * @brief 캐시된 모델의 인스턴스 추가 (Transform만 다름)
		 * @param resourcePath 모델 파일 경로 (캐시 키)
		 * @param instanceName 인스턴스 이름
		 * @param transform 인스턴스별 Transform
		 * @param ctx Vulkan Context (모델 로드 시 필요)
		 * @return 성공 여부
	   */
		bool addModelInstance(const string& resourcePath,
			const string& instanceName,
			const glm::mat4& transform,
			Context& ctx);

		/**
		 * @brief 모델 리소스 미리 로드 (캐싱)
		 * @param resourcePath 모델 파일 경로
		 * @param ctx Vulkan Context
	 * @return 캐시된 모델의 shared_ptr
		 */
		shared_ptr<Model> loadOrGetModel(const string& resourcePath, Context& ctx);

		/**
		 * @brief 인덱스로 노드 접근
		 */
		SceneNode& getNode(size_t index)
		{
			return nodes_[index];
		}

		const SceneNode& getNode(size_t index) const
		{
			return nodes_[index];
		}

		/**
	 * @brief 모든 노드 접근
		 */
		vector<SceneNode>& getNodes()
		{
			return nodes_;
		}

		const vector<SceneNode>& getNodes() const
		{
			return nodes_;
		}

		/**
		 * @brief 노드 개수 반환
		 */
		size_t getNodeCount() const
		{
			return nodes_.size();
		}

		/**
		 * @brief 모든 노드 제거
		 */
		void clear()
		{
			nodes_.clear();
		}

		// ========================================
		// Camera Management
		// ========================================

		/**
		 * @brief 씬의 메인 카메라 설정
		 */
		void setCamera(const Camera& camera)
		{
			camera_ = camera;
		}

		Camera& getCamera()
		{
			return camera_;
		}

		const Camera& getCamera() const
		{
			return camera_;
		}

		// ========================================
		// Cleanup
		// ========================================

		void cleanup()
		{
			nodes_.clear();
			modelCache_.clear();
		}

	private:
		vector<SceneNode> nodes_;           // 모든 씬 노드
		unordered_map<string, shared_ptr<Model>> modelCache_;  // 모델 캐시 (리소스 경로 → Model)
		Camera camera_;
	};

} // namespace BinRenderer::Vulkan
