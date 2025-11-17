#include "Model.h"
#include "ModelNode.h"
#include "Vertex.h"
#include "Logger.h"
#include "ModelLoader.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <filesystem>

#include <glm/gtx/string_cast.hpp>
#include <iostream>

namespace BinRenderer::Vulkan {

using namespace std;
using namespace glm;

Model::Model(Context& ctx, VulkanResourceManager* resourceManager)
    : ctx_(ctx), resourceManager_(resourceManager)  // ✅ resourceManager 초기화
{
    rootNode_ = make_unique<ModelNode>();
    rootNode_->name = "Root";

    // Initialize animation system
    animation_ = make_unique<Animation>();
    
    if (resourceManager_) {
        printLog("✅ Model created with VulkanResourceManager support");
    }
}

Model::~Model()
{
    cleanup();
}

void Model::prepareForBindlessRendering(Sampler& sampler, vector<MaterialUBO>& allMaterials,
      TextureManager& textureManager)
{
    for (auto& t : textures_) {
        t->setSampler(sampler.handle());
    }

    int materialBaseIndex = int(allMaterials.size());
    int textureBaseIndex = int(textureManager.textures_.size());

    // Append textures to textureManager.textures_ (shared_ptr이므로 복사)
    textureManager.textures_.reserve(textureManager.textures_.size() + textures_.size());
    for (const auto& texture : textures_) {  // ✅ const auto& 로 변경 (복사)
   textureManager.textures_.push_back(texture);  // ✅ shared_ptr 복사
    }
    // textures_.clear(); 제거 - shared_ptr이므로 유지 가능

    // Create single large storage buffer for all materials (bindless)
    if (!materials_.empty()) {

        // Adjust all material texture indices by adding baseIndex if they are not -1
        for (auto& material : materials_) {
            if (material.ubo_.baseColorTextureIndex_ != -1) {
                material.ubo_.baseColorTextureIndex_ += textureBaseIndex;
            }
            if (material.ubo_.emissiveTextureIndex_ != -1) {
                material.ubo_.emissiveTextureIndex_ += textureBaseIndex;
            }
            if (material.ubo_.normalTextureIndex_ != -1) {
                material.ubo_.normalTextureIndex_ += textureBaseIndex;
            }
            if (material.ubo_.opacityTextureIndex_ != -1) {
                material.ubo_.opacityTextureIndex_ += textureBaseIndex;
            }
            if (material.ubo_.metallicRoughnessTextureIndex_ != -1) {
                material.ubo_.metallicRoughnessTextureIndex_ += textureBaseIndex;
            }
            if (material.ubo_.occlusionTextureIndex_ != -1) {
                material.ubo_.occlusionTextureIndex_ += textureBaseIndex;
            }
        }

        for (const auto& material : materials_) {
            allMaterials.push_back(material.ubo_);
        }

        // Iterate all meshes and add materialBaseIndex to mesh material index
        for (auto& mesh : meshes_) {
            mesh.materialIndex_ += materialBaseIndex;
        }
    }
}

void Model::createVulkanResources()
{
    // Create mesh buffers
    for (auto& mesh : meshes_) {
        mesh.createBuffers(ctx_);
    }

    // Create material uniform buffers
    // for (auto& material : materials_) {
    //    material.createUniformBuffer(ctx_);
    //    material.updateUniformBuffer();
    //}
}

void Model::loadFromModelFile(const string& modelFilename, bool readBistroObj)
{
    ModelLoader modelLoader(*this);
    modelLoader.loadFromModelFile(modelFilename, readBistroObj);
    createVulkanResources();
}

void Model::calculateBoundingBox()
{
    boundingBoxMin_ = vec3(FLT_MAX);
    boundingBoxMax_ = vec3(-FLT_MAX);

    for (const auto& mesh : meshes_) {
        boundingBoxMin_ = min(boundingBoxMin_, mesh.minBounds);
        boundingBoxMax_ = max(boundingBoxMax_, mesh.maxBounds);
    }
}

void Model::cleanup()
{
    // ========================================
    // ✅ GPU Instancing: Step 2 - Cleanup Instance Buffer
    // ========================================
    destroyInstanceBuffer();

    for (auto& mesh : meshes_) {
        mesh.cleanup(ctx_.device());
    }

    for (auto& texture : textures_) {
        texture->cleanup();
    }

    meshes_.clear();
    materials_.clear();
}

void Model::updateAnimation(float deltaTime)
{
    if (animation_ && animation_->hasAnimations()) {
      animation_->updateAnimation(deltaTime);
    }
}

// ========================================
// ✅ GPU Instancing: Step 1 - Instance Management Implementation
// ========================================

void Model::addInstance(const glm::mat4& transform, uint32_t materialOffset)
{
    InstanceData instance;
    instance.modelMatrix = transform;
    instance.materialOffset = materialOffset;
    instance.padding[0] = 0;
    instance.padding[1] = 0;
    instance.padding[2] = 0;
    
    instances_.push_back(instance);

    printLog("✅ Added instance #{} to model '{}' at ({:.2f}, {:.2f}, {:.2f})",
        instances_.size() - 1, name_, transform[3][0], transform[3][1], transform[3][2]);

    // ========================================
    // ✅ GPU Instancing: Step 2 - Auto-create/update buffer
    // ========================================
    // Recreate buffer with new size (매번 인스턴스 추가 시)
    createInstanceBuffer();  // ✅ 자동으로 버퍼 생성/재생성
}

void Model::updateInstance(uint32_t index, const glm::mat4& transform)
{
    if (index >= instances_.size()) {
   printLog("❌ Invalid instance index {} (max: {})", index, instances_.size() - 1);
        return;
    }
    
    instances_[index].modelMatrix = transform;
  
    printLog("🔄 Updated instance #{} of model '{}'", index, name_);

    // ========================================
    // ✅ GPU Instancing: Step 2 - Update buffer
    // ========================================
    if (hasInstanceBuffer()) {
  updateInstanceBuffer();  // Upload to GPU
    }
}

void Model::removeInstance(uint32_t index)
{
    if (index >= instances_.size()) {
   printLog("❌ Invalid instance index {} (max: {})", index, instances_.size() - 1);
        return;
 }
    
    instances_.erase(instances_.begin() + index);
    
    printLog("🗑️ Removed instance #{} from model '{}'", index, name_);
}

void Model::clearInstances()
{
    size_t count = instances_.size();
    instances_.clear();
    
    printLog("🧹 Cleared {} instances from model '{}'", count, name_);
}

// ========================================
// ✅ GPU Instancing: Step 2 - Instance Buffer Implementation
// ========================================

void Model::createInstanceBuffer()
{
    if (instances_.empty()) {
        printLog("⚠️ No instances to create buffer for model '{}'", name_);
        return;
    }

    // Destroy existing buffer if any
    destroyInstanceBuffer();

    instanceBufferSize_ = sizeof(InstanceData) * instances_.size();

    printLog("📦 Creating instance buffer for model '{}': {} instances ({} bytes)",
   name_, instances_.size(), instanceBufferSize_);

    // Create buffer (HOST_VISIBLE for dynamic updates)
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = instanceBufferSize_;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;  // Used as vertex input
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

check(vkCreateBuffer(ctx_.device(), &bufferInfo, nullptr, &instanceBuffer_));

    // Allocate memory
    VkMemoryRequirements memRequirements;
 vkGetBufferMemoryRequirements(ctx_.device(), instanceBuffer_, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = ctx_.getMemoryTypeIndex(
  memRequirements.memoryTypeBits,
     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    check(vkAllocateMemory(ctx_.device(), &allocInfo, nullptr, &instanceBufferMemory_));
    check(vkBindBufferMemory(ctx_.device(), instanceBuffer_, instanceBufferMemory_, 0));

    // Persistent mapping for efficient updates
    check(vkMapMemory(ctx_.device(), instanceBufferMemory_, 0, instanceBufferSize_, 0, 
     &instanceBufferMapped_));

    // Initial upload
    updateInstanceBuffer();

    printLog("✅ Instance buffer created for model '{}'", name_);
}

void Model::updateInstanceBuffer()
{
    if (!instanceBufferMapped_ || instances_.empty()) {
   return;
    }

    // Copy instance data to mapped memory
    memcpy(instanceBufferMapped_, instances_.data(), instanceBufferSize_);

    // No need to flush - using HOST_COHERENT memory
    
    printLog("🔄 Updated instance buffer for model '{}' ({} instances)",
    name_, instances_.size());
}

void Model::destroyInstanceBuffer()
{
    if (instanceBuffer_ == VK_NULL_HANDLE) {
        return;
    }

    printLog("🗑️ Destroying instance buffer for model '{}'", name_);

    if (instanceBufferMapped_) {
        vkUnmapMemory(ctx_.device(), instanceBufferMemory_);
  instanceBufferMapped_ = nullptr;
    }

    if (instanceBuffer_ != VK_NULL_HANDLE) {
        vkDestroyBuffer(ctx_.device(), instanceBuffer_, nullptr);
        instanceBuffer_ = VK_NULL_HANDLE;
    }

    if (instanceBufferMemory_ != VK_NULL_HANDLE) {
    vkFreeMemory(ctx_.device(), instanceBufferMemory_, nullptr);
        instanceBufferMemory_ = VK_NULL_HANDLE;
    }

    instanceBufferSize_ = 0;
}

// ========================================
// ✅ GPU Instancing: Step 4 - Pipeline Configuration Implementation
// ========================================

VkVertexInputBindingDescription Model::getInstanceBindingDescription()
{
    /*
     * Instance Buffer Binding Description:
   * - binding: 1 (Binding 0은 vertex buffer, Binding 1은 instance buffer)
     * - stride: sizeof(InstanceData)
     * - inputRate: VK_VERTEX_INPUT_RATE_INSTANCE (per-instance data)
     */
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 1;  // Instance buffer uses binding 1
    bindingDescription.stride = sizeof(InstanceData);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;  // ✅ Per-instance!
    
    return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> Model::getInstanceAttributeDescriptions()
{
    /*
     * Instance Attribute Descriptions:
     * - location 10-13: instanceModelMatrix (mat4 = 4 × vec4)
     * - location 14: instanceMaterialOffset (uint32_t)
*/
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(5);
    
    // instanceModelMatrix - mat4 (4개의 vec4)
    // Column 0 (location = 10)
    attributeDescriptions[0].binding = 1;
    attributeDescriptions[0].location = 10;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(InstanceData, modelMatrix) + sizeof(float) * 0;
    
    // Column 1 (location = 11)
    attributeDescriptions[1].binding = 1;
    attributeDescriptions[1].location = 11;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(InstanceData, modelMatrix) + sizeof(float) * 4;
    
    // Column 2 (location = 12)
    attributeDescriptions[2].binding = 1;
    attributeDescriptions[2].location = 12;
    attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(InstanceData, modelMatrix) + sizeof(float) * 8;
    
    // Column 3 (location = 13)
  attributeDescriptions[3].binding = 1;
    attributeDescriptions[3].location = 13;
    attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(InstanceData, modelMatrix) + sizeof(float) * 12;
    
    // instanceMaterialOffset (location = 14)
    attributeDescriptions[4].binding = 1;
    attributeDescriptions[4].location = 14;
    attributeDescriptions[4].format = VK_FORMAT_R32_UINT;
    attributeDescriptions[4].offset = offsetof(InstanceData, materialOffset);
    
    return attributeDescriptions;
}

} // namespace BinRenderer::Vulkan