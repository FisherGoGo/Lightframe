#include "Camera.h"
#include "CameraSettings.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

Camera::Camera()
{
    resetPose();
}

void Camera::resetPose()
{
    // 沿用 P01 的机位，朝向原点的前景主体。
    position = glm::vec3(0.0f, 1.2f, 3.0f);
    yaw = -90.0f;
    pitch = glm::degrees(std::atan2(-1.2f, 3.0f));
    updateDirections();
}

void Camera::updateDirections()
{
    float yawRadians = glm::radians(yaw);
    float pitchRadians = glm::radians(pitch);
    front = glm::normalize(glm::vec3(
        std::cos(yawRadians) * std::cos(pitchRadians), std::sin(pitchRadians),
        std::sin(yawRadians) * std::cos(pitchRadians)));
    right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
    up = glm::normalize(glm::cross(right, front));
}

glm::mat4 Camera::viewMatrix() const
{
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::projectionMatrix(const CameraSettings& settings) const
{
    // 焦距仅改变投影；窗口尺寸与机位不参与照片视场角计算。
    return glm::perspective(settings.verticalFovRadians(), settings.aspectRatio(), 0.1f, 100.0f);
}

void Camera::move(glm::vec3 localDirection, float deltaSeconds)
{
    glm::vec3 direction = right * localDirection.x
        + glm::vec3(0.0f, 1.0f, 0.0f) * localDirection.y + front * localDirection.z;
    float length = glm::length(direction);
    if (length > 0.0f && deltaSeconds > 0.0f)
    {
        // 组合后归一化，斜向及上升+前进不会加速。
        position += direction / length * speed * deltaSeconds;
    }
}

void Camera::rotate(float deltaX, float deltaY)
{
    yaw += deltaX * sensitivity;
    pitch = std::clamp(pitch + deltaY * sensitivity, -89.0f, 89.0f);
    updateDirections();
}
