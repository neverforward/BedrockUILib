# BedrockUILib

面向 LeviLamina 的纯客户端前置库。其他客户端模组可以用它基于游戏自己的 UI 栈创建屏幕、面板和 HUD 元素——和游戏画信息框、设置面板用的是同一套东西。它不是叠在游戏画面上的覆盖层窗口，所有渲染都在游戏的 UI pass 之内。

## 环境要求

- Windows x64，Minecraft 基岩版 GDK 客户端（1.26.60.x），通过 LeviLauncher 启动。
- [xmake](https://xmake.io) + clang-cl，C++20。
- LeviLamina client 26.60.x。

## 构建

```bash
xmake f -y -p windows -a x64 -m release --target_type=client
xmake build BedrockUILib
```

产物在 `bin/BedrockUILib/`

## 测试

```bash
xmake f -y -p windows -a x64 -m debug --tests=y --target_type=client
xmake build BedrockUILibTests
xmake run BedrockUILibTests
```

测完之后要重新配置回 release（`--tests=n`）再编模组本体，规则见 [CONTRIBUTING.md](CONTRIBUTING.md)。

## 演示面板（仅 `--tests=y`）

发布构建不包含演示面板：只有开启测试时 `src-demo/` 才会加进模组目标。游戏内验证：

```bash
xmake f -y -p windows -a x64 -m debug --tests=y --target_type=client
xmake build BedrockUILib
```

把 `bin/BedrockUILib/` 拷到 `<实例>/mods/BedrockUILib/`，启动游戏后：

- **F6** 开关面板；
- **Esc** 关闭面板。

## 文档

文档是 `docs/` 下的 VitePress 站点（根目录英文，`docs/zh/` 中文）：

```bash
cd docs
npm install
npm run dev      # 本地预览
npm run build    # 静态站点生成到 docs/.vitepress/dist/
```

## 版本

0.x 期间只保证源码兼容。

## 许可证

MIT（见 [LICENSE](LICENSE)）。