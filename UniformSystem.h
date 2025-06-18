#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <cstdint>

namespace BinRenderer {

	// bgfx와 유사한 미리 정의된 유니폼 타입
	enum class PredefinedUniformType : uint8_t
	{
		View,
		Proj,
		ViewProj,
		Model,
		ModelView,
		ModelViewProj,
		InvView,
		InvProj,
		InvViewProj,
	};

    struct UniformInfo {
        std::string name;
        uint32_t offset;
        uint32_t size;
    };

    struct PredefinedUniformInfo {
        PredefinedUniformType type;
        uint32_t              offset;
        uint32_t              size;
        
    };

    class UniformLayout {
    public:
        void AddUniform(const std::string& name, uint32_t size);
        void AddPredefined(PredefinedUniformType type, uint32_t size);

        uint32_t GetTotalSize() const { return m_totalSize; }
        const std::vector<UniformInfo>& GetUniforms() const { return m_uniforms; }
        const std::vector<PredefinedUniformInfo>& GetPredefineds() const { return m_predefineds; }
        const UniformInfo* Find(const std::string& name) const;

    private:
        std::vector<UniformInfo> m_uniforms;
        std::vector<PredefinedUniformInfo> m_predefineds;
        uint32_t m_totalSize = 0;
    };

    class UniformSet {
    public:
        explicit UniformSet(std::shared_ptr<UniformLayout> layout);

        void Set(const std::string& name, const void* data, uint32_t size);
        const void* GetRawData() const { return m_buffer.data(); }
        uint32_t GetSize() const { return static_cast<uint32_t>(m_buffer.size()); }
        // Submit 시점에 자동으로 채워주는 함수
        void ApplyPredefined(PredefinedUniformType type, const void* data, uint32_t size);

    private:
        std::shared_ptr<UniformLayout> m_layout;
        std::vector<uint8_t> m_buffer;
    };

} // namespace BinRenderer
