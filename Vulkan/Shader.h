#pragma once

#include "Context.h"

#include <spirv-reflect/spirv_reflect.h>
#include <unordered_map>
#include <vulkan/vulkan.h>
#include <array>

namespace BinRenderer::Vulkan {

using namespace std;

class Shader
{
    friend class ShaderManager; // Allow ShaderManager to access private members

  public:
    Shader(Context& ctx, string spvFilename);

    Shader(Shader&& other) noexcept;

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader& operator=(Shader&&) = delete;

    ~Shader();

    void cleanup();

    // Get compute shader local workgroup size (local_size_x, local_size_y, local_size_z)
    auto getLocalWorkgroupSize() const -> array<uint32_t, 3>;

  private:
    Context& ctx_; // for creation and cleanup
    VkShaderModule shaderModule_{VK_NULL_HANDLE};
    SpvReflectShaderModule reflectModule_{};
    VkShaderStageFlagBits stage_{VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM};
    string name_{""};

    auto readSpvFile(const string& spvFilename) -> vector<char>;
    auto createShaderModule(const vector<char>& shaderCode) -> VkShaderModule;
    auto createRefModule(const vector<char>& shaderCode) -> SpvReflectShaderModule;
    auto makeVertexInputAttributeDescriptions() const -> vector<VkVertexInputAttributeDescription>;
};

} // namespace BinRenderer::Vulkan