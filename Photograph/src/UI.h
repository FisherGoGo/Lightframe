#pragma once

#include "ViewportRect.h"

struct GLFWwindow;
struct CameraSettings;
class Camera;

struct UIRequests
{
    bool resetPose = false;
    bool resetFocalLength = false;
    bool resetExperiment = false;
};

// 管理 ImGui 生命周期、参数控件与输入状态；不直接操作机位。
class UI
{
public:
    UI() = default;
    ~UI();

    UI(const UI&) = delete;
    UI& operator=(const UI&) = delete;
    UI(UI&&) = delete;
    UI& operator=(UI&&) = delete;

    bool initialize(GLFWwindow* window);
    void beginFrame(bool mouseLookActive);
    UIRequests draw(CameraSettings& settings, const Camera& camera);
    void render();

    const ViewportRect& getViewportRect() const { return viewport; }
    bool capturesMouse() const;
    bool capturesKeyboard() const;
    bool rightMousePressed() const;
    bool resetPosePressed() const;
    float mouseWheel() const;

private:
    bool contextCreated = false;
    bool glfwBackendInitialized = false;
    bool openglBackendInitialized = false;
    ViewportRect viewport;
};
