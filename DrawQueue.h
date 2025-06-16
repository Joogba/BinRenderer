#pragma once
#include "DrawCommand.h"
#include <vector>

namespace BinRenderer {

    class DrawQueue {
    public:
        void Clear() { m_commands.clear(); }
        void Submit(const DrawCommand& cmd) { m_commands.push_back(cmd); }
        const std::vector<DrawCommand>& GetCommands() const { return m_commands; }

    private:
        std::vector<DrawCommand> m_commands;
    };

}
