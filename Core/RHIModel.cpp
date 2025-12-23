#include "RHIModel.h"
#include "Logger.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace BinRenderer
{
	RHIModel::RHIModel(RHI* rhi)
		: rhi_(rhi)
	{
	}

	RHIModel::~RHIModel()
	{
		destroyBuffers();
	}

	bool RHIModel::loadFromFile(const std::string& filePath)
	{
		filePath_ = filePath;

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filePath,
			aiProcess_Triangulate |
			aiProcess_GenSmoothNormals |
			aiProcess_CalcTangentSpace |
			aiProcess_FlipUVs);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			printLog("Failed to load model: {}", filePath);
			return false;
		}

		printLog("Loading model: {}", filePath);
		printLog("  Meshes: {}", scene->mNumMeshes);
		printLog("  Materials: {}", scene->mNumMaterials);
		printLog("  Animations: {}", scene->mNumAnimations);

		// 메시 로드
		meshes_.reserve(scene->mNumMeshes);
		for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
		{
			const aiMesh* aiMesh = scene->mMeshes[i];
			auto mesh = std::make_unique<RHIMesh>(rhi_);

			// 정점 데이터
			std::vector<RHIVertex> vertices;
			vertices.resize(aiMesh->mNumVertices);
			for (uint32_t j = 0; j < aiMesh->mNumVertices; ++j)
			{
				RHIVertex& vertex = vertices[j];
				
				vertex.setPosition(glm::vec3(
					aiMesh->mVertices[j].x,
					aiMesh->mVertices[j].y,
					aiMesh->mVertices[j].z
				));

				if (aiMesh->HasNormals())
				{
					vertex.setNormal(glm::vec3(
						aiMesh->mNormals[j].x,
						aiMesh->mNormals[j].y,
						aiMesh->mNormals[j].z
					));
				}

				if (aiMesh->HasTextureCoords(0))
				{
					vertex.setTexCoord(glm::vec2(
						aiMesh->mTextureCoords[0][j].x,
						aiMesh->mTextureCoords[0][j].y
					));
				}

				if (aiMesh->HasTangentsAndBitangents())
				{
					vertex.setTangent(glm::vec3(
						aiMesh->mTangents[j].x,
						aiMesh->mTangents[j].y,
						aiMesh->mTangents[j].z
					));
					vertex.setBitangent(glm::vec3(
						aiMesh->mBitangents[j].x,
						aiMesh->mBitangents[j].y,
						aiMesh->mBitangents[j].z
					));
				}
			}

			// 인덱스 데이터
			std::vector<uint32_t> indices;
			for (uint32_t j = 0; j < aiMesh->mNumFaces; ++j)
			{
				const aiFace& face = aiMesh->mFaces[j];
				for (uint32_t k = 0; k < face.mNumIndices; ++k)
				{
					indices.push_back(face.mIndices[k]);
				}
			}

			mesh->setVertices(vertices);
			mesh->setIndices(indices);
			mesh->setMaterialIndex(aiMesh->mMaterialIndex);
			mesh->setName(aiMesh->mName.C_Str());

			// GPU 버퍼 생성
			if (!mesh->createBuffers())
			{
				printLog("Failed to create buffers for mesh: {}", mesh->getName());
				continue;
			}

			meshes_.push_back(std::move(mesh));
		}

		// 머티리얼 로드
		materials_.resize(scene->mNumMaterials);
		for (uint32_t i = 0; i < scene->mNumMaterials; ++i)
		{
			// TODO: 실제 머티리얼 로딩
			materials_[i].getData().baseColorFactor = glm::vec4(1.0f);
		}

		// Animation 로드
		if (scene->mNumAnimations > 0)
		{
			animation_ = std::make_unique<Animation>();
			animation_->loadFromScene(scene);
			printLog("  Animation loaded: {}", animation_->getCurrentAnimationName());
		}

		printLog("Model loaded successfully: {}", filePath);
		return true;
	}

	void RHIModel::createBuffers()
	{
		// Meshes already create their own buffers
	}

	void RHIModel::destroyBuffers()
	{
		// Meshes handle their own cleanup
		meshes_.clear();
		
		// ✅ GPU Instancing: instance buffer 정리
		destroyInstanceBuffer();
	}

	void RHIModel::draw(RHI* rhi, uint32_t instanceCount)
	{
		// ✅ GPU Instancing: instance buffer가 있으면 바인딩
		if (isInstanced() && instanceBuffer_)
		{
			rhi->cmdBindVertexBuffer(instanceBuffer_, 0);
		}

		for (const auto& mesh : meshes_)
		{
			mesh->bind(rhi);
			
			// ✅ GPU Instancing: 인스턴스 개수 전달
			uint32_t drawInstanceCount = isInstanced() ? getInstanceCount() : instanceCount;
			mesh->draw(rhi, drawInstanceCount);
		}
	}

	void RHIModel::setAnimation(std::unique_ptr<Animation> animation)
	{
		animation_ = std::move(animation);
	}

	// ========================================
	// ✅ GPU Instancing 구현
	// ========================================

	void RHIModel::addInstance(const InstanceData& instanceData)
	{
		instances_.push_back(instanceData);
		
		// Instance buffer 재생성 (크기 변경)
		if (instanceBuffer_)
		{
			destroyInstanceBuffer();
		}
		createInstanceBuffer();
		updateInstanceBuffer();
	}

	void RHIModel::updateInstance(uint32_t index, const InstanceData& instanceData)
	{
		if (index >= instances_.size())
		{
			printLog("ERROR: Invalid instance index {}", index);
			return;
		}

		instances_[index] = instanceData;
		updateInstanceBuffer();
	}

	void RHIModel::removeInstance(uint32_t index)
	{
		if (index >= instances_.size())
		{
			printLog("ERROR: Invalid instance index {}", index);
			return;
		}

		instances_.erase(instances_.begin() + index);

		if (instances_.empty())
		{
			destroyInstanceBuffer();
		}
		else
		{
			// Buffer 재생성
			destroyInstanceBuffer();
			createInstanceBuffer();
			updateInstanceBuffer();
		}
	}

	void RHIModel::clearInstances()
	{
		instances_.clear();
		destroyInstanceBuffer();
	}

	void RHIModel::updateInstanceBuffer()
	{
		if (!instanceBuffer_ || instances_.empty())
		{
			return;
		}

		// GPU로 데이터 전송
		void* data = rhi_->mapBuffer(instanceBuffer_);
		if (data)
		{
			memcpy(data, instances_.data(), instances_.size() * sizeof(InstanceData));
			rhi_->unmapBuffer(instanceBuffer_);
		}
	}

	void RHIModel::createInstanceBuffer()
	{
		if (instances_.empty())
		{
			return;
		}

		RHIBufferCreateInfo bufferInfo{};
		bufferInfo.size = instances_.size() * sizeof(InstanceData);
		bufferInfo.usage = RHI_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferInfo.memoryProperties = RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		instanceBuffer_ = rhi_->createBuffer(bufferInfo);
		if (!instanceBuffer_)
		{
			printLog("ERROR: Failed to create instance buffer");
			return;
		}

		printLog("✅ Instance buffer created: {} instances", instances_.size());
	}

	void RHIModel::destroyInstanceBuffer()
	{
		if (instanceBuffer_)
		{
			rhi_->destroyBuffer(instanceBuffer_);
			instanceBuffer_ = nullptr;
		}
	}

} // namespace BinRenderer
