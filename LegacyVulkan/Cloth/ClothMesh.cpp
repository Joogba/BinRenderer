#include "ClothMesh.h"
#include "../Logger.h"
#include "../DescriptorSet.h"
#include "../PipelineConfig.h"
#include "../MappedBuffer.h"
#include "../StorageBuffer.h"

#include <glm/glm.hpp>

namespace BinRenderer::Vulkan {

ClothMesh::ClothMesh(Context& ctx, ShaderManager& shaderManager, ClothSimulation& simulation)
    : ctx_(ctx), shaderManager_(shaderManager), simulation_(simulation)
{
}

ClothMesh::~ClothMesh()
{
    cleanup();
}

void ClothMesh::initialize(VkFormat colorFormat, VkFormat depthFormat, 
       VkSampleCountFlagBits msaaSamples)
{
    BinRenderer::printLog("Initializing ClothMesh rendering...");

    createRenderPipeline(colorFormat, depthFormat, msaaSamples);
    createDescriptorSets();

    BinRenderer::printLog("ClothMesh rendering initialized");
}

void ClothMesh::cleanup()
{
    renderPipeline_.reset();
    sceneBuffers_.clear();
}

void ClothMesh::createRenderPipeline(VkFormat colorFormat, VkFormat depthFormat,
   VkSampleCountFlagBits msaaSamples)
{
    BinRenderer::printLog("Creating cloth render pipeline...");
    BinRenderer::printLog(" - Color format: {}", static_cast<uint32_t>(colorFormat));
    BinRenderer::printLog(" - Depth format: {}", static_cast<uint32_t>(depthFormat));
    BinRenderer::printLog(" - MSAA samples: {}", static_cast<uint32_t>(msaaSamples));

    // PipelineConfig 생성
    PipelineConfig config;
    config.name = "cloth";
    config.type = PipelineConfig::Type::Graphics;
    
    // Vertex input: None (Storage Buffer에서 직접 읽기)
    config.vertexInput.type = PipelineConfig::VertexInput::Type::None;
    
    // Depth/Stencil
    config.depthStencil.depthTest = true;
    config.depthStencil.depthWrite = true;
    config.depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    
    // Rasterization (양면 렌더링)
    config.rasterization.cullMode = VK_CULL_MODE_NONE;
    config.rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    
    // MSAA - 메모리 절약을 위해 Single로 변경
    config.multisample.type = PipelineConfig::Multisample::Type::Single;

    renderPipeline_ = std::make_unique<Pipeline>(
 ctx_, shaderManager_, config,
  std::vector<VkFormat>{colorFormat},
   depthFormat,
        VK_SAMPLE_COUNT_1_BIT  //  MSAA 비활성화
    );

    BinRenderer::printLog(" - Created cloth render pipeline");
}

void ClothMesh::createDescriptorSets()
{
    const uint32_t frameCount = 2; // kMaxFramesInFlight

    // Scene Uniform Buffers 생성 (per frame)
    sceneBuffers_.resize(frameCount);
    for (uint32_t i = 0; i < frameCount; ++i) {
        sceneBuffers_[i] = std::make_unique<MappedBuffer>(ctx_);
  sceneBuffers_[i]->createUniformBuffer(sceneData_);
  }

    // Descriptor Sets 생성
    descriptorSets_.resize(frameCount);
    for (uint32_t i = 0; i < frameCount; ++i) {
        std::vector<std::reference_wrapper<Resource>> resources;
        resources.push_back(*simulation_.getPositionBuffer());  // set 0, binding 0
        resources.push_back(*sceneBuffers_[i]);    // set 0, binding 1

        descriptorSets_[i].create(
      ctx_,
      renderPipeline_->layouts()[0],
     resources
        );
    }

  BinRenderer::printLog(" - Created cloth descriptor sets");
}

void ClothMesh::render(VkCommandBuffer cmd, uint32_t frameIndex,
   const glm::mat4& viewProjection, const glm::vec3& cameraPos)
{
    // Scene data 업데이트
    sceneData_.viewProjection = viewProjection;
    sceneData_.cameraPos = cameraPos;
    sceneData_.time = 0.0f; // TODO: 실제 시간 전달
    sceneBuffers_[frameIndex]->updateFromCpuData();

    // Pipeline 바인딩
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderPipeline_->pipeline());

    // Descriptor sets 바인딩
 vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
        renderPipeline_->pipelineLayout(), 0, 1,
        &descriptorSets_[frameIndex].handle(), 0, nullptr);

    // Push constants (Model matrix)
    vkCmdPushConstants(cmd, renderPipeline_->pipelineLayout(),
        VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &modelMatrix_);

    // Draw using indexed rendering
    // Vertex shader reads from Storage Buffer directly (gl_VertexIndex)
    const uint32_t indexCount = simulation_.getIndexCount();
    
    // 안전성 체크
    if (indexCount == 0) {
        BinRenderer::printLog("[ClothMesh] Warning: Index count is 0, skipping render");
        return;
    }
 
    StorageBuffer* indexBuffer = simulation_.getIndexBuffer();
    if (!indexBuffer) {
        BinRenderer::printLog("[ClothMesh] Error: Index buffer is null!");
        return;
    }

    VkBuffer vkIndexBuffer = indexBuffer->buffer();
    if (vkIndexBuffer == VK_NULL_HANDLE) {
        BinRenderer::printLog("[ClothMesh] Error: VkBuffer handle is null!");
   return;
    }

    // Bind index buffer and draw
    vkCmdBindIndexBuffer(cmd, vkIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(cmd, indexCount, 1, 0, 0, 0);
}

} // namespace BinRenderer::Vulkan
