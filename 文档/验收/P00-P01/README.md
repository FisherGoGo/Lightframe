# P00 / P01 本轮验收

验收日志、截图、验证脚本和独立测试代码仅保留在本地，不纳入 GitHub 仓库。本文保留历史验收结果、本地文件名及本轮命令；文档中的脚本命令需使用本地保留的脚本执行。

2026-10-13 起，本文完整复验命令仅在用户明确要求专项验证时运行，不属于每次修改后的默认检查。默认仅构建当前配置并启动程序确认不报错，运行效果由用户手动检查，详见 [agent.md 第 6 节](../../../agent.md#6-默认检查与手动验收)。下列已执行结果保留为历史记录。

日期：2026-10-12。P00 基线是提交 `971cc36` 的应用源码，P01 是本轮 App/Mesh 拆分后的源码。本文记录实际执行结果；复验脚本默认写入 `recheck-*`，避免覆盖本轮证据。

## 环境与结果

- 环境：Windows NT 10.0.22631.0；Visual Studio 2026 Community、v145（MSVC 14.51.36231）、MSBuild 18.10.1.42706、Windows SDK 10.0、C++17；依赖版本见 [工程 README](../../../Photograph/README.md)。
- GPU：NVIDIA GeForce RTX 4070 Laptop GPU，OpenGL 3.3.0，驱动报告 NVIDIA 610.74。
- P00/P01：x64 和 Win32 的 Debug/Release 均执行 Rebuild，均为 0 个警告、0 个错误；每种配置的 `--smoke-test` 均通过。
- Mesh 单独抽出时，另执行一次 x64 Debug Build 和自检，通过，见 `p01-mesh-*`。
- 所有程序从仓库根目录启动，证明 Shader 读取依赖 exe 所在目录。
- 窗口验收用 Win32 自动操作和仅客户区截图完成；宽窗、竖窗、恢复尺寸与最小化恢复均继续显示立方体，Esc 键盘消息由原输入检查处理，返回 0。未做人工操作验收。
- P01 Shader 文件缺失、GLSL 编译失败、GLSL 接口不匹配导致的链接失败均保留诊断并返回 -1。错误注入只修改 `Photograph/bin/p01-error-tests/` 下的测试副本。
- 资源生命周期通过代码核查：App 析构令自身上下文成为当前，依次析构 Mesh（VAO/VBO）、Shader（program），然后销毁窗口、终止 GLFW。未执行 GPU 泄漏分析或 GLFW/GLAD 初始化失败注入。

## 画面证据

| 窗口状态 | 客户区 | 立方体包围尺寸 | P00/P01 对比 |
|---|---|---|---|
| 正常 | 900×600 | 318×325 | PNG SHA256 相同 |
| 宽窗 | 1200×600 | 318×325 | PNG SHA256 相同 |
| 竖窗 | 600×900 | 212×217 | PNG SHA256 相同 |
| 恢复尺寸 | 900×600 | 318×325 | PNG SHA256 相同 |
| 最小化后恢复 | 900×600 | 中心橙色，画面恢复 | PNG SHA256 相同 |

五组 PNG 文件内容完全一致，见 比较日志（`p01-image-comparison.log`，仅本地）。宽窗的取景尺寸不变，竖窗的立方体尺寸按 2/3 缩放（允许整数像素舍入误差），符合 3:2 比例留边。


构建日志为 `p00-build-*.log` / `p01-build-*.log`，自检日志为 `p00-smoke-*.log` / `p01-smoke-*.log`。窗口日志为 P00（`p00-window.log`，仅本地） 和 P01（`p01-window.log`，仅本地）；异常结果见 汇总（`p01-errors.log`，仅本地） 与 `p01-error-missing/compile/link.log`。P01 交付源码与配置的 SHA256 见 版本清单（`p01-source-sha256.log`，仅本地）。构建日志已从 MSBuild 默认本地编码转存为 UTF-8，内容未改写；复验脚本直接输出 UTF-8 日志。

## 本轮命令和复验

从仓库根目录运行，使用 PowerShell 7（`pwsh`）。构建脚本用 vswhere 查找 MSBuild，设置子进程环境后构建，并随每次构建运行隐藏窗口自检。本轮开始时 MSBuild 因 `HTTPS_PROXY` / `https_proxy` 同名变量报 MSB6001；脚本将变量名转为大写并去重后成功。它不修改用户或系统环境变量。

本轮按以下顺序执行；P00 命令执行时源码尚未拆分，当前直接重跑不会重新得到历史基线：

```powershell
# P00：拆分前
& ./文档/验收/P00-P01/build.ps1 -Stage p00
& ./文档/验收/P00-P01/verify-window.ps1 -Stage p00

# P01：先抽出 Mesh，再抽出 App
& ./文档/验收/P00-P01/build.ps1 -Stage p01-mesh -Platforms x64 -Configurations Debug -Target Build
& ./文档/验收/P00-P01/build.ps1 -Stage p01
& ./文档/验收/P00-P01/verify-window.ps1 -Stage p01
& ./文档/验收/P00-P01/verify-errors.ps1 -Stage p01
```

当前版本复验：

```powershell
& ./文档/验收/P00-P01/build.ps1
& ./文档/验收/P00-P01/verify-window.ps1
& ./文档/验收/P00-P01/verify-errors.ps1
```

`verify-window.ps1` 需要可访问的交互桌面，并短暂显示和调整测试窗口；当前 Codex 文件沙箱无法截取桌面，实际窗口验收在允许桌面访问的执行环境完成。工具使用每线程 DPI 感知以准确截取客户区，并等待首帧绘制完成；截图不包含窗口外的桌面内容。显示区域必须能容纳测试窗口（最大客户区 1200×600 或 600×900，另加边框及 100 px 起始偏移）。

尚未在课程演示设备或其他显卡验证。P00/P01 已验收，阶段 A 仍需 P02 的移动、转向和复位。
