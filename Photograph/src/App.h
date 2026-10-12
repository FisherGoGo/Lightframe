#pragma once

#include <memory>
#include "Camera.h"
#include "Scene.h"

struct GLFWwindow;
class Mesh;
class Shader;

// 协调窗口、输入和每帧绘制；GPU 资源先于窗口上下文销毁。
class App
{
public:
    App();
    ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;
    App(App&&) = delete;
    App& operator=(App&&) = delete;

    int run(bool smokeTest = false);

private:
    bool initialize(bool smokeTest);
    int renderLoop(bool smokeTest);
    void processInput(float deltaSeconds);
    void stopMouseLook();
    void processMouseMovement();

    GLFWwindow* window = nullptr;
    bool glfwInitialized = false;
    std::unique_ptr<Shader> ourShader;
    std::unique_ptr<Mesh> cube;
    Camera camera;
    Scene scene;
    bool mouseLookActive = false;
    double lastCursorX = 0.0;
    double lastCursorY = 0.0;
};
