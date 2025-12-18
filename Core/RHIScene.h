#pragma once

#include "../RHI/Core/RHI.h"
#include "RHIModel.h"
#include "../Scene/Animation.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace BinRenderer
{
	/**
	 * @brief Scene 내 모델 인스턴스
	 */
	struct RHISceneNode
	{
		std::shared_ptr<RHIModel> model;
		std::string name;
		glm::mat4 transform = glm::mat4(1.0f);
		bool visible = true;
	};

	/**
	 * @brief RHI 기반 Scene 관리
	 */
	class RHIScene
	{
	public:
		RHIScene(RHI* rhi);
		~RHIScene();

		// 모델 추가
		void addModel(const std::string& resourcePath, const std::string& name, const glm::mat4& transform);
		
		// 모델 인스턴스 추가 (GPU 인스턴싱)
		void addModelInstance(const std::string& resourcePath, const std::string& instanceName, const glm::mat4& transform);

		// 노드 접근
		std::vector<RHISceneNode>& getNodes() { return nodes_; }
		const std::vector<RHISceneNode>& getNodes() const { return nodes_; }

		// 업데이트
		void update(float deltaTime);

	private:
		std::shared_ptr<RHIModel> loadOrGetModel(const std::string& resourcePath);

		RHI* rhi_;
		std::vector<RHISceneNode> nodes_;
		std::unordered_map<std::string, std::shared_ptr<RHIModel>> modelCache_;
	};

} // namespace BinRenderer
