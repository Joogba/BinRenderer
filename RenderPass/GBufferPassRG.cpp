#include "GBufferPassRG.h"
#include "../Core/Logger.h"

namespace BinRenderer
{
	GBufferPassRG::GBufferPassRG(RHI* rhi)
		: RGPass<GBufferPassData>(rhi, "GBufferPass")
	{
	}

	GBufferPassRG::~GBufferPassRG()
	{
		shutdown();
	}

	bool GBufferPassRG::initialize()
	{
		printLog("[GBufferPassRG] Initializing...");

		// 파이프라인 생성 (셰이더, 파이프라인 상태 등)
		createPipeline();

		printLog("[GBufferPassRG] Initialized successfully");
		return true;
	}

	void GBufferPassRG::shutdown()
	{
		destroyPipeline();
	}

	void GBufferPassRG::setup(GBufferPassData& data, RenderGraphBuilder& builder)
	{
		printLog("[GBufferPassRG] Setup - Creating G-Buffer resources");

		// Albedo (RGB: Albedo, A: AO)
		RGTextureDesc albedoDesc;
		albedoDesc.name = "GBuffer_Albedo";
		albedoDesc.width = width_;
		albedoDesc.height = height_;
		albedoDesc.format = RHI_FORMAT_R8G8B8A8_UNORM;
		albedoDesc.usage = RHI_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | 
		     RHI_IMAGE_USAGE_SAMPLED_BIT;
		data.albedo = builder.writeTexture(builder.createTexture(albedoDesc));

		// Normal (RGB: Normal in World Space)
		RGTextureDesc normalDesc;
		normalDesc.name = "GBuffer_Normal";
		normalDesc.width = width_;
		normalDesc.height = height_;
		normalDesc.format = RHI_FORMAT_R16G16B16A16_SFLOAT;
		normalDesc.usage = RHI_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | 
		 RHI_IMAGE_USAGE_SAMPLED_BIT;
		data.normal = builder.writeTexture(builder.createTexture(normalDesc));

		// Position (RGB: Position in World Space)
		RGTextureDesc positionDesc;
		positionDesc.name = "GBuffer_Position";
		positionDesc.width = width_;
		positionDesc.height = height_;
		positionDesc.format = RHI_FORMAT_R16G16B16A16_SFLOAT;
		positionDesc.usage = RHI_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | 
		        RHI_IMAGE_USAGE_SAMPLED_BIT;
		data.position = builder.writeTexture(builder.createTexture(positionDesc));

		// Metallic-Roughness (R: Metallic, G: Roughness)
		RGTextureDesc mrDesc;
		mrDesc.name = "GBuffer_MetallicRoughness";
		mrDesc.width = width_;
		mrDesc.height = height_;
		mrDesc.format = RHI_FORMAT_R8G8_UNORM;
		mrDesc.usage = RHI_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | 
		         RHI_IMAGE_USAGE_SAMPLED_BIT;
		data.metallicRoughness = builder.writeTexture(builder.createTexture(mrDesc));

		// Depth
		RGTextureDesc depthDesc;
		depthDesc.name = "GBuffer_Depth";
		depthDesc.width = width_;
		depthDesc.height = height_;
		depthDesc.format = RHI_FORMAT_D32_SFLOAT;
		depthDesc.usage = RHI_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | 
		        RHI_IMAGE_USAGE_SAMPLED_BIT;
		data.depth = builder.writeTexture(builder.createTexture(depthDesc));

		printLog("[GBufferPassRG] G-Buffer resources declared");
	}

	void GBufferPassRG::execute(const GBufferPassData& data, RHI* rhi, uint32_t frameIndex)
	{
		// TODO: 실제 G-Buffer 렌더링
		// 
		// 1. Render Pass 시작
		// 2. 파이프라인 바인딩
		// 3. MRT로 모든 G-Buffer 타겟에 렌더링
		//    - Albedo
		//    - Normal (World Space)
		//    - Position (World Space)
		//    - Metallic-Roughness
		// 4. Depth 기록
		// 5. Render Pass 종료

		// 예시:
		// rhi->cmdBindPipeline(pipeline_);
		// rhi->cmdSetViewport(...);
		// rhi->cmdSetScissor(...);
		// 
		// for (auto& mesh : scene.getMeshes()) {
		//     rhi->cmdBindVertexBuffer(mesh.vertexBuffer);
		//     rhi->cmdBindIndexBuffer(mesh.indexBuffer);
		//     rhi->cmdDrawIndexed(mesh.indexCount);
		// }
	}

	void GBufferPassRG::createPipeline()
	{
		// TODO: 파이프라인 생성
		// 
		// 1. 셰이더 로드
		//    - gbuffer.vert
		//    - gbuffer.frag (MRT 출력)
		// 
		// 2. 파이프라인 상태 설정
		// - Vertex Input
		//    - Input Assembly
		//- Rasterization (Cull Back)
		//    - Depth Test (Enabled)
		//    - Color Blend (4개 attachment)
		// 
		// 3. 파이프라인 생성
		//    pipeline_ = rhi_->createPipeline(...);
	}

	void GBufferPassRG::destroyPipeline()
	{
		if (pipeline_.isValid()) {
			rhi_->destroyPipeline(pipeline_);
			pipeline_ = {};
		}

		if (vertexShader_.isValid()) {
			rhi_->destroyShader(vertexShader_);
			vertexShader_ = {};
		}

		if (fragmentShader_.isValid()) {
			rhi_->destroyShader(fragmentShader_);
			fragmentShader_ = {};
		}
	}

} // namespace BinRenderer
