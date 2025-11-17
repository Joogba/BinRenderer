#pragma once

#include <memory>

// Forward declarations (플랫폼 독립적)
namespace BinRenderer {
	class MeshRegistry;
	class PSORegistry;
	class SamplerRegistry;
	class TextureRegistry;
	class MaterialRegistry;
	class ShaderRegistry;
}

namespace BinRenderer {

    /**
     * @brief 플랫폼 독립적 리소스 레지스트리 통합 관리자
     * 
     * 메타데이터 레벨의 리소스만 관리합니다.
     * GPU 리소스 관리는 각 API별 매니저(VulkanResourceManager 등)에서 담당합니다.
     */
    class ResourceManager {
    public:
        ResourceManager();
  ~ResourceManager();

        // 접근자 함수
MeshRegistry& Meshes();
        PSORegistry& PSOs();
        SamplerRegistry& Samplers();
        TextureRegistry& Textures();
    MaterialRegistry& Materials();
        ShaderRegistry& Shaders();

    private:
   std::unique_ptr<MeshRegistry> m_meshRegistry;
        std::unique_ptr<PSORegistry> m_psoRegistry;
    std::unique_ptr<SamplerRegistry> m_samplerRegistry;
        std::unique_ptr<TextureRegistry> m_textureRegistry;
        std::unique_ptr<MaterialRegistry> m_materialRegistry;
 std::unique_ptr<ShaderRegistry> m_shaderRegistry;
    };

} // namespace BinRenderer