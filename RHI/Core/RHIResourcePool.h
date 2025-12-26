#pragma once
#include <vector>
#include <queue>

#include "RHIHandle.h"

namespace BinRenderer
{
	template<typename T, typename HandleType>
	class RHIResourcePool
	{
	private:
		struct Slot
		{
			T* resource = nullptr;
			uint32_t generation = 1; // 0은 무효
		};
		std::vector<Slot> slots;
		std::queue<uint32_t> freeIndices; // 빈 슬롯 인덱스 큐

	public:
		HandleType insert(T* resource) {
			uint32_t index;
			uint32_t generation;

			if (freeIndices.empty()) {
				// 빈자리가 없어서 뒤에 추가
				index = static_cast<uint32_t>(slots.size());
				generation = 1; // 처음이니까 1세대

				slots.push_back({ resource, generation });
			}
			else {
				//빈자리 재활용
				index = freeIndices.front();
				freeIndices.pop();

				// 기존 슬롯에 저장되어 있던 Generation을 그대로 사용!
				// (삭제될 때 이미 +1 되어있는 상태임)
				generation = slots[index].generation;

				slots[index].resource = resource;
			}

			// 핸들 생성!
			return HandleType(index, generation);
		}

		T* get(HandleType handle)
		{
			uint32_t index = handle.getIndex();
			if (index >= slots.size())
				return nullptr;
			if (slots[index].generation != handle.getGeneration())
				return nullptr; // 세대 불일치
			return slots[index].resource;
		}
		
		void remove(HandleType handle)
		{
			T* res = get(handle);
			if (!res) return; 					
			delete res;

			uint32_t index = handle.getIndex();
			if (index >= slots.size())
				return;
			if (slots[index].generation != handle.getGeneration())
				return; // 세대 불일치

			slots[index].resource = nullptr;
			slots[index].generation++;// 세대 증가
			freeIndices.push(index);// 빈 슬롯 인덱스 큐에 추가
		}
	};
}