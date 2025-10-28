#pragma once
#include <cstdint>

namespace BinRenderer {

    class ITransientBufferAllocator {
    public:
        virtual ~ITransientBufferAllocator() = default;

        // 프레임 경계
        virtual void beginFrame() = 0;
        virtual void endFrame() = 0;

        // 할당: size와 align을 받아 포인터와 버퍼 내 offset 반환
        virtual uint32_t alloc(uint32_t sizeBytes, void*& dataPtr, uint32_t align = 16) = 0;

        // 실제 GPU 버퍼 핸들 반환 (플랫폼별 타입으로 캐스팅 필요)
        virtual void* buffer() const = 0;
    };

} // namespace BinRenderer