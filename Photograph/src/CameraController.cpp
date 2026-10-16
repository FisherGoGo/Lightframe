#include "CameraController.h"

#include <GLFW/glfw3.h>
#include "Camera.h"

void CameraController::update(GLFWwindow* window, Camera& camera, float deltaSeconds)
{
    // 与作业的 Update 一样，先转头，再沿本帧更新后的朝向移动。
    processMouseMovement(window, camera);

    glm::vec3 direction(0.0f);
    direction.z = static_cast<float>((glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        - (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS));
    direction.x = static_cast<float>((glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        - (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS));
    direction.y = static_cast<float>((glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        - (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS));
    camera.move(direction, deltaSeconds);
}

void CameraController::startMouseLook(GLFWwindow* window)
{
    mouseLookActive = true;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // 与作业的 Attach 一样，捕获后立即读取位置作为基准。
    glfwGetCursorPos(window, &lastCursorX, &lastCursorY);
}

void CameraController::stopMouseLook(GLFWwindow* window)
{
    mouseLookActive = false;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void CameraController::resetPose(GLFWwindow* window, Camera& camera)
{
    camera.resetPose();
    stopMouseLook(window);
}

void CameraController::processMouseMovement(GLFWwindow* window, Camera& camera)
{
    if (!mouseLookActive) { return; }

    double x, y;
    glfwGetCursorPos(window, &x, &y);
    float deltaX = static_cast<float>(x - lastCursorX);
    float deltaY = static_cast<float>(lastCursorY - y);
    lastCursorX = x;
    lastCursorY = y;
    camera.rotate(deltaX, deltaY);
}
