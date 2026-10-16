#include "App.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <algorithm>
#include <iostream>
#include "Shader.h"
#include "Mesh.h"
#include "UI.h"

namespace
{
    void errorCallback(int code, const char* message)
    {
        std::cout << "GLFW error " << code << ": " << message << std::endl;
    }
}

App::App() = default;

App::~App()
{
    // 包括初始化失败的路径：先释放 GPU 资源，再关闭窗口和 GLFW。
    if (window)
    {
        glfwMakeContextCurrent(window);
        ui.reset();
        cube.reset();
        ourShader.reset();
        glfwDestroyWindow(window);
    }
    if (glfwInitialized)
    {
        glfwTerminate();
    }
}

int App::run(bool smokeTest)
{
    if (!initialize(smokeTest))
    {
        return -1;
    }
    return renderLoop(smokeTest);
}

bool App::initialize(bool smokeTest)
{
    // --------初始化 GLFW 和窗口--------
    glfwSetErrorCallback(errorCallback);

    if (!glfwInit())
    {
        std::cout << "GLFW init failed\n";
        return false;
    }

    glfwInitialized = true;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if (smokeTest)
    {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }

    window = glfwCreateWindow(
        smokeTest ? 900 : 1200, 600,
        "Lightframe | Focal length experiment",
        nullptr, nullptr
    );

    if (!window)
    {
        std::cout << "Window creation failed\n";
        return false;
    }

    glfwMakeContextCurrent(window);

    glfwSetWindowUserPointer(window, this);
    glfwSetWindowFocusCallback(window, [](GLFWwindow* source, int focused)
    {
        if (!focused)
        {
            static_cast<App*>(glfwGetWindowUserPointer(source))->cameraController.stopMouseLook(source);
        }
    });

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "GLAD init failed\n";
        return false;
    }

    glfwSwapInterval(smokeTest ? 0 : 1);

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GPU: " << glGetString(GL_RENDERER) << std::endl;

    // --------从外部文件读入着色器--------
    ourShader = std::make_unique<Shader>("shaders/vertex.txt", "shaders/fragment.txt");

    if (ourShader->ID == 0)
    {
        return false;
    }

    // --------创建立方体顶点资源--------
    cube = std::make_unique<Mesh>();

    if (!smokeTest)
    {
        ui = std::make_unique<UI>();
        if (!ui->initialize(window)) { return false; }
    }

    glEnable(GL_DEPTH_TEST);
    return true;
}

int App::renderLoop(bool smokeTest)
{
    int modelLocation = glGetUniformLocation(ourShader->ID, "model");
    int viewLocation = glGetUniformLocation(ourShader->ID, "view");
    int projectionLocation = glGetUniformLocation(ourShader->ID, "projection");
    int faceColorLocation = glGetUniformLocation(ourShader->ID, "faceColor");

    // --------渲染循环--------
    int result = 0;
    double lastFrameTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        double now = glfwGetTime();
        // 长时间停顿后不补走整段距离；每帧（含最小化期间）刷新时钟。
        float deltaSeconds = static_cast<float>(std::min(now - lastFrameTime, 0.1));
        lastFrameTime = now;

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        // 最小化时暂停绘制，恢复窗口后继续。
        if (width == 0 || height == 0)
        {
            cameraController.stopMouseLook(window);
            glfwWaitEventsTimeout(0.1);
            continue;
        }

        if (ui)
        {
            ui->beginFrame(cameraController.isMouseLookActive());
            const UIRequests requests = ui->draw(settings, camera);
            if (requests.resetPose || requests.resetExperiment)
            {
                cameraController.resetPose(window, camera);
            }
            if (requests.resetFocalLength)
            {
                settings.setFocalLengthMm(CameraSettings::defaultFocalLengthMm);
            }
            if (requests.resetExperiment) { settings.setFocalLengthMm(24.0f); }
            processInput(deltaSeconds);
        }

        glClearColor(0.08f, 0.10f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 自检沿用完整窗口；普通运行使用 UI 计算的真实照片矩形。
        int viewportWidth = width;
        int viewportHeight = static_cast<int>(width / 1.5f);
        if (viewportHeight > height)
        {
            viewportHeight = height;
            viewportWidth = static_cast<int>(height * 1.5f);
        }
        glViewport((width - viewportWidth) / 2, (height - viewportHeight) / 2,
            viewportWidth, viewportHeight);

        if (ui)
        {
            const ViewportRect& viewport = ui->getViewportRect();
            int windowWidth, windowHeight;
            glfwGetWindowSize(window, &windowWidth, &windowHeight);
            const float scaleX = static_cast<float>(width) / windowWidth;
            const float scaleY = static_cast<float>(height) / windowHeight;
            const int left = static_cast<int>(std::round(viewport.x * scaleX));
            const int right = static_cast<int>(std::round((viewport.x + viewport.width) * scaleX));
            const int top = static_cast<int>(std::round(viewport.y * scaleY));
            const int bottom = static_cast<int>(std::round((viewport.y + viewport.height) * scaleY));
            glViewport(left, height - bottom, right - left, bottom - top);
        }

        glm::mat4 projection = camera.projectionMatrix(settings);
        glm::mat4 view = camera.viewMatrix();

        ourShader->use();
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
        for (const auto& object : scene.getObjects())
        {
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(object.model));
            cube->draw(faceColorLocation, object.tint);
        }

        // 自动验证：前面保持橙色，且深度已写入；后画的背面不能覆盖它。
        if (smokeTest)
        {
            unsigned char pixel[3] = {};
            float depth = 1.0f;
            glReadPixels(width / 2, height / 2, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
            glReadPixels(width / 2, height / 2, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

            if (glGetError() != GL_NO_ERROR || pixel[0] < 200 || pixel[1] < 100 || pixel[2] > 100
                || !std::isfinite(depth) || depth <= 0.0f || depth >= 1.0f)
            {
                std::cout << "Smoke test failed: cube color/depth readback.\n";
                result = -1;
            }
            else
            {
                std::cout << "Smoke test passed: scene, camera matrices, depth test, pixel readback.\n";
            }

            break;
        }

        if (ui) { ui->render(); }
        glfwSwapBuffers(window);
    }

    return result;
}

void App::processInput(float deltaSeconds)
{
    if (!glfwGetWindowAttrib(window, GLFW_FOCUSED)
        || glfwGetWindowAttrib(window, GLFW_ICONIFIED))
    {
        cameraController.stopMouseLook(window);
        return;
    }

    const bool inputCaptured = ui->capturesKeyboard() || ui->capturesMouse();
    if (!ui->capturesKeyboard() && glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if (cameraController.isMouseLookActive()
        && (inputCaptured || glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS))
    {
        cameraController.stopMouseLook(window);
    }
    if (inputCaptured) { return; }

    double cursorX, cursorY;
    glfwGetCursorPos(window, &cursorX, &cursorY);
    const bool overViewport = ui->getViewportRect().contains(cursorX, cursorY);
    if (ui->resetPosePressed()) { cameraController.resetPose(window, camera); }
    if (overViewport && ui->rightMousePressed()
        && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        cameraController.startMouseLook(window);
    }
    if (overViewport && !cameraController.isMouseLookActive())
    {
        const float focalLength = std::clamp(settings.focalLengthMm + ui->mouseWheel() * 2.0f,
            CameraSettings::minFocalLengthMm, CameraSettings::maxFocalLengthMm);
        settings.setFocalLengthMm(focalLength);
    }
    if (!overViewport && !cameraController.isMouseLookActive()) { return; }
    cameraController.update(window, camera, deltaSeconds);
}
