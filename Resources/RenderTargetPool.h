#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>

#include "Core/Handle.h"
#include "Core/RenderEnums.h"
#include "TextureRegistry.h"

namespace std {
    template<>
    struct std::hash <BinRenderer::TextureDesc> {
        size_t operator()(const BinRenderer::TextureDesc& d) const {
            return hash<uint32_t>()(d.width) ^ hash<uint32_t>()(d.height) ^ hash<uint32_t>()((uint32_t)d.format);
        }
    };
}

namespace BinRenderer {
    class RenderTargetPool {
    public:
        TextureHandle Acquire(const TextureDesc& desc) {
            // 1. 같은 desc 텍스처가 있으면 꺼내서 반환
            auto& pool = m_pool[desc];
            if (!pool.empty()) {
                TextureHandle h = pool.back();
                pool.pop_back();
                return h;
            }
            // 2. 없으면 새로 생성 (Registry 등에서!)
            TextureHandle newTex = m_textureRegistry->Register("name",desc); // 레지스터함수는 이름이 같이필요함
            return newTex;
        }

        void Release(const TextureDesc& desc, TextureHandle handle) {
            m_pool[desc].push_back(handle);
        }

        void Reset() {
            // 프레임/그래프 종료 후 미사용 자원 반납 (풀 비우거나 필요시 해제)
            m_pool.clear();
        }

        void SetTextureRegistry(TextureRegistry* reg) { m_textureRegistry = reg; }

    private:
        std::unordered_map<TextureDesc, std::vector<TextureHandle>> m_pool;
        TextureRegistry* m_textureRegistry = nullptr; // Texture 핸들 생성/관리
    };    
}