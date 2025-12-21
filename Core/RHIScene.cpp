#include "RHIScene.h"
#include "Logger.h"

namespace BinRenderer
{
	RHIScene::RHIScene(RHI* rhi)
		: rhi_(rhi)
	{
	}

	RHIScene::~RHIScene()
	{
		clear();
	}

	// ========================================
	// 모델 관리 (레거시 기능 통합)
	// ========================================

	void RHIScene::addModel(std::shared_ptr<RHIModel> model, const std::string& name)
	{
		if (!model)
		{
			printLog("ERROR: Cannot add null model");
			return;
		}

		RHISceneNode node;
		node.model = model;
		node.name = name;
		node.transform = glm::mat4(1.0f);
		node.visible = true;

		nodes_.push_back(node);

		printLog("✅ RHIScene::addModel - {}", name);
	}

	void RHIScene::addModel(const std::string& resourcePath, const std::string& name, const glm::mat4& transform)
	{
		auto model = loadOrGetModel(resourcePath);
		if (!model)
		{
			printLog("ERROR: Failed to load model: {}", resourcePath);
			return;
		}

		RHISceneNode node;
		node.model = model;
		node.name = name;
		node.transform = transform;
		node.visible = true;

		nodes_.push_back(node);

		printLog("✅ RHIScene::addModel - {} ({})", name, resourcePath);
	}

	bool RHIScene::addModelInstance(const std::string& resourcePath, const std::string& instanceName, const glm::mat4& transform)
	{
		auto model = loadOrGetModel(resourcePath);
		if (!model)
		{
			printLog("ERROR: Failed to load model for instance: {}", resourcePath);
			return false;
		}

		RHISceneNode node;
		node.model = model;
		node.name = instanceName;
		node.transform = transform;
		node.visible = true;

		nodes_.push_back(node);

		printLog("✅ RHIScene::addModelInstance - {} (cached: {})", instanceName, resourcePath);
		return true;
	}

	std::shared_ptr<RHIModel> RHIScene::loadOrGetModel(const std::string& resourcePath)
	{
		// 캐시 확인
		auto it = modelCache_.find(resourcePath);
		if (it != modelCache_.end())
		{
			printLog("📦 Using cached model: {}", resourcePath);
			return it->second;
		}

		// 새로 로드
		auto model = std::make_shared<RHIModel>(rhi_);
		
		// TODO: RHIModel::loadFromFile 구현 필요
		// if (!model->loadFromFile(resourcePath))
		// {
		// 	printLog("ERROR: Failed to load model file: {}", resourcePath);
		// 	return nullptr;
		// }

		modelCache_[resourcePath] = model;
		printLog("✅ Loaded and cached model: {}", resourcePath);
		return model;
	}

	// ========================================
	// RHIRenderer와 호환 (Model* 포인터 벡터)
	// ========================================

	std::vector<RHIModel*> RHIScene::getModels()
	{
		std::vector<RHIModel*> models;
		models.reserve(nodes_.size());

		for (auto& node : nodes_)
		{
			if (node.model && node.visible)
			{
				models.push_back(node.model.get());
			}
		}

		return models;
	}

	void RHIScene::clear()
	{
		nodes_.clear();
		// modelCache_는 shared_ptr이므로 자동 정리됨
		printLog("🗑️ RHIScene cleared");
	}

	// ========================================
	// 업데이트
	// ========================================

	void RHIScene::update(float deltaTime)
	{
		// 카메라 업데이트
		camera_.update(deltaTime);

		// 모든 노드의 애니메이션 업데이트
		for (auto& node : nodes_)
		{
			if (node.model && node.model->hasAnimation())
			{
				node.model->getAnimation()->updateAnimation(deltaTime);
			}
		}
	}

} // namespace BinRenderer
