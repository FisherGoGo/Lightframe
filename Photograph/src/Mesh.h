#pragma once

#include <glm/glm.hpp>

// 当前只提供基线立方体；VAO/VBO 在有效 OpenGL 上下文内创建和释放。
class Mesh
{
public:
    Mesh();
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&&) = delete;
    Mesh& operator=(Mesh&&) = delete;

    // 调用方先启用 Shader 并设置矩阵；保留各面颜色及原来的绘制顺序。
    void draw(int faceColorLocation, glm::vec3 tint = glm::vec3(1.0f)) const;

private:
    unsigned int VAO = 0;
    unsigned int VBO = 0;
};
