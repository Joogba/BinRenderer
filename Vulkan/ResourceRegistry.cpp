#include "ResourceRegistry.h"
#include "Context.h"
#include "Image2D.h"
#include "MappedBuffer.h"

namespace BinRenderer::Vulkan {

using BinRenderer::printLog; // Add using declaration for printLog

ResourceRegistry::ResourceRegistry(Context& ctx) : ctx_(ctx) {
    printLog("ResourceRegistry initialized");
}

ResourceRegistry::~ResourceRegistry() {
    clear();
    printLog("ResourceRegistry destroyed");
}

// ============================================================================
// Resource Retrieval
// ============================================================================

Resource* ResourceRegistry::getResource(ImageHandle handle) {
    std::shared_lock lock(mutex_);
    
    auto it = imageResources_.find(handle);
    if (it == imageResources_.end()) {
  printLog("ERROR: Invalid ImageHandle: {}", handle.value);
        return nullptr;
    }
    
    return it->second.resource.get();
}

const Resource* ResourceRegistry::getResource(ImageHandle handle) const {
    std::shared_lock lock(mutex_);

    auto it = imageResources_.find(handle);
    if (it == imageResources_.end()) {
        return nullptr;
    }
    
    return it->second.resource.get();
}

Resource* ResourceRegistry::getResource(BufferHandle handle) {
    std::shared_lock lock(mutex_);
    
    auto it = bufferResources_.find(handle);
    if (it == bufferResources_.end()) {
      printLog("ERROR: Invalid BufferHandle: {}", handle.value);
      return nullptr;
    }
    
    return it->second.resource.get();
}

const Resource* ResourceRegistry::getResource(BufferHandle handle) const {
    std::shared_lock lock(mutex_);
    
  auto it = bufferResources_.find(handle);
    if (it == bufferResources_.end()) {
  return nullptr;
  }
    
    return it->second.resource.get();
}

// ============================================================================
// Name-based Lookup
// ============================================================================

ImageHandle ResourceRegistry::findImage(const std::string& name) const {
    std::shared_lock lock(mutex_);
    
    auto it = nameToImageHandle_.find(name);
    if (it == nameToImageHandle_.end()) {
        printLog("WARNING: Image '{}' not found in registry", name);
   return ImageHandle::invalid();
    }
    
    return it->second;
}

BufferHandle ResourceRegistry::findBuffer(const std::string& name) const {
std::shared_lock lock(mutex_);
    
  auto it = nameToBufferHandle_.find(name);
  if (it == nameToBufferHandle_.end()) {
        printLog("WARNING: Buffer '{}' not found in registry", name);
 return BufferHandle::invalid();
    }
    
    return it->second;
}

// ============================================================================
// Resource Destruction
// ============================================================================

void ResourceRegistry::destroyResource(ImageHandle handle) {
    std::unique_lock lock(mutex_);
    destroyResourceInternal(handle);
}

void ResourceRegistry::destroyResource(BufferHandle handle) {
    std::unique_lock lock(mutex_);
 destroyBufferInternal(handle);
}

void ResourceRegistry::destroyResourceInternal(ImageHandle handle) {
  auto it = imageResources_.find(handle);
    if (it == imageResources_.end()) {
   printLog("WARNING: Attempted to destroy invalid ImageHandle: {}", handle.value);
        return;
    }
    
    const std::string& name = it->second.debugName;
    
    // Cleanup the resource
    it->second.resource->cleanup();
    
    // Remove from name lookup
    if (!name.empty()) {
  nameToImageHandle_.erase(name);
    }
    
    // Remove from storage
    imageResources_.erase(it);
    
    printLog("DEBUG: Destroyed image resource '{}' (handle: {})", name, handle.value);
}

void ResourceRegistry::destroyBufferInternal(BufferHandle handle) {
    auto it = bufferResources_.find(handle);
    if (it == bufferResources_.end()) {
      printLog("WARNING: Attempted to destroy invalid BufferHandle: {}", handle.value);
        return;
    }
    
    const std::string& name = it->second.debugName;
    
  // Cleanup the resource
    it->second.resource->cleanup();
    
// Remove from name lookup
    if (!name.empty()) {
        nameToBufferHandle_.erase(name);
    }
    
 // Remove from storage
  bufferResources_.erase(it);
    
  printLog("DEBUG: Destroyed buffer resource '{}' (handle: {})", name, handle.value);
}

// ============================================================================
// Utilities
// ============================================================================

std::optional<std::string> ResourceRegistry::getDebugName(ImageHandle handle) const {
    std::shared_lock lock(mutex_);
    
    auto it = imageResources_.find(handle);
    if (it == imageResources_.end()) {
        return std::nullopt;
    }
    
    return it->second.debugName;
}

std::optional<std::string> ResourceRegistry::getDebugName(BufferHandle handle) const {
    std::shared_lock lock(mutex_);
    
    auto it = bufferResources_.find(handle);
  if (it == bufferResources_.end()) {
     return std::nullopt;
    }
    
    return it->second.debugName;
}

bool ResourceRegistry::isValid(ImageHandle handle) const {
    std::shared_lock lock(mutex_);
    return imageResources_.contains(handle);
}

bool ResourceRegistry::isValid(BufferHandle handle) const {
    std::shared_lock lock(mutex_);
    return bufferResources_.contains(handle);
}

void ResourceRegistry::clear() {
    std::unique_lock lock(mutex_);
    
  // Cleanup all image resources
    for (auto& [handle, entry] : imageResources_) {
        entry.resource->cleanup();
    }
    
    // Cleanup all buffer resources
    for (auto& [handle, entry] : bufferResources_) {
        entry.resource->cleanup();
    }
    
 imageResources_.clear();
  bufferResources_.clear();
    nameToImageHandle_.clear();
    nameToBufferHandle_.clear();

    printLog("INFO: Cleared all resources from registry");
}

ResourceRegistry::Stats ResourceRegistry::getStats() const {
    std::shared_lock lock(mutex_);
 
    return Stats{
        .totalResources = imageResources_.size() + bufferResources_.size(),
        .imageResources = imageResources_.size(),
        .bufferResources = bufferResources_.size(),
        .namedResources = nameToImageHandle_.size() + nameToBufferHandle_.size()
    };
}

std::vector<ImageHandle> ResourceRegistry::getAllImageHandles() const {
    std::shared_lock lock(mutex_);
    
    std::vector<ImageHandle> handles;
    handles.reserve(imageResources_.size());
    
    for (const auto& [handle, entry] : imageResources_) {
        handles.push_back(handle);
  }
  
    return handles;
}

std::vector<BufferHandle> ResourceRegistry::getAllBufferHandles() const {
    std::shared_lock lock(mutex_);
    
    std::vector<BufferHandle> handles;
    handles.reserve(bufferResources_.size());
    
    for (const auto& [handle, entry] : bufferResources_) {
        handles.push_back(handle);
  }
    
    return handles;
}

} // namespace BinRenderer::Vulkan
