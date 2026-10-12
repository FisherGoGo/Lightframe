#include "Shader.h"

#include <Windows.h>
#include <glad/glad.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    // 相对路径从 exe 所在目录开始，避免受启动工作目录影响。
    wchar_t executablePath[32768];
    DWORD length = GetModuleFileNameW(nullptr, executablePath, 32768);

    if (length == 0 || length == 32768)
    {
        std::cout << "Cannot find executable directory.\n";
        return;
    }

    std::filesystem::path resourceRoot = std::filesystem::path(executablePath).parent_path();
    std::filesystem::path vertexFilePath = resourceRoot / vertexPath;
    std::filesystem::path fragmentFilePath = resourceRoot / fragmentPath;

    // --------读入顶点和片段着色器文件--------
    std::ifstream vertexFile(vertexFilePath);
    std::ifstream fragmentFile(fragmentFilePath);

    if (!vertexFile.is_open() || !fragmentFile.is_open())
    {
        if (!vertexFile.is_open())
        {
            std::cout << "Cannot open shader: " << vertexFilePath.u8string() << std::endl;
        }
        if (!fragmentFile.is_open())
        {
            std::cout << "Cannot open shader: " << fragmentFilePath.u8string() << std::endl;
        }
        return;
    }

    std::stringstream vertexSStream;
    std::stringstream fragmentSStream;
    vertexSStream << vertexFile.rdbuf();
    fragmentSStream << fragmentFile.rdbuf();

    if (vertexFile.bad() || fragmentFile.bad())
    {
        std::cout << "Shader read failed: " << vertexFilePath.u8string()
            << " or " << fragmentFilePath.u8string() << std::endl;
        return;
    }

    // 字符串在编译期间保持有效，c_str() 提供 OpenGL 所需的源码指针。
    std::string vertexString = vertexSStream.str();
    std::string fragmentString = fragmentSStream.str();
    const char* vertexSource = vertexString.c_str();
    const char* fragmentSource = fragmentString.c_str();

    // --------编译着色器--------
    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexSource, nullptr);
    glCompileShader(vertex);

    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentSource, nullptr);
    glCompileShader(fragment);

    bool vertexSuccess = checkCompileErrors(vertex, vertexFilePath.u8string());
    bool fragmentSuccess = checkCompileErrors(fragment, fragmentFilePath.u8string());

    if (!vertexSuccess || !fragmentSuccess)
    {
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return;
    }

    // --------链接着色器程序--------
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    if (!checkCompileErrors(ID, "PROGRAM"))
    {
        std::cout << "Shader files: " << vertexFilePath.u8string()
            << " and " << fragmentFilePath.u8string() << std::endl;
        glDeleteProgram(ID);
        ID = 0;
    }
}

void Shader::use()
{
    glUseProgram(ID);
}

// 保留完整日志，方便定位外部文件中的 GLSL 错误。
bool Shader::checkCompileErrors(unsigned int shaderID, const std::string& type)
{
    int success;
    int length;

    if (type != "PROGRAM")
    {
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &length);
    }
    else
    {
        glGetProgramiv(shaderID, GL_LINK_STATUS, &success);
        glGetProgramiv(shaderID, GL_INFO_LOG_LENGTH, &length);
    }

    if (!success)
    {
        std::vector<char> infoLog(static_cast<std::size_t>(length) + 1);

        if (type != "PROGRAM")
        {
            glGetShaderInfoLog(shaderID, length, nullptr, infoLog.data());
        }
        else
        {
            glGetProgramInfoLog(shaderID, length, nullptr, infoLog.data());
        }

        std::cout << type << " compile/link error: " << infoLog.data() << std::endl;
        return false;
    }

    return true;
}
