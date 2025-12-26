#include "VulkanResourceManager.h"
#include "Model.h"
#include "Context.h"
#include "ResourceRegistry.h"
#include "Image2D.h"
#include "Logger.h"
#include "../Core/RenderStates.h"
#include "../Resources/ResourceManager.h"
#include "../Resources/TextureRegistry.h"  //  TextureRegistry 헤더 추가

namespace BinRenderer::Vulkan {

	VulkanResourceManager::VulkanResourceManager(
		BinRenderer::ResourceManager& baseResourceManager,
		Context& ctx)
		: baseResourceManager_(baseResourceManager),
		  ctx_(ctx),
		  gpuResourceRegistry_(std::make_unique<ResourceRegistry>(ctx))
	{
		printLog("VulkanResourceManager initialized");
	}

	VulkanResourceManager::~VulkanResourceManager()
	{
		ClearModelCache();
		printLog("VulkanResourceManager destroyed");
	}

	ResourceRegistry& VulkanResourceManager::GetGpuResources()
	{
		return *gpuResourceRegistry_;
	}

	std::shared_ptr<Model> VulkanResourceManager::LoadOrGetModel(
		const std::string& resourcePath)
	{
		// 캐시 확인
		auto it = modelCache_.find(resourcePath);
		if (it != modelCache_.end()) {
			printLog(" Model cache HIT: {}", resourcePath);
			return it->second;
		}

		// 캐시 미스 - 새로 로드
		printLog("📦 Loading model: {}", resourcePath);
		auto model = std::make_shared<Model>(ctx_, this);  //  this 전달하여 VulkanResourceManager 제공
		model->loadFromModelFile(resourcePath, false);

		// 캐시에 저장
		modelCache_[resourcePath] = model;

		printLog("   Total cached models: {}", modelCache_.size());
		return model;
	}

	void VulkanResourceManager::UnloadModel(const std::string& resourcePath)
	{
		auto it = modelCache_.find(resourcePath);
		if (it != modelCache_.end()) {
			printLog("🗑️ Unloading model: {}", resourcePath);
			modelCache_.erase(it);
			printLog("   Remaining cached models: {}", modelCache_.size());
		}
	}

	void VulkanResourceManager::ClearModelCache()
	{
		if (!modelCache_.empty()) {
			printLog("🗑️ Clearing model cache ({} models)", modelCache_.size());
			modelCache_.clear();
		}
	}

	// ========================================
	// Texture Loading
	// ========================================

	std::shared_ptr<Image2D> VulkanResourceManager::LoadOrGetTexture(
		const std::string& texturePath,
		bool sRGB)
	{
		// 캐시 확인
		auto it = textureCache_.find(texturePath);
		if (it != textureCache_.end()) {
			printLog(" Texture cache HIT: {}", texturePath);
			return it->second;
		}

		// 캐시 미스 - 새로 로드
		printLog("🖼️ Loading texture: {} (sRGB: {})", texturePath, sRGB);
		auto texture = std::make_shared<Image2D>(ctx_);
		
		try {
			texture->createTextureFromImage(texturePath, false, sRGB);
			
			// 캐시에 저장
			textureCache_[texturePath] = texture;
			
			// Registry에 메타데이터 등록
			RegisterTextureMetadata(texturePath, texture);
			
			printLog("   Total cached textures: {}", textureCache_.size());
			return texture;
		}
		catch (const std::exception& e) {
			printLog("❌ Failed to load texture: {} - {}", texturePath, e.what());
			return nullptr;
		}
	}

	void VulkanResourceManager::RegisterTextureMetadata(
		const std::string& name,
		const std::shared_ptr<Image2D>& texture)
	{
		if (!texture) return;

		// TextureDesc 생성 (플랫폼 독립적 메타데이터)
		BinRenderer::TextureDesc desc{};
		desc.width = texture->width();
		desc.height = texture->height();
		desc.format = BinRenderer::Format::R8G8B8A8_UNORM;  //  올바른 Format 사용
		desc.bindFlags = static_cast<uint32_t>(BinRenderer::BindFlags::Bind_ShaderResource);

		// Registry에 등록
		baseResourceManager_.Textures().Register(name, desc);
		
		printLog("📝 Registered texture metadata: {} ({}x{})", 
			name, desc.width, desc.height);
	}

} // namespace BinRenderer::Vulkan
