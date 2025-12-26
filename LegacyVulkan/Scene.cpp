#include "Scene.h"
#include "Logger.h"
#include "VulkanResourceManager.h"

namespace BinRenderer::Vulkan {

	void Scene::addModel(shared_ptr<Model> model, const string& name)
	{
		SceneNode node(model, name);
		nodes_.push_back(node);
	}

	shared_ptr<Model> Scene::loadOrGetModel(const string& resourcePath)
	{
		// VulkanResourceManager 사용 (주입받지 않았으면 경고)
		if (!vulkanResourceManager_) {
			printLog("❌ ERROR: VulkanResourceManager not set in Scene!");
			return nullptr;
		}

		// VulkanResourceManager에 위임
		return vulkanResourceManager_->LoadOrGetModel(resourcePath);
	}

	bool Scene::addModelInstance(const string& resourcePath,
		const string& instanceName,
		const glm::mat4& transform)
	{
		// ========================================
		//  VulkanResourceManager를 통한 캐싱 + GPU Instancing
		// ========================================
		
		// 1. 캐시된 모델 확인 (VulkanResourceManager가 Context를 가지고 있음)
		shared_ptr<Model> cachedModel = loadOrGetModel(resourcePath);
		
		if (!cachedModel) {
			printLog("❌ Failed to load model: {}", resourcePath);
			return false;
		}
		
		// 2. 첫 번째 인스턴스인지 확인
		bool isFirstInstance = (cachedModel->getInstanceCount() == 0);
		
		// 3. GPU Instancing에 transform 추가
		cachedModel->addInstance(transform);
		
		// 4. 각 인스턴스마다 SceneNode 생성
		SceneNode node;
		node.model = cachedModel;
		node.name = instanceName;
		node.transform = transform;
		node.visible = true;
		
		nodes_.push_back(node);
		
		if (isFirstInstance) {
			printLog("📦 First instance of model: '{}'", instanceName);
			printLog("   Model cached at: {}", resourcePath);
		} else {
			printLog(" Added instance #{}: '{}'", cachedModel->getInstanceCount(), instanceName);
			printLog("   Transform: ({:.2f}, {:.2f}, {:.2f})", 
				transform[3][0], transform[3][1], transform[3][2]);
		}
		
		printLog("   Total instances of this model: {}", cachedModel->getInstanceCount());
		printLog("   Total scene nodes: {}", nodes_.size());
		
		return true;
	}

} // namespace BinRenderer::Vulkan
