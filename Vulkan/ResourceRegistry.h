#pragma once

#include "ResourceHandle.h"
#include "Resource.h"
#include "Logger.h"

#include <unordered_map>
#include <memory>
#include <string>
#include <optional>
#include <shared_mutex>
#include <vector>

namespace BinRenderer::Vulkan {

using BinRenderer::printLog; // Add using declaration for printLog

class Context;

/// @brief Centralized registry for managing all GPU resources with type-safe handles
/// @details Provides handle-based resource access, automatic lifetime management,
///       and backward compatibility with name-based lookup
class ResourceRegistry {
public:
    explicit ResourceRegistry(Context& ctx);
    ~ResourceRegistry();
  
    // Prevent copying
    ResourceRegistry(const ResourceRegistry&) = delete;
    ResourceRegistry& operator=(const ResourceRegistry&) = delete;
    
    // Allow moving
    ResourceRegistry(ResourceRegistry&&) noexcept = default;
    ResourceRegistry& operator=(ResourceRegistry&&) noexcept = default;
    
    // ========================================================================
    // Generic Resource Registration
    // ========================================================================
    
    /// @brief Register a resource with automatic handle generation
  /// @param name Debug name for the resource (optional, can be empty)
    /// @param resource Unique pointer to the resource (ownership transferred)
    /// @return Handle to the registered resource (invalid on failure)
    template<typename ResourceType>
    [[nodiscard]] auto registerResource(
    const std::string& name,
        std::unique_ptr<ResourceType> resource) -> ImageHandle
        requires std::is_base_of_v<Resource, ResourceType>
{
 if (!resource) {
    printLog("ERROR: Cannot register null resource '{}'", name);
    return ImageHandle::invalid();
  }
     
     std::unique_lock lock(mutex_);
  
    // Check for duplicate names
        if (!name.empty() && nameToImageHandle_.contains(name)) {
     printLog("WARNING: Resource '{}' already exists, replacing", name);
  destroyResourceInternal(nameToImageHandle_[name]);
        }
        
        ImageHandle handle = HandleGenerator::generate<ImageTag>();
     
 ResourceEntry entry{
      .resource = std::move(resource),
     .debugName = name,
            .type = entry.resource->getType()
     };
     
        imageResources_[handle] = std::move(entry);
        
        if (!name.empty()) {
  nameToImageHandle_[name] = handle;
        }
     
      printLog("DEBUG: Registered {} resource '{}' with handle {}",
entry.type == Resource::Type::Image ? "image" : "buffer",
              name, handle.value);
        
    return handle;
    }
    
    // ========================================================================
    // Specialized Registration Methods
 // ========================================================================
    
    /// @brief Register an image resource (Image2D, etc.)
    template<typename ImageType>
    [[nodiscard]] ImageHandle registerImage(
        const std::string& name,
        std::unique_ptr<ImageType> image)
        requires std::is_base_of_v<Resource, ImageType>
    {
      return registerResource(name, std::move(image));
    }
    
    /// @brief Register a buffer resource (MappedBuffer, StorageBuffer, etc.)
    template<typename BufferType>
    [[nodiscard]] BufferHandle registerBuffer(
        const std::string& name,
 std::unique_ptr<BufferType> buffer)
 requires std::is_base_of_v<Resource, BufferType>
    {
    if (!buffer) {
     printLog("ERROR: Cannot register null buffer '{}'", name);
          return BufferHandle::invalid();
 }
        
        std::unique_lock lock(mutex_);
    
    if (!name.empty() && nameToBufferHandle_.contains(name)) {
printLog("WARNING: Buffer '{}' already exists, replacing", name);
      destroyBufferInternal(nameToBufferHandle_[name]);
        }
        
        BufferHandle handle = HandleGenerator::generate<BufferTag>();
 
        ResourceEntry entry{
  .resource = std::move(buffer),
   .debugName = name,
     .type = Resource::Type::Buffer
      };
     
  bufferResources_[handle] = std::move(entry);
     
        if (!name.empty()) {
   nameToBufferHandle_[name] = handle;
        }
 
     printLog("DEBUG: Registered buffer '{}' with handle {}", name, handle.value);
    
        return handle;
    }

    // ========================================================================
    // Resource Retrieval
    // ========================================================================
    
    /// @brief Get resource by handle (generic)
    /// @return Pointer to resource, or nullptr if handle is invalid
    [[nodiscard]] Resource* getResource(ImageHandle handle);
    [[nodiscard]] const Resource* getResource(ImageHandle handle) const;
    
    [[nodiscard]] Resource* getResource(BufferHandle handle);
    [[nodiscard]] const Resource* getResource(BufferHandle handle) const;
    
    /// @brief Get resource as specific type with runtime type checking
    /// @tparam T Derived resource type (e.g., Image2D, MappedBuffer)
    /// @return Typed pointer or nullptr if handle invalid or type mismatch
    template<typename T>
    [[nodiscard]] T* getResourceAs(ImageHandle handle)
        requires std::is_base_of_v<Resource, T>
    {
        Resource* res = getResource(handle);
     if (!res) {
    return nullptr;
        }
        
        T* typedRes = dynamic_cast<T*>(res);
        if (!typedRes) {
   printLog("ERROR: ImageHandle {} is not of type {}",
         handle.value, typeid(T).name());
        }
  return typedRes;
    }
    
    template<typename T>
  [[nodiscard]] const T* getResourceAs(ImageHandle handle) const
        requires std::is_base_of_v<Resource, T>
    {
     const Resource* res = getResource(handle);
        if (!res) {
  return nullptr;
        }

        const T* typedRes = dynamic_cast<const T*>(res);
    if (!typedRes) {
        printLog("ERROR: ImageHandle {} is not of type {}",
   handle.value, typeid(T).name());
  }
  return typedRes;
    }
    
  template<typename T>
    [[nodiscard]] T* getResourceAs(BufferHandle handle)
   requires std::is_base_of_v<Resource, T>
    {
   Resource* res = getResource(handle);
if (!res) {
       return nullptr;
    }
    
        T* typedRes = dynamic_cast<T*>(res);
      if (!typedRes) {
      printLog("ERROR: BufferHandle {} is not of type {}",
 handle.value, typeid(T).name());
}
   return typedRes;
    }
    
    template<typename T>
    [[nodiscard]] const T* getResourceAs(BufferHandle handle) const
        requires std::is_base_of_v<Resource, T>
  {
  const Resource* res = getResource(handle);
     if (!res) {
     return nullptr;
        }
   
     const T* typedRes = dynamic_cast<const T*>(res);
     if (!typedRes) {
    printLog("ERROR: BufferHandle {} is not of type {}",
   handle.value, typeid(T).name());
  }
   return typedRes;
    }
    
    // ========================================================================
    // Name-based Lookup (Backward Compatibility)
    // ========================================================================
    
    /// @brief Find resource by name
    /// @return Handle to resource, or invalid handle if not found
    [[nodiscard]] ImageHandle findImage(const std::string& name) const;
    [[nodiscard]] BufferHandle findBuffer(const std::string& name) const;
    
    // ========================================================================
    // Resource Destruction
    // ========================================================================
    
    /// @brief Destroy resource and free associated GPU memory
    void destroyResource(ImageHandle handle);
    void destroyResource(BufferHandle handle);
    
    // Convenience aliases
void destroyImage(ImageHandle handle) { destroyResource(handle); }
    void destroyBuffer(BufferHandle handle) { destroyResource(handle); }
    
    // ========================================================================
    // Utilities
  // ========================================================================
  
    /// @brief Get debug name for a resource
    [[nodiscard]] std::optional<std::string> getDebugName(ImageHandle handle) const;
    [[nodiscard]] std::optional<std::string> getDebugName(BufferHandle handle) const;
    
    /// @brief Check if handle points to valid resource
    [[nodiscard]] bool isValid(ImageHandle handle) const;
    [[nodiscard]] bool isValid(BufferHandle handle) const;
    
    /// @brief Clear all resources (calls cleanup on each)
    void clear();
    
    /// @brief Get resource statistics
    struct Stats {
        size_t totalResources = 0;
        size_t imageResources = 0;
     size_t bufferResources = 0;
        size_t namedResources = 0;
    };
    
 [[nodiscard]] Stats getStats() const;
    
    /// @brief Get all image handles (useful for debugging/iteration)
    [[nodiscard]] std::vector<ImageHandle> getAllImageHandles() const;
    [[nodiscard]] std::vector<BufferHandle> getAllBufferHandles() const;
  
private:
    Context& ctx_;
    
    // Resource storage
    struct ResourceEntry {
        std::unique_ptr<Resource> resource;
        std::string debugName;
        Resource::Type type;
    };
    
    std::unordered_map<ImageHandle, ResourceEntry> imageResources_;
    std::unordered_map<BufferHandle, ResourceEntry> bufferResources_;
    
    // Name to handle lookup (backward compatibility)
    std::unordered_map<std::string, ImageHandle> nameToImageHandle_;
    std::unordered_map<std::string, BufferHandle> nameToBufferHandle_;
    
    // Thread safety
    mutable std::shared_mutex mutex_;
  
    // Internal helpers
    void destroyResourceInternal(ImageHandle handle);
    void destroyBufferInternal(BufferHandle handle);
};

} // namespace BinRenderer::Vulkan
