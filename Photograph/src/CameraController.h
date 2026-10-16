#pragma once

struct GLFWwindow;
class Camera;

// 管理相机键鼠操作；窗口与 Camera 由 App 持有并在调用时传入。
class CameraController
{
public:
    // App 确认允许相机输入后调用；先转向，再沿本帧朝向移动。
    void update(GLFWwindow* window, Camera& camera, float deltaSeconds);
    void startMouseLook(GLFWwindow* window);
    void stopMouseLook(GLFWwindow* window);
    void resetPose(GLFWwindow* window, Camera& camera);
    bool isMouseLookActive() const { return mouseLookActive; }

private:
    void processMouseMovement(GLFWwindow* window, Camera& camera);

    bool mouseLookActive = false;
    double lastCursorX = 0.0;
    double lastCursorY = 0.0;
};
