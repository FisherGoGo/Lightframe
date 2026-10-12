#include "Mesh.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace
{
    // 每个面由两个三角形组成，共 36 个顶点；边长为 1，中心在原点。
    const float vertices[] = {
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

}

Mesh::Mesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 每个顶点有 3 个 float，表示位置。
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

Mesh::~Mesh()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void Mesh::draw(int faceColorLocation, glm::vec3 tint) const
{
    glBindVertexArray(VAO);
    for (int face = 0; face < 6; ++face)
    {
        glm::vec3 color = faceColors[face] * tint;
        glUniform3fv(faceColorLocation, 1, glm::value_ptr(color));
        glDrawArrays(GL_TRIANGLES, face * 6, 6);
    }
}
