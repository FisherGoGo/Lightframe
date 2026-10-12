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
        900, 600,
        "Lightframe | WASD move | Q/E height | RMB look | R reset | Esc exit",
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
            static_cast<App*>(glfwGetWindowUserPointer(source))->stopMouseLook();
        }
    });
    glfwSetMouseButtonCallback(window, [](GLFWwindow* source, int button, int action, int)
    {
        auto* app = static_cast<App*>(glfwGetWindowUserPointer(source));
        if (button != GLFW_MOUSE_BUTTON_RIGHT) { return; }
        if (action == GLFW_PRESS && glfwGetWindowAttrib(source, GLFW_FOCUSED))
        {
            app->mouseLookActive = true;
            glfwSetInputMode(source, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            // 与作业的 Attach 一样，捕获后立即读取位置作为基准。
            glfwGetCursorPos(source, &app->lastCursorX, &app->lastCursorY);
        }
        else if (action == GLFW_RELEASE)
        {
            app->stopMouseLook();
        }
    });
    glfwSetKeyCallback(window, [](GLFWwindow* source, int key, int, int action, int)
    {
        if (key == GLFW_KEY_R && action == GLFW_PRESS)
        {
            auto* app = static_cast<App*>(glfwGetWindowUserPointer(source));
            app->camera.resetPose();
            app->stopMouseLook();
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
        processInput(deltaSeconds);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        // 最小化时暂停绘制，恢复窗口后继续。
        if (width == 0 || height == 0)
        {
            glfwWaitEventsTimeout(0.1);
            continue;
        }

        glClearColor(0.08f, 0.10f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 保持 3:2 取景比例，窗口比例不同时留边，立方体不会被拉伸。
        int viewportWidth = width;
        int viewportHeight = static_cast<int>(width / 1.5f);
        if (viewportHeight > height)
        {
            viewportHeight = height;
            viewportWidth = static_cast<int>(height * 1.5f);
        }
        glViewport((width - viewportWidth) / 2, (height - viewportHeight) / 2,
            viewportWidth, viewportHeight);

        glm::mat4 projection = camera.projectionMatrix();
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

        glfwSwapBuffers(window);
    }

    return result;
}

void App::processInput(float deltaSeconds)
{
    if (!glfwGetWindowAttrib(window, GLFW_FOCUSED)
        || glfwGetWindowAttrib(window, GLFW_ICONIFIED))
    {
        return;
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    // 与作业的 Update 一样，先转头，再沿本帧更新后的朝向移动。
    processMouseMovement();

    glm::vec3 direction(0.0f);
    direction.z = static_cast<float>((glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        - (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS));
    direction.x = static_cast<float>((glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        - (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS));
    direction.y = static_cast<float>((glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        - (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS));
    camera.move(direction, deltaSeconds);
}

void App::stopMouseLook()
{
    mouseLookActive = false;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void App::processMouseMovement()
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
