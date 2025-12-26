#include "RGGraph.h"
#include <algorithm>
#include <queue>
#include <iostream>

namespace BinRenderer
{
	RenderGraph::RenderGraph(RHI* rhi)
		: rhi_(rhi)
	{
	}

	RenderGraph::~RenderGraph()
	{
		deallocateResources();
	}

	// ========================================
	// 컴파일
	// ========================================

	void RenderGraph::compile()
	{
		if (compiled_) {
			return;
		}

		// 1. 사용되지 않는 패스 제거
		cullUnusedPasses();

		// 2. Topological Sort로 실행 순서 결정
		topologicalSort();

		// 3. 리소스 할당
		allocateResources();

		compiled_ = true;
	}

	// ========================================
	// 실행
	// ========================================

	void RenderGraph::execute(uint32_t frameIndex)
	{
		if (!compiled_) {
			std::cerr << "[RenderGraph] Error: Graph not compiled!" << std::endl;
			return;
		}

		// 정렬된 순서대로 패스 실행
		for (auto* pass : sortedPasses_) {
			pass->execute(rhi_, frameIndex);
		}
	}

	void RenderGraph::reset()
	{
		passes_.clear();
		sortedPasses_.clear();
		builder_.textures_.clear();
		builder_.buffers_.clear();
		deallocateResources();
		compiled_ = false;
	}

	// ========================================
	// 리소스 접근
	// ========================================

	RHIImageHandle RenderGraph::getFinalOutput() const
	{
		auto handle = builder_.getFinalOutput();
		return getTexture(handle);
	}

	RHIImageHandle RenderGraph::getTexture(RGTextureHandle handle) const
	{
		if (!handle.isValid()) {
			return {};
		}

		auto it = allocatedTextures_.find(handle.index);
		if (it != allocatedTextures_.end()) {
			return it->second;
		}

		// Imported 리소스 확인
		if (handle.index < builder_.textures_.size()) {
			return builder_.textures_[handle.index].importedImage;
		}

		return {};
	}

	RHIBufferHandle RenderGraph::getBuffer(RGBufferHandle handle) const
	{
		if (!handle.isValid()) {
			return {};
		}

		auto it = allocatedBuffers_.find(handle.index);
		if (it != allocatedBuffers_.end()) {
			return it->second;
		}

		// Imported 리소스 확인
		if (handle.index < builder_.buffers_.size()) {
			return builder_.buffers_[handle.index].importedBuffer;
		}

		return {};
	}

	// ========================================
	// 디버그
	// ========================================

	void RenderGraph::printExecutionOrder() const
	{
		std::cout << "[RenderGraph] Execution Order:" << std::endl;
		for (size_t i = 0; i < sortedPasses_.size(); ++i) {
			std::cout << "  " << i << ": " << sortedPasses_[i]->getName() << std::endl;
		}
	}

	void RenderGraph::printResourceUsage() const
	{
		std::cout << "[RenderGraph] Resource Usage:" << std::endl;
		std::cout << "  Textures: " << builder_.textures_.size() << std::endl;
		std::cout << "  Buffers: " << builder_.buffers_.size() << std::endl;
	}

	// ========================================
	// 내부 헬퍼 함수
	// ========================================

	void RenderGraph::topologicalSort()
	{
		sortedPasses_.clear();

		if (passes_.empty()) {
			return;
		}

		// 각 패스의 진입 차수(indegree) 계산
		std::vector<int> indegree(passes_.size(), 0);
		std::vector<std::vector<int>> adjList(passes_.size());

		// 의존성 그래프 구축
		for (size_t i = 0; i < passes_.size(); ++i) {
			const auto& pass = passes_[i];
			
			for (const auto& dep : pass->getDependencies()) {
				// Read 의존성: 이전에 Write한 패스를 찾음
				if (dep.accessType == RGResourceAccessType::Read || 
				    dep.accessType == RGResourceAccessType::ReadWrite) {
					
					for (size_t j = 0; j < i; ++j) {
						const auto& prevPass = passes_[j];
						
						for (const auto& prevDep : prevPass->getDependencies()) {
							if (prevDep.accessType == RGResourceAccessType::Write ||
							    prevDep.accessType == RGResourceAccessType::ReadWrite) {
								
								bool sameResource = false;
								if (dep.isTexture && prevDep.isTexture && dep.texture == prevDep.texture) {
									sameResource = true;
								} else if (!dep.isTexture && !prevDep.isTexture && dep.buffer == prevDep.buffer) {
									sameResource = true;
								}

								if (sameResource) {
									adjList[j].push_back(static_cast<int>(i));
									indegree[i]++;
								}
							}
						}
					}
				}
			}
		}

		// Kahn's Algorithm을 사용한 Topological Sort
		std::queue<int> q;
		for (size_t i = 0; i < passes_.size(); ++i) {
			if (indegree[i] == 0) {
				q.push(static_cast<int>(i));
			}
		}

		while (!q.empty()) {
			int idx = q.front();
			q.pop();

			sortedPasses_.push_back(passes_[idx].get());
			passes_[idx]->setExecutionOrder(static_cast<uint32_t>(sortedPasses_.size()) - 1);

			for (int next : adjList[idx]) {
				indegree[next]--;
				if (indegree[next] == 0) {
					q.push(next);
				}
			}
		}

		// 순환 의존성 체크
		if (sortedPasses_.size() != passes_.size()) {
			std::cerr << "[RenderGraph] Error: Circular dependency detected!" << std::endl;
			sortedPasses_.clear();
		}
	}

	void RenderGraph::allocateResources()
	{
		// 텍스처 할당
		for (size_t i = 0; i < builder_.textures_.size(); ++i) {
			const auto& node = builder_.textures_[i];
			
			// Imported 리소스는 건너뜀
			if (node.desc.isImported) {
				continue;
			}

			// RHI를 통해 텍스처 생성
			RHIImageCreateInfo createInfo{};
			createInfo.width = node.desc.width;
			createInfo.height = node.desc.height;
			createInfo.depth = node.desc.depth;
			createInfo.mipLevels = node.desc.mipLevels;
			createInfo.arrayLayers = node.desc.arrayLayers;
			createInfo.format = node.desc.format;
			createInfo.samples = node.desc.samples;
			createInfo.usage = node.desc.usage;

			RHIImageHandle image = rhi_->createImage(createInfo);
			allocatedTextures_[static_cast<uint32_t>(i)] = image;
		}

		// 버퍼 할당
		for (size_t i = 0; i < builder_.buffers_.size(); ++i) {
			const auto& node = builder_.buffers_[i];
			
			// Imported 리소스는 건너뜀
			if (node.desc.isImported) {
				continue;
			}

			// RHI를 통해 버퍼 생성
			RHIBufferCreateInfo createInfo{};
			createInfo.size = node.desc.size;
			createInfo.usage = node.desc.usage;

			RHIBufferHandle buffer = rhi_->createBuffer(createInfo);
			allocatedBuffers_[static_cast<uint32_t>(i)] = buffer;
		}
	}

	void RenderGraph::deallocateResources()
	{
		// 할당된 텍스처 해제
		for (auto& pair : allocatedTextures_) {
			if (pair.second.isValid()) {
				rhi_->destroyImage(pair.second);
			}
		}
		allocatedTextures_.clear();

		// 할당된 버퍼 해제
		for (auto& pair : allocatedBuffers_) {
			if (pair.second.isValid()) {
				rhi_->destroyBuffer(pair.second);
			}
		}
		allocatedBuffers_.clear();
	}

	void RenderGraph::cullUnusedPasses()
	{
		// 최종 출력과 연결되지 않은 패스 제거
		auto finalOutput = builder_.getFinalOutput();
		if (!finalOutput.isValid()) {
			// 최종 출력이 없으면 모든 패스 유지
			return;
		}

		// TODO: 더 정교한 Culling 알고리즘 구현
		// 현재는 모든 패스를 유지
	}

	void RenderGraph::addPass(std::unique_ptr<RGPassBase> pass)
	{
		// Builder에 현재 패스 설정
		builder_.currentPass_ = pass.get();
		builder_.currentPassIndex_ = static_cast<uint32_t>(passes_.size());

		// Setup 단계 실행 (리소스 선언 및 의존성 등록)
		pass->setup(builder_);

		passes_.push_back(std::move(pass));
	}

} // namespace BinRenderer
