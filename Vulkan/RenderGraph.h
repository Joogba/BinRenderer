#pragma once

#include "DescriptorSet.h"
#include "RenderPassManager.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

namespace BinRenderer::Vulkan {

using namespace std;

class RenderGraph
{
    friend class Renderer;

  public:
    struct RenderNode // RenderPass (simplified - no MSAA support)
    {
        vector<string> pipelineNames;
        vector<string> colorAttachments;
        string depthAttachment;
        string stencilAttachment;

        // ========================================
        // NEW: RenderPassManager 통합을 위한 필드
        // ========================================
        string name = "";              // Pass 이름
        string passType = "default";   // "scene", "cloth", "post", "gui" 등
        int priority = 100; // 실행 순서 (낮을수록 먼저)
        bool enabled = true;        // 활성화 여부
    };

    void addRenderNode(RenderNode node)
    {
        renderNodes_.push_back(node);
    }

    void writeToFile(const string& filename) const;
    bool readFromFile(const string& filename);

    // ========================================
    // NEW: RenderPassManager 통합 메서드
    // ========================================

    /**
     * @brief RenderNode 목록 반환 (외부 접근용)
     */
    const vector<RenderNode>& getRenderNodes() const { return renderNodes_; }

  private:
    vector<RenderNode> renderNodes_;
    // 여기서는 파이프라인들이 하나씩 순서대로 실행되는 간단한 구조

    // Helper methods for JSON serialization
    string vectorToJsonArray(const vector<string>& vec) const;
    vector<string> jsonArrayToVector(const string& jsonArray) const;
    string escapeJsonString(const string& str) const;
    string unescapeJsonString(const string& str) const;
    vector<string> parseJsonField(const string& nodeContent, const string& fieldName) const;
    string parseJsonStringField(const string& nodeContent, const string& fieldName) const;

    // NEW: JSON parsing helpers
    int parseJsonIntField(const string& nodeContent, const string& fieldName, int defaultValue) const;
    bool parseJsonBoolField(const string& nodeContent, const string& fieldName, bool defaultValue) const;
};

} // namespace BinRenderer::Vulkan