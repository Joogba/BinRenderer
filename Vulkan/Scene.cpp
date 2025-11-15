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
		// ✅ GPU Instancing: Step B - 자동 인스턴싱
		// ========================================
		
		// 1. 캐시된 모델 확인 (GPU Instancing용)
		shared_ptr<Model> cachedModel = loadOrGetModel(resourcePath, ctx);
		
		if (!cachedModel) {
			printLog("❌ Failed to load model: {}", resourcePath);
			return false;
		}
		
		// 2. 이미 같은 모델이 Scene에 있는지 확인
		SceneNode* existingNode = nullptr;
		for (auto& node : nodes_) {
			if (node.model == cachedModel) {
				existingNode = &node;
				break;
			}
		}
		
		// 3. 기존 노드가 있으면 인스턴스 추가
		if (existingNode) {
			printLog("✅ Found existing model, adding as instance #{}", 
				cachedModel->getInstanceCount());
			
			cachedModel->addInstance(transform);
			
			printLog("✅ Added instance '{}' at ({:.2f}, {:.2f}, {:.2f}) - Total: {} instances",
				instanceName, transform[3][0], transform[3][1], transform[3][2],
				cachedModel->getInstanceCount());
			
			return true;
		}
		
		// 4. 새 모델이면 첫 번째 인스턴스로 추가
		printLog("📦 First instance of model, creating new node");
		
		cachedModel->addInstance(transform);
		cachedModel->name() = instanceName;
		
		SceneNode node;
		node.model = cachedModel;
		node.name = instanceName;
		node.transform = transform;
		node.visible = true;
		
		nodes_.push_back(node);
		
		printLog("✅ Added first instance '{}' at ({:.2f}, {:.2f}, {:.2f})",
			instanceName, transform[3][0], transform[3][1], transform[3][2]);
		
		return true;
	}

} // namespace BinRenderer::Vulkan
