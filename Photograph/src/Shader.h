#pragma once

#include <string>

// 参考课程项目的 Shader 类：读取文件、编译、链接，再用 use() 启用。
class Shader
{
public:
    Shader(const char* vertexPath, const char* fragmentPath);

    unsigned int ID = 0; // Shader Program ID；加载失败时保持为 0。
    void use();

private:
    bool checkCompileErrors(unsigned int shaderID, const std::string& type);
};
