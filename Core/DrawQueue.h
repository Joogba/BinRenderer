#pragma once
#include "DrawCommand.h"
#include <vector>
#include <algorithm>
#include <functional>

namespace BinRenderer {

	class DrawQueue {
	public:
		void Clear() { m_commands.clear(); }
		void Submit(const DrawCommand& cmd) { m_commands.push_back(cmd); }
		const std::vector<DrawCommand>& GetCommands() const { return m_commands; }
		void flush(std::function<void(const DrawCommand&)> const& fn)
		{
			// 1) SortKey ���� �������� ����
			std::sort(m_commands.begin(), m_commands.end(),
				[](auto const& a, auto const& b) {
					return a.sortKey < b.sortKey;
				});
			// 2) ���ĵ� ������� �ݹ�
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
