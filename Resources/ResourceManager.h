#pragma once
#include "MeshRegistry.h"
#include "PSORegistry.h"
#include "SamplerRegistry.h"
#include "TextureRegistry.h"
#include "MaterialRegistry.h"
#include "ShaderRegistry.h"

namespace BinRenderer {

    class ResourceManager {
    public:
        // 생성자/소멸자 등
        ResourceManager();
        ~ResourceManager();

        // 접근자 함수 (const/ref)
        MeshRegistry& Meshes() { return m_meshRegistry; }
        PSORegistry& PSOs() { return m_psoRegistry; }
        SamplerRegistry& Samplers() { return m_samplerRegistry; }
        TextureRegistry& Textures() { return m_textureRegistry; }
        MaterialRegistry& Materials() { return m_materialRegistry; }
        ShaderRegistry& Shaders() { return m_shaderRegistry; }

        // 통합 검색/등록/파기 등 필요하면 추가

    private:
        MeshRegistry      m_meshRegistry;
        PSORegistry       m_psoRegistry;
        SamplerRegistry   m_samplerRegistry;
        TextureRegistry   m_textureRegistry;
        MaterialRegistry  m_materialRegistry;
        ShaderRegistry    m_shaderRegistry;
    };

} // namespace BinRenderer