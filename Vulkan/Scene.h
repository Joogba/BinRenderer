#pragma once

#include "Camera.h"
#include "Model.h"

#include <functional>
#include <memory>
#include <string>
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
     * @brief 씬에 모델 추가
     * @param model 모델 인스턴스
     * @param name 노드 이름
     * @return 추가된 노드의 인덱스
     */
    size_t addModel(shared_ptr<Model> model, const string& name = "Unnamed")
{
        nodes_.emplace_back(std::move(model), name);
        return nodes_.size() - 1;
}
    
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
    // Utility
    // ========================================
    
    /**
     * @brief 모든 모델을 순회하며 함수 적용
     */
    void forEachModel(const std::function<void(Model&, const glm::mat4&)>& func)
 {
for (auto& node : nodes_) {
if (node.model && node.visible) {
     func(*node.model, node.transform);
            }
        }
    }
    
    /**
     * @brief 렌더링 가능한 모델만 필터링하여 반환
     */
    vector<Model*> getVisibleModels()
    {
      vector<Model*> visibleModels;
  for (auto& node : nodes_) {
            if (node.model && node.visible) {
        visibleModels.push_back(node.model.get());
      }
    }
        return visibleModels;
    }
    
    /**
     * @brief ✅ NEW: Renderer 호환을 위한 shared_ptr 벡터 반환
     */
    vector<shared_ptr<Model>> getModelSharedPtrs()
    {
     vector<shared_ptr<Model>> modelPtrs;
        for (auto& node : nodes_) {
      if (node.model && node.visible) {
modelPtrs.push_back(node.model);
            }
        }
   return modelPtrs;
    }
    
    /**
     * @brief Legacy compatibility: 모델 포인터 벡터 반환 (Renderer 호환성)
     * @deprecated getVisibleModels() 또는 getModelSharedPtrs() 사용 권장
     */
    vector<Model*> getModelPointers()
    {
        return getVisibleModels();
    }
    
  private:
    vector<SceneNode> nodes_;
    Camera camera_;
};

} // namespace BinRenderer::Vulkan
