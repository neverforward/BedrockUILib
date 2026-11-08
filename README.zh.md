# BedrockUILib

面向 LeviLamina 的**纯客户端**前置库：让其他客户端模组用**游戏自己的 UI 栈**创建屏幕、面板和
HUD 元素——也就是游戏画自己那种信息框、设置面板的方式。它不是叠在游戏画面上的外部覆盖层窗口，
也不在游戏的 UI 渲染过程之外画东西。

> 状态：**仓库骨架**。目前没有公共 API。设计决策见
> [docs/investigation-native-ui.md](docs/investigation-native-ui.md) 和
> [docs/architecture.md](docs/architecture.md)。

## 它不是什么

- 不是玩家向模组：除了 `--tests=y` 里的 demo，它不提供任何自己的界面。
- 不是服务端模组：`--target_type=server` 在配置阶段直接报错，这是设计。
- 不是带外观的 UI 套件：颜色、圆角、字体、间距都是使用者提供的主题 token，默认值只是占位。

## 环境要求

- Windows x64，Minecraft 基岩版 GDK 客户端（1.26.60.x），通过 LeviLauncher 启动。
- [xmake](https://xmake.io) + clang-cl，C++20。
- LeviLamina client 26.60.x（版本钉在 `xmake.lua`）。

## 构建

```bash
xmake f -y -p windows -a x64 -m release --target_type=client
xmake build BedrockUILib
```

产物在 `bin/BedrockUILib/`（dll、pdb、manifest.json、`lang/`）。`lang/` 是运行时目录，不在 dll 里。

## 测试

```bash
xmake f -y -p windows -a x64 -m debug --tests=y --target_type=client
xmake build BedrockUILibTests
xmake run BedrockUILibTests
```

跑完测试要重新配回 release 且 `--tests=n` 再编模组本体，规则见 [CONTRIBUTING.md](CONTRIBUTING.md)。

## 演示面板（只在 `--tests=y` 里）

演示面板永远不进发布产物：只有开测试时 `src-demo/` 才会加进模组目标。游戏内验证步骤：

```bash
xmake f -y -p windows -a x64 -m debug --tests=y --target_type=client
xmake build BedrockUILib
```

把 `bin/BedrockUILib/` 拷到 `<实例>/mods/BedrockUILib/`，进游戏后：

- **F6** 开关面板；
- **Esc** 关闭面板。

面板打开的是一个画廊首页，点击按钮进入各组件自己的页面（Buttons、Input），每个页面就是该组件的参考文档，
`Back` 返回首页。

[docs/investigation-native-ui.md](docs/investigation-native-ui.md) 的 "Minimal runnable verification" 一节。

## 版本

0.x 期间只保证源码兼容。版本号同时出现在 `src/bul/Version.h`、`CHANGELOG.md` 和 git tag，三处必须一致。

## 许可证

MIT（见 [LICENSE](LICENSE)）。
