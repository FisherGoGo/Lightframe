# Photograph 工程初始化

使用现有的 Visual Studio 2026 原生 C++ 工程，工具集 v145、Windows SDK 10.0。
公共设置在 `Dependencies.props`：C++17、UTF-8、/W4、并行编译。
Debug 使用 /MDd，Release 使用 /MD；GLFW 使用 DLL，避免第三方静态库的运行库冲突。

## 启动

1. 用 Visual Studio 打开 `Photograph.slnx`。
2. 选择 **Debug / x64**，将 Photograph 设为启动项目。
3. 按 F5。看到深色背景和橙色立方体即可验证绘制；固定机位能看到三个面，Esc 退出。

当前机器已经准备好依赖。在新机器上，从本目录运行：

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\setup-deps.ps1
```

脚本从官方仓库获取固定版本并验证 SHA256，不修改系统环境变量或全局 VS 设置。
首次准备依赖需要联网；依赖准备后构建和运行均不需要下载。

## 依赖与输出

| 依赖 | 版本 | 用途 |
|---|---|---|
| GLFW | 3.4 | 窗口、输入、OpenGL 上下文；官方 lib-vc2022 DLL 包 |
| GLAD | 0.1.36 | OpenGL 3.3 Core 加载器，已生成并放在 third_party/glad |
| GLM | 1.0.1 | 数学运算，头文件库 |

GLAD 使用该版本自带的规范生成：`--profile=core --api=gl=3.3 --generator=c --extensions= --reproducible`。
无需在使用工程时安装 Python。许可证保留在各依赖目录中。
GLFW 和 GLM 存放在 `.deps`，目录不进入版本控制；GLAD 源码和许可证应进入版本控制。

- 程序与 GLFW DLL：`bin/<平台>/<配置>/`。
- 中间文件：`obj/<平台>/<配置>/`。
- Debug/Release 的 x64 和 Win32 均配置了对应架构依赖，优先使用 x64。
- 调试工作目录为可执行文件目录。

基础入口为 `src/main.cpp`，目前实现窗口、固定机位立方体、model/view/projection 矩阵、深度测试、缩放与最小化处理、错误日志和退出清理。各面颜色深浅为手动设置，尚未加入光照；移动相机留待下一步。
投影暂用 45° 垂直视场角、3:2 比例、0.1～100 的裁剪范围。窗口比例变化时留边，保持物体比例。
着色器放在 `shaders/vertex.txt` 和 `shaders/fragment.txt`，由参考课程项目整理的 `src/Shader.h`、`src/Shader.cpp` 读取和编译。
入口使用 `Shader ourShader("shaders/vertex.txt", "shaders/fragment.txt")` 创建程序，使用 `ourShader.use()` 启用。
文件路径相对于 exe 所在目录；构建会自动将着色器复制到输出目录的 `shaders/`，只修改着色器也会触发复制。
修改源码目录里的着色器后重新构建再运行；加载或编译失败会输出文件路径和日志并退出。
打包时一起带上 exe、GLFW DLL 和 `shaders/`。相机、场景和后处理按上级目录的开发文档逐步加入。

代码风格参考 `Computer Graphics` 课程作业：初始化、着色器加载、顶点数据和渲染循环顺序写在 `main()` 中；大括号换行，使用 `VAO/VBO`、`ourShader` 等课程命名，用中文注释分段。输入放在简单辅助函数中，文件读取和编译检查由 `Shader` 类处理。

## 命令行验证

在 VS 的 Developer PowerShell 中运行：

```powershell
msbuild .\Photograph.vcxproj /m /p:Configuration=Debug /p:Platform=x64
msbuild .\Photograph.vcxproj /m /p:Configuration=Release /p:Platform=x64
& .\bin\x64\Debug\Photograph.exe --smoke-test
```

`--smoke-test` 使用隐藏窗口编译着色器、绘制立方体并读回中心颜色和深度，检查橙色前面未被后画的面覆盖、深度已写入，成功时自动退出。
演示设备的显卡与驱动兼容性仍需在实际设备验证。

初始化验证：本机 x64/Win32 的 Debug/Release 均构建通过；x64 Debug 与 Release 的隐藏窗口测试通过。
外置着色器验证：四种构建配置均从工程外的工作目录启动并通过绘制测试；文件缺失和 GLSL 编译错误均能正确报错退出；单独更新着色器会触发资源复制。
验证显卡为 NVIDIA GeForce RTX 4070 Laptop GPU，OpenGL 上下文为 3.3，驱动报告版本 610.74。

固定机位立方体验证：x64 Debug/Release 构建通过，隐藏窗口颜色与深度读回测试通过。
