#pragma once

#include <vector>
#include <algorithm>
#include <functional>
#include <unordered_map>

#include "DrawCommand.h"

namespace BinRenderer {

	class DrawQueue {
	public:
		void Clear() { m_commands.clear(); }
		void Submit(const DrawCommand& cmd) { m_commands.push_back(cmd); }
		const std::vector<DrawCommand>& GetCommands() const { return m_commands; }

        void Flush(const std::function<void(const DrawCommand&, const std::vector<glm::mat4>&, size_t)>& fn)
        {
            // 그룹핑: PSO+Material+Mesh 기준
            std::unordered_map<InstancingKey, std::vector<const DrawCommand*>> instancedGroups;

            for (const auto& cmd : m_commands) {
                InstancingKey key{ cmd.psoHandle, cmd.materialHandle, cmd.meshHandle };
                instancedGroups[key].push_back(&cmd);
            }

            // 그룹별로 DrawCall
            for (auto& pair : instancedGroups) {
                const auto& cmds = pair.second;
                if (cmds.size() == 1) {
                    // 일반 Draw
                    std::vector<glm::mat4> transforms{ cmds[0]->transform };
                    fn(*cmds[0], transforms, 1);
                }
                else {
                    // 인스턴싱 필요: transforms 배열 합치기
                    std::vector<glm::mat4> instanceTransforms;
                    for (auto* cmd : cmds)
                        instanceTransforms.push_back(cmd->transform);

                    fn(*cmds[0], instanceTransforms, instanceTransforms.size());
                }
            }
            m_commands.clear();
        }

	private:
		std::vector<DrawCommand> m_commands;
	};

}
