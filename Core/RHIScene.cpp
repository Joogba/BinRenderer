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
	}

	void RHIScene::addModel(const std::string& resourcePath, const std::string& name, const glm::mat4& transform)
	{
		auto model = loadOrGetModel(resourcePath);
		if (!model)
		{
			printLog("Failed to load model: {}", resourcePath);
			return;
		}

		RHISceneNode node;
		node.model = model;
		node.name = name;
		node.transform = transform;
		node.visible = true;

		nodes_.push_back(node);

		printLog("RHIScene::addModel - {} ({})", name, resourcePath);
	}

	void RHIScene::addModelInstance(const std::string& resourcePath, const std::string& instanceName, const glm::mat4& transform)
	{
		auto model = loadOrGetModel(resourcePath);
		if (!model)
		{
			printLog("Failed to load model for instance: {}", resourcePath);
			return;
		}

		RHISceneNode node;
		node.model = model;
		node.name = instanceName;
		node.transform = transform;
		node.visible = true;

		nodes_.push_back(node);

		printLog("RHIScene::addModelInstance - {} (cached: {})", instanceName, resourcePath);
	}

	std::shared_ptr<RHIModel> RHIScene::loadOrGetModel(const std::string& resourcePath)
	{
		// 캐시 확인
		auto it = modelCache_.find(resourcePath);
		if (it != modelCache_.end())
		{
			printLog("Using cached model: {}", resourcePath);
			return it->second;
		}

		// 새로 로드
		auto model = std::make_shared<RHIModel>(rhi_);
		if (!model->loadFromFile(resourcePath))
		{
			return nullptr;
		}

		modelCache_[resourcePath] = model;
		return model;
	}

	void RHIScene::update(float deltaTime)
	{
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
