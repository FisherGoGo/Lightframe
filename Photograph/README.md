# Photograph 工程初始化

使用现有的 Visual Studio 2026 原生 C++ 工程，工具集 v145、Windows SDK 10.0。
公共设置在 `Dependencies.props`：C++17、UTF-8、/W4、并行编译。
Debug 使用 /MDd，Release 使用 /MD；GLFW 使用 DLL，避免第三方静态库的运行库冲突。

## 启动

1. 用 Visual Studio 打开 `Photograph.slnx`。
2. 选择 **Debug / x64**，将 Photograph 设为启动项目。
3. 按 F5。打开 1200×600 窗口，左侧为焦距面板，右侧为 3:2 取景器；默认焦距 50 mm。用面板切换焦距，在取景器内移动、观察或滚轮变焦。

| 操作 | 行为 |
|---|---|
| W / S | 沿观察方向前进 / 后退 |
| A / D | 向左 / 向右平移 |
| Q / E | 沿世界竖直方向下降 / 上升 |
| 在取景器内按住右键并移动鼠标 | 转向；松开恢复普通光标 |
| 在取景器内滚轮 | 每单位滚轮改变 2 mm，限制在 24～85 mm；观察时不变焦 |
| 焦距滑块 / 24、35、50、85 mm 按钮 | 连续变焦 / 切换预设，不改变机位；Ctrl+单击滑块可输入精确值 |
| R / Reset pose | 只恢复初始位置和方向，退出右键观察，保留焦距 |
| Reset focal length | 只恢复 50 mm，保留机位 |
| Reset experiment A | 同时恢复初始机位和 24 mm |
| Esc | UI 未占用键盘时退出；编辑数值时先取消编辑 |

鼠标在取景器内或已进入右键观察时才更新机位；面板、留边和 UI 编辑期间不移动、不启动观察，也不滚轮变焦。单击取景器空白可退出面板的键盘焦点。界面当前使用 ImGui 默认字体和英文标签，无需额外字体资源。

按 2026-10-13 的最新安排，当前面板保留作开发与调试 UI，P04～P09 在此接入相应标准控件；参考图风格的摄影 HUD 默认在 P09 验收后的 P10 界面整理阶段制作，原面板届时作为可开关调试入口保留。若确定只交最低版本，可在 P05 验收后进入 P10；详见 [HUD 排期与方案](../文档/10-摄影HUD风格与实现方案.md#8-启动阶段实施顺序与验收)。当前程序外观和操作不变。

移动速度为 2.5 m/s，组合方向归一化；允许自由飞行和穿过物体。窗口失焦或最小化时停止移动，失焦同时退出观察；返回后需要重新按下移动键或右键。单帧移动时间差最多为 0.1 s，避免长时间停顿后突然跳远。

鼠标转向沿用 10.12 作业的逐帧读取形式：右键捕获后立即记录鼠标坐标，每帧 `glfwGetCursorPos()` 计算相对上帧的差值，先转向再移动。角度增益为 0.05°/坐标单位，不乘时间差；pitch 限制为 ±89°。

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
| Dear ImGui | 1.91.9b | 焦距面板；官方 GLFW / OpenGL3 后端，源码直接编入 exe |

GLAD 使用该版本自带的规范生成：`--profile=core --api=gl=3.3 --generator=c --extensions= --reproducible`。
无需在使用工程时安装 Python。许可证保留在各依赖目录中。
GLFW、GLM 和 Dear ImGui 存放在 `.deps`，目录不进入版本控制；GLAD 源码和许可证应进入版本控制。ImGui 使用固定标签 `v1.91.9b`，依赖脚本验证下载包 SHA256；集成参考[官方 GLFW/OpenGL3 示例](https://github.com/ocornut/imgui/blob/v1.91.9b/examples/example_glfw_opengl3/main.cpp)和[输入分配说明](https://github.com/ocornut/imgui/blob/v1.91.9b/docs/FAQ.md#q-how-can-i-tell-whether-to-dispatch-mousekeyboard-to-dear-imgui-or-my-application)。

- 程序与 GLFW DLL：`bin/<平台>/<配置>/`。
- 中间文件：`obj/<平台>/<配置>/`。
- Debug/Release 的 x64 和 Win32 均配置了对应架构依赖，优先使用 x64。
- 调试工作目录为可执行文件目录。

入口 `src/main.cpp` 只解析 `--smoke-test`，创建 `App` 并返回运行结果。文件、类、函数职责及完整调用流程见 [09-代码结构与职责](../文档/09-代码结构与职责.md)；代码结构变化时在同一次修改中同步更新该文档。当前骨架职责如下：

| 文件 | 已实现职责 |
|---|---|
| `src/App.h/.cpp` | GLFW/GLAD 初始化、窗口、输入分配、复位请求、Esc、时间差、取景矩形换算、遍历实例绘制和自检 |
| `src/CameraController.h/.cpp` | WASD/QE 输入、鼠标捕获/释放、观察状态与坐标基准、逐帧转向和移动、复位 |
| `src/Camera.h/.cpp` | 位置、yaw/pitch、方向基向量、观察/焦距投影矩阵、移动、转向和复位 |
| `src/CameraSettings.h` | 唯一焦距设置、24～85 mm 校验、固定 36×24 mm 传感器、FOV 与比例计算 |
| `src/UI.h/.cpp` | ImGui 生命周期、焦距控件、机位/FOV 读数、复位请求、布局与输入捕获状态 |
| `src/ViewportRect.h` | 逻辑坐标下的真实取景矩形与鼠标命中判断 |
| `src/Scene.h/.cpp` | 地面、前景主体和三个远景参照物的模型矩阵与颜色乘子 |
| `src/Mesh.h/.cpp` | 单位立方体的 36 个顶点、逐面颜色与绘制、VAO/VBO 的创建和析构 |
| `src/Shader.h/.cpp` | 外部着色器加载、编译/链接日志、启用程序和析构释放 |

App 拥有 Camera、CameraController、CameraSettings、Scene、UI、Mesh 和 Shader，在 OpenGL 上下文有效时先销毁 UI 后端及其 GPU 资源，再销毁 Mesh/Shader，最后销毁窗口并终止 GLFW；初始化失败也通过同一析构路径清理。App、UI、Mesh、Shader 禁止复制和移动，避免重复释放；每个 App 实例运行一次。CameraController 不持有窗口或 Camera，调用时显式接收；Camera、CameraController、CameraSettings 和 Scene 不持有 GPU 资源，五个实例共享一个 Mesh。各面颜色深浅仍为手动设置，尚未加入光照。
投影读取 CameraSettings：`fovY = 2*atan(24/(2*focalLengthMm))`，比例 36/24，近远裁剪面 0.1～100 m。窗口缩放只改变面板旁的取景显示矩形，保持完整 3:2 构图；当前直接绘制到窗口，固定分辨率离屏目标留到 P04。
着色器放在 `shaders/vertex.txt` 和 `shaders/fragment.txt`，由参考课程项目整理的 `src/Shader.h`、`src/Shader.cpp` 读取和编译。
App 使用 `std::make_unique<Shader>("shaders/vertex.txt", "shaders/fragment.txt")` 创建程序，使用 `ourShader->use()` 启用；Mesh 的 `draw(faceColorLocation, tint)` 将原逐面颜色乘以实例颜色，默认乘子为白色，保留原绘制顺序。
文件路径相对于 exe 所在目录；构建会自动将着色器复制到输出目录的 `shaders/`，只修改着色器也会触发复制。
修改源码目录里的着色器后重新构建再运行；加载或编译失败会输出文件路径和日志并退出。
打包时一起带上 exe、GLFW DLL、`shaders/` 及依赖许可文件。ImGui 已编入 exe，无单独 DLL；曝光、景深、拍照等按上级目录的开发文档逐步加入。

代码风格参考 `Computer Graphics` 课程作业：大括号换行，保留 `VAO/VBO`、`ourShader` 等课程命名，用中文注释分段。App 每帧先建立 UI、编辑同一份设置、执行复位请求，再根据 UI 捕获与取景命中分配输入、绘制场景，最后合成 UI。ImGui 官方后端接收全部键鼠事件，并串接 App 的失焦释放回调；App 按下沿处理右键进入/R 复位，连续移动与鼠标差值仍由 CameraController 处理。观察时 UI 忽略虚拟鼠标坐标，松开后恢复；新增 `.cpp/.h` 已登记到工程和筛选器。

## 默认构建与运行检查

2026-10-13 起，代码修改后默认只构建当前使用的配置并启动程序，确认短暂运行及正常关闭没有报错或异常退出；画面和功能效果由用户手动检查。未指定配置时使用 x64 Debug。

从本目录，在 VS 的 Developer PowerShell 中运行：

```powershell
msbuild .\Photograph.vcxproj /m /p:Configuration=Debug /p:Platform=x64
& .\bin\x64\Debug\Photograph.exe
```

默认不执行四种配置、`--smoke-test`、CPU/数值测试、窗口自动操作或截图比较。用户明确要求专项验证时再运行现有入口和脚本；纯文档修改不构建或启动程序。详细约定见 [agent.md 第 6 节](../agent.md#6-默认检查与手动验收)。

## 可选专项检查与历史记录

`--smoke-test` 为保留的可选检查入口，使用 900×600 隐藏窗口，不初始化 UI，采用默认 50 mm 投影绘制完整 Scene 并读回中心颜色和深度，检查橙色前景主体未被后画的面及远景物体覆盖、深度已写入，成功时自动退出。本轮未执行该入口。
演示设备的显卡与驱动兼容性仍需在实际设备验证。

以下三条为骨架拆分前的历史验证：

初始化验证：本机 x64/Win32 的 Debug/Release 均构建通过；x64 Debug 与 Release 的隐藏窗口测试通过。
外置着色器验证：四种构建配置均从工程外的工作目录启动并通过绘制测试；文件缺失和 GLSL 编译错误均能正确报错退出；单独更新着色器会触发资源复制。
验证显卡为 NVIDIA GeForce RTX 4070 Laptop GPU，OpenGL 上下文为 3.3，驱动报告版本 610.74。

固定机位立方体验证：x64 Debug/Release 构建通过，隐藏窗口颜色与深度读回测试通过。

2026-10-12 P00/P01 本轮验收：拆分前后 x64/Win32 × Debug/Release 均完成 Rebuild 和绘制自检，从仓库根目录启动仍能加载资源；Mesh 单独拆出时也完成 x64 Debug 构建和自检。真实窗口通过自动操作验证正常尺寸、宽窗、竖窗、恢复尺寸、最小化恢复和 Esc 退出；五种状态的拆分前后客户区 PNG SHA256 完全一致。Shader 文件缺失、GLSL 编译失败和程序链接失败均输出相应诊断，返回 -1。

验收说明见 [P00/P01 验收记录](../文档/验收/P00-P01/README.md)，最新任务状态见 [08 执行记录](../文档/08-小项目拆分与模块化推进.md#7-执行记录)。验收日志、截图、验证脚本和独立测试代码仅保留在本地，不随仓库上传；文档中的脚本命令作为历史验收记录，需使用本地脚本执行。演示设备及其他显卡仍待验证。

2026-10-12 P02 验收：四种配置 Rebuild 与绘制自检通过，纯 CPU 检查通过帧率无关移动、斜向限速、方向、俯仰限幅、复位与场景落地关系。真实窗口自动验证移动、右键进入/拖动/释放、复位、失焦恢复、缩放、最小化恢复和 Esc；记录与复验命令见 [P02 验收](../文档/验收/P02/README.md)。当时无摄影参数 UI，焦距实验由 P03 接入。

2026-10-13 CameraController 拆分复验：相机键鼠操作已从 App 抽出，原键位和逐帧转向形式保持一致。四种配置 Rebuild（0 警告、0 错误）与绘制自检、10 项 CPU 检查及完整窗口自动操作均通过；五张基准/恢复截图与上一版内部 RGB 像素一致。职责和复验命令见 [P02 控制器拆分验收](../文档/验收/P02/README.md#2026-10-13调整cameracontroller-拆分)。对焦与拍照仍交给后续对应模块。

2026-10-13 P03：CameraSettings、焦距投影、ImGui 面板、取景矩形和 UI/相机输入分流实现完成。x64 Debug 构建通过（0 警告、0 错误），普通程序短暂运行并正常关闭返回 0，无错误日志。效果待用户手动验收；步骤见 [P03 检查与手动验收](../文档/验收/P03/README.md)。本轮未执行其他配置、自检、CPU 测试、自动交互或截图比较。
