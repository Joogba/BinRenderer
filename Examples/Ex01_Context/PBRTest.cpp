#include "Vulkan/Application.h"

using namespace BinRenderer::Vulkan;

int main()
{
    ApplicationConfig config;

   /* config.models.push_back(ModelConfig("models/DamagedHelmet.glb", "Helmet")
        .setTransform(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f),
            glm::vec3(1.0f, 0.0f, 0.0f))));

    config.camera = config.camera.forHelmet();*/

    config.models.push_back(
        ModelConfig("characters/Leonard/Bboy Hip Hop Move.fbx", "Dancer")
        .setTransform(glm::rotate(
            glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-6.719f, 0.21f, -1.860f)),
                glm::vec3(0.012f)),
            glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)))
        .setAnimation(true, 0, 1.0f, true));

    config.models.push_back(
        ModelConfig("models/AmazonLumberyardBistroMorganMcGuire/exterior.obj", "Bistro")
        .setBistroModel(true)
        .setTransform(glm::scale(glm::mat4(1.0f), glm::vec3(0.01f))));

    config.camera = CameraConfig::forBistro();

    auto app = std::make_unique<Application>(config);

    app->run();
    return 0;
}
