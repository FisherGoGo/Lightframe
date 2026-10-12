#pragma once

#include <glm/glm.hpp>
#include <vector>

struct SceneObject
{
    glm::mat4 model;
    glm::vec3 tint;
};

// 只保存实例变换与颜色；所有实例复用 App 拥有的单位立方体。
class Scene
{
public:
    Scene();
    const std::vector<SceneObject>& getObjects() const { return objects; }

private:
    std::vector<SceneObject> objects;
};
