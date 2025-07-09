#pragma once

#include <vector>
#include <algorithm>
#include <functional>

#include "DrawCommand.h"

namespace BinRenderer {

	class DrawQueue {
	public:
		void Clear() { m_commands.clear(); }
		void Submit(const DrawCommand& cmd) { m_commands.push_back(cmd); }
		const std::vector<DrawCommand>& GetCommands() const { return m_commands; }
		void flush(std::function<void(const DrawCommand&)> const& fn)
		{
			// 1) SortKey 기준 오름차순 정렬
			std::sort(m_commands.begin(), m_commands.end(),
				[](auto const& a, auto const& b) {
					return a.sortKey < b.sortKey;
				});
			// 2) 정렬된 순서대로 콜백
			for (auto const& cmd : m_commands)
			{
				fn(cmd);
			}
			m_commands.clear();
		}

	private:
		std::vector<DrawCommand> m_commands;
	};

}
