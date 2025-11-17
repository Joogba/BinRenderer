#include "ResourceManager.h"

// Registry 헤더 include
#include "MeshRegistry.h"
#include "PSORegistry.h"
#include "SamplerRegistry.h"
#include "TextureRegistry.h"
#include "MaterialRegistry.h"
#include "ShaderRegistry.h"

namespace BinRenderer {

	ResourceManager::ResourceManager()
		: m_meshRegistry(std::make_unique<MeshRegistry>()),
		  m_psoRegistry(std::make_unique<PSORegistry>()),
		  m_samplerRegistry(std::make_unique<SamplerRegistry>()),
		  m_textureRegistry(std::make_unique<TextureRegistry>()),
		  m_materialRegistry(std::make_unique<MaterialRegistry>()),
		  m_shaderRegistry(std::make_unique<ShaderRegistry>())
	{
		// 각 레지스트리는 unique_ptr로 초기화됨
	}

	ResourceManager::~ResourceManager()
	{
		// 각 레지스트리는 자동으로 소멸됨 (unique_ptr)
	}

	// 접근자 구현
	MeshRegistry& ResourceManager::Meshes() { return *m_meshRegistry; }
	PSORegistry& ResourceManager::PSOs() { return *m_psoRegistry; }
	SamplerRegistry& ResourceManager::Samplers() { return *m_samplerRegistry; }
	TextureRegistry& ResourceManager::Textures() { return *m_textureRegistry; }
	MaterialRegistry& ResourceManager::Materials() { return *m_materialRegistry; }
	ShaderRegistry& ResourceManager::Shaders() { return *m_shaderRegistry; }

} // namespace BinRenderer
