#pragma once

#include "Context.h"
#include "Material.h"
#include "Mesh.h"
#include "ModelNode.h"
#include "Sampler.h"
#include "Image2D.h"
#include "TextureManager.h"
#include "Vertex.h"
#include "VulkanTools.h"
#include "Animation.h"
#include "StorageBuffer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace BinRenderer::Vulkan {

using namespace std;
using namespace glm;

// Forward declaration
class VulkanResourceManager;

// ========================================
// ✅ GPU Instancing: Step 1
// ========================================
// Per-instance data structure (16-byte aligned for GPU)
struct InstanceData {
    glm::mat4 modelMatrix;      // 64 bytes
    uint32_t materialOffset;// 4 bytes (optional: per-instance material override)
    uint32_t padding[3];      // 12 bytes (padding for 16-byte alignment)
};

class Model
{
  friend class ModelLoader;

  public:
    Model(Context& ctx, VulkanResourceManager* resourceManager = nullptr);  // ✅ VulkanResourceManager 추가
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;
    Model& operator=(Model&&) = delete;
    ~Model();

    void cleanup();
    void createVulkanResources();

    void prepareForBindlessRendering(Sampler& sampler, vector<MaterialUBO>& materials,
     TextureManager& textureManager);

    // Animation methods
    void updateAnimation(float deltaTime);
    bool hasAnimations() const
    {
        return animation_ && animation_->hasAnimations();
    }
    bool hasBones() const
    {
        return animation_ && animation_->hasBones();
    }
    uint32_t getAnimationCount() const
    {
     return animation_ ? animation_->getAnimationCount() : 0;
    }
    uint32_t getBoneCount() const
    {
        return animation_ ? animation_->getBoneCount() : 0;
    }

    // Animation playback control
    void playAnimation()
    {
   if (animation_)
   animation_->play();
    }
    void pauseAnimation()
    {
        if (animation_)
     animation_->pause();
    }
    void stopAnimation()
    {
        if (animation_)
            animation_->stop();
    }
    bool isAnimationPlaying() const
    {
      return animation_ && animation_->isPlaying();
    }
    void setAnimationIndex(uint32_t index)
    {
        if (animation_)
   animation_->setAnimationIndex(index);
    }
    void setAnimationSpeed(float speed)
    {
      if (animation_)
    animation_->setPlaybackSpeed(speed);
    }
    void setAnimationLooping(bool loop)
    {
   if (animation_)
            animation_->setLooping(loop);
  }

    // Bone matrices for shaders
    const vector<mat4>& getBoneMatrices() const
    {
        static const vector<mat4> empty;
        return animation_ ? animation_->getBoneMatrices() : empty;
    }
Animation* getAnimation() const
    {
        return animation_.get();
    }

    // ========================================
    // ✅ GPU Instancing: Instance Management
    // ========================================
    void addInstance(const glm::mat4& transform, uint32_t materialOffset = 0);
    void updateInstance(uint32_t index, const glm::mat4& transform);
    void removeInstance(uint32_t index);
    void clearInstances();
    
    uint32_t getInstanceCount() const { return static_cast<uint32_t>(instances_.size()); }
    const vector<InstanceData>& getInstances() const { return instances_; }
    
    // ✅ FIX: 1개 이상이면 instancing 활성화 (>= 1)
    bool isInstanced() const { return instances_.size() >= 1; }

    // ========================================
    // ✅ GPU Instancing: Step 2 - Buffer Access
    // ========================================
    VkBuffer getInstanceBuffer() const { return instanceBuffer_; }
    bool hasInstanceBuffer() const { return instanceBuffer_ != VK_NULL_HANDLE; }

    // ========================================
    // ✅ GPU Instancing: Step 4 - Pipeline Configuration
    // ========================================
    // Instance vertex input binding/attributes for pipeline creation
 static VkVertexInputBindingDescription getInstanceBindingDescription();
    static std::vector<VkVertexInputAttributeDescription> getInstanceAttributeDescriptions();

    // ========================================
    // Accessors
 // ========================================
    vector<Mesh>& meshes() { return meshes_; }
    vector<Material>& materials() { return materials_; }
    uint32_t numMaterials() const { return uint32_t(materials_.size()); }
    ModelNode* rooNode() const { return rootNode_.get(); }
    vec3 boundingBoxMin() const { return boundingBoxMin_; }
    vec3 boundingBoxMax() const { return boundingBoxMax_; }

    void loadFromModelFile(const string& modelFilename, bool readBistroObj);

    string& name() { return name_; }
    bool& visible() { return visible_; }
    mat4& modelMatrix() { return modelMatrix_; }
    float* coeffs() { return coeffs_; }

  private:
    Context& ctx_;
    VulkanResourceManager* resourceManager_ = nullptr;

    // Model asset data
    vector<Mesh> meshes_;
    vector<Material> materials_;

    vector<shared_ptr<Image2D>> textures_;  // ✅ unique_ptr → shared_ptr 변경 (캐싱 지원)
    vector<string> textureFilenames_;
    vector<bool> textureSRgb_; // sRGB 여부

    unique_ptr<ModelNode> rootNode_;
    unique_ptr<Animation> animation_;

    mat4 globalInverseTransform_ = mat4(1.0f);

    // Bounding box
    vec3 boundingBoxMin_ = vec3(FLT_MAX);
    vec3 boundingBoxMax_ = vec3(-FLT_MAX);

    string name_{};
    bool visible_ = true;
  mat4 modelMatrix_ = mat4(1.0f);  // Legacy: 단일 인스턴스용 (하위 호환)
    float coeffs_[16] = {0.0f}; // 여러가지 옵션에 사용

    // ========================================
    // ✅ GPU Instancing: Instance Storage
    // ========================================
 vector<InstanceData> instances_;  // All instance transforms

    // ========================================
    // ✅ GPU Instancing: Step 2 - Instance Buffer
    // ========================================
    VkBuffer instanceBuffer_ = VK_NULL_HANDLE;
    VkDeviceMemory instanceBufferMemory_ = VK_NULL_HANDLE;
    void* instanceBufferMapped_ = nullptr;  // Persistent mapping for updates
    VkDeviceSize instanceBufferSize_ = 0;
    
    // Instance buffer management
  void createInstanceBuffer();
    void updateInstanceBuffer();
    void destroyInstanceBuffer();

    void calculateBoundingBox();
};

} // namespace BinRenderer::Vulkan