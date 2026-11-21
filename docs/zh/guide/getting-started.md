# 快速开始

BedrockUILib 是 [LeviLamina](https://github.com/LiteLDev/LeviLamina) 的客户端库。
其它客户端模组用它把界面、面板和 HUD 元素画在游戏自己的 UI 栈上——就是游戏用来
画信息框和设置面板的那套栈。它不是覆盖层窗口：所有内容都在游戏 UI pass 里绘制，
所以会跟随界面缩放，并和其余 UI 一起排序。

## 环境要求

- Windows x64、通过 LeviLauncher 启动的 Minecraft Bedrock GDK 客户端（1.26.60.x）。
- [xmake](https://xmake.io) + clang-cl、C++20。
- LeviLamina 客户端 26.60.x。

## 构建

```bash
xmake f -y -p windows -a x64 -m release --target_type=client
xmake build BedrockUILib
```

产物在 `bin/BedrockUILib/`。

## 测试

```bash
xmake f -y -p windows -a x64 -m debug --tests=y --target_type=client
xmake build BedrockUILibTests
xmake run BedrockUILibTests
```

构建模组本体前记得切回 release（`--tests=n`）。项目规则见
[CONTRIBUTING.md](https://github.com/neverforward/BedrockUILib/blob/main/CONTRIBUTING.md)。

## Demo

demo 面板不进 release 构建：只有开启测试时 `src-demo/` 才会加入模组目标。

```bash
xmake f -y -p windows -a x64 -m debug --tests=y --target_type=client
xmake build BedrockUILib
```

把 `bin/BedrockUILib/` 拷到 `<实例>/mods/BedrockUILib/`，进游戏后：

- **F6** 开关画廊，**Esc** 关闭；
- 画廊就是文档：首页列出各组件，每个组件一页，每页只用公开 API 编写。

::: tip
游戏运行时 `BedrockUILib.dll` 会被占用（文件锁），复制新构建前请先完全退出游戏。
:::

## 在另一个模组里使用

库是一个带导出 C++ API 的 DLL（`bul/Export.h`）：消费者模组把 `src/` 和
`src-client/` 加入 include 路径、链接构建产物旁边的 `BedrockUILib.lib`，并且要用
相同的编译工具链和 LeviLamina SDK 版本（C++ ABI 跨 dll）。细节见
[API 导出](/zh/components/api-export)。

## 文档地图

| 文档 | 回答什么 |
| --- | --- |
| [组件](/zh/components/) | 控件库：布局、表单字段、每个控件、样式分层和声明式 spec 树。 |
| [架构](/zh/architecture) | 模块划分、输入/光标/文本的限制，以及哪些有单测、哪些在游戏里验证。 |
| [原生 UI 调研](https://github.com/neverforward/BedrockUILib/blob/main/docs/investigation-native-ui.md) | 这个客户端上原生 UI 怎么做、引擎提供什么、为什么走现在的实现路线。 |

## 许可证

MIT（见 [LICENSE](https://github.com/neverforward/BedrockUILib/blob/main/LICENSE)）。
