#pragma once

#include <string>

// 参考课程项目的 Shader 类：读取文件、编译、链接，再用 use() 启用。
class Shader
{
public:
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&&) = delete;
    Shader& operator=(Shader&&) = delete;

    unsigned int ID = 0; // Shader 拥有程序并在析构时释放；加载失败时保持为 0。
    void use();

private:
    bool checkCompileErrors(unsigned int shaderID, const std::string& type);
};
