#pragma once

#include <glm/glm.hpp>

struct CameraSettings;

// 世界单位为米；只管理机位与矩阵，不持有窗口或 GPU 资源。
class Camera
{
public:
    Camera();
    glm::mat4 viewMatrix() const;
    glm::mat4 projectionMatrix(const CameraSettings& settings) const;
    void move(glm::vec3 localDirection, float deltaSeconds);
    void rotate(float deltaX, float deltaY);
    void resetPose();

    const glm::vec3& getPosition() const { return position; }
    const glm::vec3& getFront() const { return front; }

private:
    void updateDirections();

    glm::vec3 position;
    glm::vec3 front, right, up;
    float yaw = -90.0f;
    float pitch = 0.0f;
    float speed = 2.5f;
    float sensitivity = 0.05f;
};
