#pragma once

#include <glm/glm.hpp>

namespace BinRenderer
{
    struct Light
    {
        glm::vec3 position{0.0f};
        float intensity{1.0f};
        glm::vec3 color{1.0f};
        float padding{0.0f};
    };
}
