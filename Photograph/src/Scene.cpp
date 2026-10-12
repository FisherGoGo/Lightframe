#include "Scene.h"

#include <glm/gtc/matrix_transform.hpp>

Scene::Scene()
{
    auto addCube = [this](glm::vec3 position, glm::vec3 scale, float angle, glm::vec3 tint)
    {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, scale);
        objects.push_back({ model, tint });
    };

    // 地面上表面 y=-0.5，与前景主体底面相接；背景依次远离相机。
    addCube({0.0f, -0.6f, -3.0f}, {16.0f, 0.2f, 18.0f}, 0.0f, {0.35f, 0.55f, 0.70f});
    addCube({0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, -25.0f, {1.0f, 1.0f, 1.0f});
    addCube({-1.7f, 0.3f, -3.0f}, {1.1f, 1.6f, 1.1f}, 15.0f, {0.45f, 1.0f, 1.0f});
    addCube({2.5f, 0.8f, -6.0f}, {1.6f, 2.6f, 1.6f}, -10.0f, {0.65f, 0.75f, 1.0f});
    addCube({-3.0f, 1.1f, -9.0f}, {1.8f, 3.2f, 1.8f}, 0.0f, {0.50f, 0.85f, 0.65f});
}
