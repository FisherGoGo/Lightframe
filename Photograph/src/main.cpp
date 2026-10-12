#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <iostream>
#include <string>
#include "Shader.h"

void processInput(GLFWwindow* window);
void errorCallback(int code, const char* message);

/*
创建 OpenGL 3.3 窗口，从固定机位观察一个立方体。
按 Esc 退出；--smoke-test 用于自动验证绘制结果。
*/
int main(int argc, char** argv)
{
    bool smokeTest = argc > 1 && std::string(argv[1]) == "--smoke-test";

    // --------初始化 GLFW 和窗口--------
    glfwSetErrorCallback(errorCallback);

    if (!glfwInit())
    {
        std::cout << "GLFW init failed\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if (smokeTest)
    {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }

    GLFWwindow* window = glfwCreateWindow(
        900, 600,
        "Photograph | OpenGL 3.3 | Esc to exit",
        nullptr, nullptr
    );

    if (!window)
    {
        std::cout << "Window creation failed\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "GLAD init failed\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glfwSwapInterval(smokeTest ? 0 : 1);

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GPU: " << glGetString(GL_RENDERER) << std::endl;

    // --------从外部文件读入着色器--------
    Shader ourShader("shaders/vertex.txt", "shaders/fragment.txt");

    if (ourShader.ID == 0)
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // --------初始化顶点数据、VBO 和 VAO--------
    // 每个面由两个三角形组成，共 36 个顶点；边长为 1，中心在原点。
    float vertices[] = {
        // 前面（+Z），先绘制，后面的面不能覆盖它。
        -0.5f, -0.5f,  0.5f,   0.5f, -0.5f,  0.5f,   0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,  -0.5f,  0.5f,  0.5f,  -0.5f, -0.5f,  0.5f,
        // 后面（-Z）。
        -0.5f, -0.5f, -0.5f,   0.5f, -0.5f, -0.5f,   0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,  -0.5f,  0.5f, -0.5f,  -0.5f, -0.5f, -0.5f,
        // 左面（-X）。
        -0.5f, -0.5f, -0.5f,  -0.5f, -0.5f,  0.5f,  -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,  -0.5f,  0.5f, -0.5f,  -0.5f, -0.5f, -0.5f,
        // 右面（+X）。
         0.5f, -0.5f, -0.5f,   0.5f, -0.5f,  0.5f,   0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,   0.5f,  0.5f, -0.5f,   0.5f, -0.5f, -0.5f,
        // 底面（-Y）。
        -0.5f, -0.5f, -0.5f,   0.5f, -0.5f, -0.5f,   0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,  -0.5f, -0.5f,  0.5f,  -0.5f, -0.5f, -0.5f,
        // 顶面（+Y）。
        -0.5f,  0.5f, -0.5f,   0.5f,  0.5f, -0.5f,   0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,  -0.5f,  0.5f,  0.5f,  -0.5f,  0.5f, -0.5f
    };

    // 用不同深浅区分各面，方便观察形状；此时尚未加入光照。
    const glm::vec3 faceColors[] = {
        {0.95f, 0.55f, 0.20f}, {0.45f, 0.22f, 0.08f},
        {0.65f, 0.32f, 0.12f}, {0.75f, 0.38f, 0.14f},
        {0.50f, 0.25f, 0.10f}, {1.00f, 0.70f, 0.35f}
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 每个顶点有 3 个 float，表示位置。
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glEnable(GL_DEPTH_TEST);

    // --------固定机位与模型矩阵--------
    glm::mat4 model = glm::rotate(
        glm::mat4(1.0f), glm::radians(-25.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 1.2f, 3.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    int modelLocation = glGetUniformLocation(ourShader.ID, "model");
    int viewLocation = glGetUniformLocation(ourShader.ID, "view");
    int projectionLocation = glGetUniformLocation(ourShader.ID, "projection");
    int faceColorLocation = glGetUniformLocation(ourShader.ID, "faceColor");

    // --------渲染循环--------
    int result = 0;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        processInput(window);

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

        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f), 1.5f, 0.1f, 100.0f);

        ourShader.use();
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
        glBindVertexArray(VAO);
        for (int face = 0; face < 6; ++face)
        {
            glUniform3fv(faceColorLocation, 1, glm::value_ptr(faceColors[face]));
            glDrawArrays(GL_TRIANGLES, face * 6, 6);
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
                std::cout << "Smoke test passed: cube, matrices, depth test, pixel readback.\n";
            }

            break;
        }

        glfwSwapBuffers(window);
    }

    // --------释放资源，再关闭窗口--------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(ourShader.ID);

    glfwDestroyWindow(window);
    glfwTerminate();
    return result;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void errorCallback(int code, const char* message)
{
    std::cout << "GLFW error " << code << ": " << message << std::endl;
}
