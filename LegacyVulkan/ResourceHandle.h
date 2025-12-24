#pragma once

#include <cstdint>
#include <type_traits>
#include <functional>
#include <atomic> // Add atomic header

namespace BinRenderer::Vulkan {

/// @brief Type-safe resource handle wrapper
/// @details Provides compile-time type safety and runtime validation for GPU resources
template<typename Tag>
struct ResourceHandle {
    uint64_t value = 0;
    
    // Constructors
    constexpr ResourceHandle() noexcept = default;
    constexpr explicit ResourceHandle(uint64_t val) noexcept : value(val) {}
    
    // Validation
    [[nodiscard]] constexpr bool isValid() const noexcept { return value != 0; }
    [[nodiscard]] constexpr explicit operator bool() const noexcept { return isValid(); }
    
    // Comparison operators
 [[nodiscard]] constexpr bool operator==(const ResourceHandle& other) const noexcept {
        return value == other.value;
    }
    
    [[nodiscard]] constexpr bool operator!=(const ResourceHandle& other) const noexcept {
    return value != other.value;
    }
    
 [[nodiscard]] constexpr bool operator<(const ResourceHandle& other) const noexcept {
        return value < other.value;
    }
    
    // Factory methods
  [[nodiscard]] static constexpr ResourceHandle invalid() noexcept {
        return ResourceHandle{0};
    }
  
    // Reset to invalid state
    constexpr void reset() noexcept {
        value = 0;
    }
};

// ============================================================================
// Tag types for different resource kinds
// ============================================================================

struct ImageTag {};
struct BufferTag {};
struct PipelineTag {};
struct DescriptorSetTag {};
struct SamplerTag {};
struct ShaderTag {};
struct CommandBufferTag {};

// ============================================================================
// Concrete handle types
// ============================================================================

using ImageHandle = ResourceHandle<ImageTag>;
using BufferHandle = ResourceHandle<BufferTag>;
using PipelineHandle = ResourceHandle<PipelineTag>;
using DescriptorSetHandle = ResourceHandle<DescriptorSetTag>;
using SamplerHandle = ResourceHandle<SamplerTag>;
using ShaderHandle = ResourceHandle<ShaderTag>;
using CommandBufferHandle = ResourceHandle<CommandBufferTag>;

// ============================================================================
// Resource type traits
// ============================================================================

/// @brief Check if type is a resource handle at compile time
template<typename T>
struct is_resource_handle : std::false_type {};

template<typename Tag>
struct is_resource_handle<ResourceHandle<Tag>> : std::true_type {};

template<typename T>
inline constexpr bool is_resource_handle_v = is_resource_handle<T>::value;

// ============================================================================
// Handle generation helper
// ============================================================================

/// @brief Thread-safe handle generator
/// @details Generates unique handles for resource tracking
class HandleGenerator {
public:
 template<typename Tag>
    [[nodiscard]] static ResourceHandle<Tag> generate() noexcept {
    static std::atomic<uint64_t> counter(1); // Parentheses initialization for C++17 compatibility
        return ResourceHandle<Tag>{counter.fetch_add(1, std::memory_order_relaxed)};
 }
    
private:
    HandleGenerator() = default;
};

} // namespace BinRenderer::Vulkan

// ============================================================================
// Hash support for STL containers
// ============================================================================

namespace std {
    template<typename Tag>
    struct hash<BinRenderer::Vulkan::ResourceHandle<Tag>> {
   [[nodiscard]] size_t operator()(
            const BinRenderer::Vulkan::ResourceHandle<Tag>& handle) const noexcept {
      return std::hash<uint64_t>{}(handle.value);
        }
    };
} // namespace std
