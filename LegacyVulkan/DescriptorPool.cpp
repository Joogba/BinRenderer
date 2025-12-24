#include "DescriptorPool.h"
#include "Logger.h"
#include <unordered_map>
#include <iostream>
#include <algorithm>

namespace BinRenderer
{
	namespace Vulkan
	{
        DescriptorPool::DescriptorPool(VkDevice& device) : device_(device)
        {
        }

        DescriptorPool::~DescriptorPool()
        {
            cleanup();
        }

        void DescriptorPool::createFromScript()
        {
            // "DescriptorPoolSize.txt" 파일이 존재하면 읽어서 createNewPool 호출
            std::ifstream file(kScriptFilename_);

            if (file.is_open()) {
                printLog("Found DescriptorPoolSize.txt, loading previous statistics...");

                string line;
                vector<VkDescriptorPoolSize> poolSizes;
                uint32_t numSets = 0;

                while (std::getline(file, line)) {
                    std::istringstream iss(line);
                    string typeStr;
                    uint32_t count;

                    if (iss >> typeStr >> count) {
                        if (typeStr == "NumSets") {
                            numSets = count;
                        }
                        else {
                            // VulkanTools 함수를 사용하여 문자열을 VkDescriptorType으로 변환
                            VkDescriptorType type = stringToDescriptorType(typeStr);
                            if (type != VK_DESCRIPTOR_TYPE_MAX_ENUM) {
                                VkDescriptorPoolSize poolSize{};
                                poolSize.type = type;

                                // ========================================
                                // FIX: Cap the descriptor count to prevent memory exhaustion
                                // ========================================
                                const uint32_t kMaxReasonableDescriptors = 256; // Reasonable limit
                                poolSize.descriptorCount = std::min(count, kMaxReasonableDescriptors);

                                if (count > kMaxReasonableDescriptors) {
                                    printLog("WARNING: Capped {} from {} to {} (was too large)",
                                        typeStr, count, kMaxReasonableDescriptors);
                                }

                                poolSizes.push_back(poolSize);
                            }
                        }
                    }
                }
                file.close();

                // 로드된 통계로 풀 생성 (유효한 데이터가 있는 경우)
                if (numSets > 0 && !poolSizes.empty()) {
                    // Cap number of sets as well
                    const uint32_t kMaxReasonableSets = 20;
                    uint32_t cappedNumSets = std::min(numSets, kMaxReasonableSets);

                    if (numSets > kMaxReasonableSets) {
                        printLog("WARNING: Capped NumSets from {} to {}", numSets, cappedNumSets);
                    }

                    createNewPool(poolSizes, cappedNumSets);
                    printLog("Created initial pool with {} sets and {} descriptor types", cappedNumSets,
                        poolSizes.size());
                }
            }
            else {
                printLog("DescriptorPoolSize.txt not found, will create pools on-demand");
            }
        }

        bool DescriptorPool::canAllocateFromRemaining(
            const unordered_map<VkDescriptorType, uint32_t>& requiredTypeCounts,
            uint32_t numRequiredSets) const
        {
            if (descriptorPools_.empty()) {
                return false;
            }

            if (remainingSets_ < numRequiredSets) {
                return false;
            }

            for (const auto& [type, required] : requiredTypeCounts) {
                auto it = remainingTypeCounts_.find(type);
                if (it == remainingTypeCounts_.end()) {
                    return false;
                }
                if (it->second < required) {
                    return false;
                }
            }

            return true;
        }

        void DescriptorPool::createNewPool(const vector<VkDescriptorPoolSize>& typeCounts, uint32_t maxSets)
        {
            // 제공된 VkDescriptorPoolSize 벡터를 사용하여 디스크립터 풀을 직접 생성
            VkDescriptorPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
            poolInfo.flags = 0;
            poolInfo.maxSets = maxSets;
            poolInfo.poolSizeCount = static_cast<uint32_t>(typeCounts.size());
            poolInfo.pPoolSizes = typeCounts.data();

            VkDescriptorPool newPool = VK_NULL_HANDLE;
            check(vkCreateDescriptorPool(device_, &poolInfo, nullptr, &newPool));

            descriptorPools_.push_back(newPool);

            remainingSets_ = maxSets;
            for (const auto& poolSize : typeCounts) {
                remainingTypeCounts_[poolSize.type] = poolSize.descriptorCount;
            }
        }

        void DescriptorPool::updateRemainingCapacity(const vector<VkDescriptorSetLayoutBinding>& bindings,
            uint32_t numSets)
        {
            // 남은 세트 수 업데이트
            remainingSets_ -= numSets;

            // 남은 타입 카운트 업데이트
            for (const auto& binding : bindings) {
                remainingTypeCounts_[binding.descriptorType] -= binding.descriptorCount * numSets;
            }
        }

        VkDescriptorSet
            DescriptorPool::allocateDescriptorSet(const VkDescriptorSetLayout& descriptorSetLayout)
        {
            // 1. descriptorSetLayouts에서 바인딩 검색
            const auto& bindings = layoutToBindings(descriptorSetLayout);

            // 이 할당에 필요한 디스크립터 카운트 계산
            unordered_map<VkDescriptorType, uint32_t> requiredTypeCounts;
            for (const auto& binding : bindings) {
                requiredTypeCounts[binding.descriptorType] += binding.descriptorCount;
            }

            // 2. remainingSet 및 remainingTypeCounts가 충분한지 확인
            if (!canAllocateFromRemaining(requiredTypeCounts, 1)) {
                // 3. 정확한 풀 생성을 위해 VkDescriptorPoolSize 벡터로 변환
                vector<VkDescriptorPoolSize> poolSizes;
                poolSizes.reserve(requiredTypeCounts.size());

                for (const auto& [type, count] : requiredTypeCounts) {
                    VkDescriptorPoolSize poolSize{};
                    poolSize.type = type;
                    poolSize.descriptorCount = count;
                    poolSizes.push_back(poolSize);
                }

                createNewPool(poolSizes, 1);
            }

            // 4. 바인딩에 가변 디스크립터 카운트가 있는지 확인
            bool hasVariableDescriptorCount = false;
            uint32_t variableDescriptorCount = 0;
            uint32_t highestBinding = 0;

            for (const auto& binding : bindings) {
                if (binding.binding > highestBinding) {
                    highestBinding = binding.binding;
                }
            }

            for (const auto& binding : bindings) {
                if (binding.descriptorCount > 1 && binding.binding == highestBinding) {
                    hasVariableDescriptorCount = true;
                    variableDescriptorCount = binding.descriptorCount;
                    break;
                }
            }

            // 4. 마지막 풀을 사용하여 할당
            VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
            allocInfo.descriptorPool = descriptorPools_.back();
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &descriptorSetLayout;

            // 새로 추가: 필요한 경우 가변 디스크립터 카운트 정보 추가
            VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
            if (hasVariableDescriptorCount) {
                variableCountInfo.sType =
                    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
                variableCountInfo.descriptorSetCount = 1;
                variableCountInfo.pDescriptorCounts = &variableDescriptorCount;
                allocInfo.pNext = &variableCountInfo;

                printLog("Allocating descriptor set with variable count: {}", variableDescriptorCount);
            }

            VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
            VkResult res = vkAllocateDescriptorSets(device_, &allocInfo, &descriptorSet);
            if (res == VK_ERROR_OUT_OF_POOL_MEMORY) {
                exitWithMessage("Unexpected VK_ERROR_OUT_OF_POOL_MEMORY after pool creation");
            }
            check(res);

            // 5. remainingSet, remainingTypeCounts, allocatedSets, allocatedTypeCounts 업데이트
            updateRemainingCapacity(bindings, 1);
            allocatedSets_++;

            for (const auto& binding : bindings) {
                allocatedTypeCounts_[binding.descriptorType] += binding.descriptorCount;
            }

            return descriptorSet;
        }

        auto DescriptorPool::allocateDescriptorSets(
            const vector<VkDescriptorSetLayout>& descriptorSetLayouts) -> vector<VkDescriptorSet>
        {
            // 모든 레이아웃에 대한 총 필요 디스크립터 카운트 계산
            unordered_map<VkDescriptorType, uint32_t> totalRequiredTypeCounts;
            for (const auto& layout : descriptorSetLayouts) {
                const auto& bindings = layoutToBindings(layout);
                for (const auto& binding : bindings) {
                    totalRequiredTypeCounts[binding.descriptorType] += binding.descriptorCount;
                }
            }

            // 남은 용량에서 모든 세트를 할당할 수 있는지 확인
            uint32_t numRequiredSets = static_cast<uint32_t>(descriptorSetLayouts.size());
            if (!canAllocateFromRemaining(totalRequiredTypeCounts, numRequiredSets)) {
                // 정확한 풀 생성을 위해 VkDescriptorPoolSize 벡터로 변환
                vector<VkDescriptorPoolSize> poolSizes;
                poolSizes.reserve(totalRequiredTypeCounts.size());

                for (const auto& [type, count] : totalRequiredTypeCounts) {
                    VkDescriptorPoolSize poolSize{};
                    poolSize.type = type;
                    poolSize.descriptorCount = count;
                    poolSizes.push_back(poolSize);
                }

                createNewPool(poolSizes, numRequiredSets);
            }

            // 새로 추가: 레이아웃에 가변 디스크립터 카운트가 있는지 확인
            vector<uint32_t> variableDescriptorCounts;
            bool hasAnyVariableCount = false;

            for (const auto& layout : descriptorSetLayouts) {
                const auto& bindings = layoutToBindings(layout);

                bool hasVariableCount = false;
                uint32_t variableCount = 0;
                uint32_t highestBinding = 0;

                for (const auto& binding : bindings) {
                    if (binding.binding > highestBinding) {
                        highestBinding = binding.binding;
                    }
                }

                for (const auto& binding : bindings) {
                    if (binding.descriptorCount > 1 && binding.binding == highestBinding) {
                        hasVariableCount = true;
                        hasAnyVariableCount = true;
                        variableCount = binding.descriptorCount;
                        break;
                    }
                }

                variableDescriptorCounts.push_back(variableCount);
            }

            // 마지막 풀을 사용하여 할당
            VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
            allocInfo.descriptorPool = descriptorPools_.back();
            allocInfo.descriptorSetCount = numRequiredSets;
            allocInfo.pSetLayouts = descriptorSetLayouts.data();

            // 새로 추가: 필요한 경우 가변 디스크립터 카운트 정보 추가
            VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
            if (hasAnyVariableCount) {
                variableCountInfo.sType =
                    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
                variableCountInfo.descriptorSetCount = numRequiredSets;
                variableCountInfo.pDescriptorCounts = variableDescriptorCounts.data();
                allocInfo.pNext = &variableCountInfo;

                printLog("Allocating {} descriptor sets with variable counts", numRequiredSets);
            }

            vector<VkDescriptorSet> descriptorSets(allocInfo.descriptorSetCount, VK_NULL_HANDLE);
            VkResult res = vkAllocateDescriptorSets(device_, &allocInfo, descriptorSets.data());
            if (res == VK_ERROR_OUT_OF_POOL_MEMORY) {
                exitWithMessage("Unexpected VK_ERROR_OUT_OF_POOL_MEMORY after pool creation");
            }
            check(res);

            // 남은 용량 업데이트
            for (const auto& layout : descriptorSetLayouts) {
                const auto& bindings = layoutToBindings(layout);
                updateRemainingCapacity(bindings, 1);
            }

            // 할당된 세트 추적
            allocatedSets_ += numRequiredSets;

            // 이 레이아웃에서 사용된 디스크립터 타입 추적
            for (const auto& layout : descriptorSetLayouts) {
                const auto& bindings = layoutToBindings(layout);
                for (const auto& binding : bindings) {
                    allocatedTypeCounts_[binding.descriptorType] += binding.descriptorCount;
                }
            }

            return descriptorSets;
        }

        void DescriptorPool::createLayouts(const vector<LayoutInfo>& layoutInfos)
        {
            for (const auto& l : layoutInfos) {
                VkDescriptorSetLayoutCreateInfo createInfo{
                    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
                createInfo.bindingCount = static_cast<uint32_t>(l.bindings_.size());
                createInfo.pBindings = l.bindings_.data();

                // 가장 높은 바인딩을 찾기 위해 바인딩 번호로 정렬
                auto sortedBindings = l.bindings_;
                std::sort(sortedBindings.begin(), sortedBindings.end(),
                    [](const VkDescriptorSetLayoutBinding& a, const VkDescriptorSetLayoutBinding& b) {
                        return a.binding < b.binding;
                    });

                for (int i = 0; i < l.bindings_.size(); i++) {
                    if (l.bindings_[i].binding != i) {
                        exitWithMessage("binding index mismatch {} vs {}", i, l.bindings_[i].binding);
                    }
                }

                bool needsPartiallyBound = false;
                bool needsVariableCount = false;
                uint32_t highestBinding = sortedBindings.empty() ? 0 : sortedBindings.back().binding;

                // 배열에 대한 각 바인딩 확인
                for (const auto& binding : sortedBindings) {
                    if (binding.descriptorCount > 1) {
                        needsPartiallyBound = true;

                        // 가변 카운트는 가장 높은 바인딩 번호에만 허용됨
                        if (binding.binding == highestBinding) {
                            needsVariableCount = true;
                            printLog("    Binding {} is variable-length array (count={})", binding.binding,
                                binding.descriptorCount);
                        }
                        else {
                            printLog("    Binding {} is fixed-length array (count={}) - not last binding",
                                binding.binding, binding.descriptorCount);
                        }
                    }
                }

                // 레이아웃 플래그 설정 - 잘못된 플래그 제거
                VkDescriptorSetLayoutCreateFlags flags = 0;
                // 참고: VkDescriptorSetLayoutCreateFlagBits는
                // PARTIALLY_BOUND 또는 VARIABLE_DESCRIPTOR_COUNT에 해당하는 항목이 없음 - 바인딩 플래그에만 적용됨
                createInfo.flags = flags;

                // 바인딩별 플래그 설정
                vector<VkDescriptorBindingFlags> bindingFlags;
                VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};

                if (needsPartiallyBound || needsVariableCount) {
                    bindingFlags.resize(l.bindings_.size(), 0);

                    for (size_t i = 0; i < l.bindings_.size(); ++i) {
                        if (l.bindings_[i].descriptorCount > 1) {
                            bindingFlags[i] |= VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
                            bindingFlags[i] |= VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;

                            // 가변 카운트는 가장 높은 바인딩 번호에만 적용
                            if (l.bindings_[i].binding == highestBinding) {
                                bindingFlags[i] |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
                            }
                        }
                    }

                    bindingFlagsInfo.sType =
                        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
                    bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
                    bindingFlagsInfo.pBindingFlags = bindingFlags.data();
                    createInfo.pNext = &bindingFlagsInfo;
                }

                VkDescriptorSetLayout layout{ VK_NULL_HANDLE };
                check(vkCreateDescriptorSetLayout(device_, &createInfo, nullptr, &layout));
                layoutsAndInfos_.push_back({ layout, l });
            }

            // 디버그 정보 출력
            printLog("ShaderManager: Created {} unique layout(s)", layoutInfos.size());
            for (size_t i = 0; i < layoutInfos.size(); ++i) {
                const auto& info = layoutInfos[i];
                const auto& layout = std::get<0>(layoutsAndInfos_[i]);
                printLog("  Layout {} (0x{:x}): {} binding(s), used by:", i,
                    reinterpret_cast<uintptr_t>(layout), info.bindings_.size());
                for (const auto& [pipelineName, setNumber] : info.pipelineNamesAndSetNumbers_) {
                    printLog("    - Pipeline '{}', Set {}", pipelineName, setNumber);
                }

                // 바인딩 상세 정보 출력
                for (size_t j = 0; j < info.bindings_.size(); ++j) {
                    const auto& binding = info.bindings_[j];
                    printLog("    Binding {}: type={}, count={}, stages={}", binding.binding,
                        descriptorTypeToString(binding.descriptorType), binding.descriptorCount,
                        shaderStageFlagsToString(binding.stageFlags));
                }
            }
        }

        auto DescriptorPool::descriptorSetLayout(const vector<VkDescriptorSetLayoutBinding>& bindings)
            -> const VkDescriptorSetLayout&
        {
            // layoutsAndInfos_에서 바인딩 검색
            for (const auto& [layout, layoutInfo] : layoutsAndInfos_) {
                if (BindingEqual{}(layoutInfo.bindings_,
                    bindings)) { // VulkanTools.h에 정의된 연산자를 명시적으로 사용
                    return layout;
                }
            }

            if (bindings.size() == 0) {
                printLog("Empty bindings provided.");
            }

            for (const auto& binding : bindings) {
                printLog("    Binding {}: type={}, count={}, stages={}", binding.binding,
                    descriptorTypeToString(binding.descriptorType), binding.descriptorCount,
                    shaderStageFlagsToString(binding.stageFlags));
            }

            exitWithMessage(
                "Failed to find descriptor set layout for the given bindings in layoutsAndInfos_");

            // 컴파일러를 위해 필요하지만 여기에 도달해서는 안 됨
            static const VkDescriptorSetLayout empty = VK_NULL_HANDLE;
            return empty;
        }

        auto DescriptorPool::layoutToBindings(const VkDescriptorSetLayout& layout)
            -> const vector<VkDescriptorSetLayoutBinding>&
        {
            // layoutAndInfos_에서 레이아웃 검색
            for (const auto& [storedLayout, layoutInfo] : layoutsAndInfos_) {
                if (storedLayout == layout) {
                    return layoutInfo.bindings_;
                }
            }

            printLog("Failed to find descriptor set layout in layoutAndInfos_: {}",
                reinterpret_cast<uintptr_t>(layout));

            exitWithMessage("Failed to find descriptor set layout in layoutAndInfos_");

            static vector<VkDescriptorSetLayoutBinding> empty;
            return empty;
        }

        void DescriptorPool::printAllocatedStatistics() const
        {
            printLog("\n=== DescriptorPool 할당 통계 ===");

            printLog("생성된 총 풀 수: {}", descriptorPools_.size());
            printLog("할당된 총 세트 수: {}", allocatedSets_);

            if (!allocatedTypeCounts_.empty()) {
                for (const auto& [type, count] : allocatedTypeCounts_) {
                    printLog("  {}: {}", descriptorTypeToString(type), count);
                }
            }
            else {
                printLog("\n할당된 디스크립터 타입이 없습니다.");
            }

            printLog("============================================\n");
        }

        void DescriptorPool::cleanup()
        {
            if (descriptorPools_.size() > 0) {
                std::ofstream file(kScriptFilename_);
                if (file.is_open()) {
                    // ========================================
                    // FIX: Cap statistics to prevent future memory issues
                    // ========================================
                    const uint32_t kMaxReasonableSets = 20;
                    const uint32_t kMaxReasonableDescriptors = 256;
    
                    uint32_t cappedSets = std::min(allocatedSets_, kMaxReasonableSets);
    
                    // 먼저 세트 수 작성 (들여쓰기 없음)
                    file << "NumSets " << cappedSets << "\n";

                    // 할당된 각 디스크립터 타입과 카운트 작성 (들여쓰기 없음)
                    for (const auto& [type, count] : allocatedTypeCounts_) {
                        uint32_t cappedCount = std::min(count, kMaxReasonableDescriptors);
                        file << descriptorTypeToString(type) << " " << cappedCount << "\n";
                    }

                    file.close();
                    printLog("Saved descriptor pool statistics to DescriptorPoolSize.txt");
                    printLog("  (Capped to reasonable limits: {} sets, {} descriptors per type)",
 kMaxReasonableSets, kMaxReasonableDescriptors);
                }
                else {
                    printLog("Warning: Could not write to DescriptorPoolSize.txt");
                }
                printAllocatedStatistics();
            }

            for (auto& p : descriptorPools_) {
                if (p != VK_NULL_HANDLE) {
                    vkDestroyDescriptorPool(device_, p, nullptr);
                }
            }
            descriptorPools_.clear();

            // layoutAndBindings_에서 레이아웃 제거
            for (auto& [layout, bindings] : layoutsAndInfos_) {
                if (layout != VK_NULL_HANDLE) {
                    vkDestroyDescriptorSetLayout(device_, layout, nullptr);
                }
            }
            layoutsAndInfos_.clear();
        }
	}
}