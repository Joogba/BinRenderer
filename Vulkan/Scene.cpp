#include "Scene.h"
#include "Logger.h"

namespace BinRenderer::Vulkan {

	void Scene::addModel(shared_ptr<Model> model, const string& name)
	{
		SceneNode node(model, name);
		nodes_.push_back(node);
	}

	shared_ptr<Model> Scene::loadOrGetModel(const string& resourcePath, Context& ctx)
	{
		// 캐시 확인
		if (modelCache_.find(resourcePath) != modelCache_.end()) {
			printLog("✅ Model cache HIT: {}", resourcePath);
			return modelCache_[resourcePath];
		}

		// 캐시 미스 - 새로 로드
		printLog("📦 Loading model: {}", resourcePath);
		auto model = std::make_shared<Model>(ctx);
		model->loadFromModelFile(resourcePath, false);
		
		// 캐시에 저장
		modelCache_[resourcePath] = model;
		
		return model;
	}

	bool Scene::addModelInstance(const string& resourcePath,
		const string& instanceName,
		const glm::mat4& transform,
		Context& ctx)
	{
		// ========================================
		// ✅ FIX: 각 인스턴스마다 개별 노드 생성 (Transform 적용)
		// ========================================
		
		// 1. 캐시된 모델 확인 (메모리 절약)
		shared_ptr<Model> cachedModel = loadOrGetModel(resourcePath, ctx);
		
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
			printLog("✅ Added instance #{}: '{}'", cachedModel->getInstanceCount(), instanceName);
			printLog("   Transform: ({:.2f}, {:.2f}, {:.2f})", 
				transform[3][0], transform[3][1], transform[3][2]);
		}
		
		printLog("   Total instances of this model: {}", cachedModel->getInstanceCount());
		printLog("   Total scene nodes: {}", nodes_.size());
		
		return true;
	}

} // namespace BinRenderer::Vulkan
