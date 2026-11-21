# 架构

状态：**骨架 + 可行性 demo**。下面的模块图和公共 API 是计划；`Geometry`、`Color`、`TextMetrics` 和 `gfx::Painter` 已经存在，`--tests=y` 的 demo 面板能走引擎 UI pass 画出来（F6 开关，Esc 关闭）。其余都还没做。

## 模块图

```
src/                         纯逻辑，不 include mc/ 和 ll/，有单元测试
  bul/Geometry.h             Rect、Point、半开区间包含、insets、居中
  bul/Color.h                Rgba，带 fromRgb / fromArgb / withAlpha
  bul/TextMetrics.h          度量接口 + UTF-8 安全的 truncateToWidth
  bul/Version.h              版本和 ABI 宏
  bul/Layout.*               计划中：行/列/滚动/网格/表格求解
  bul/HitTest.*              计划中：点 -> 控件，考虑 z-order
  bul/Focus.*                计划中：焦点顺序、Tab/方向键切换、模态作用域
  bul/Theme.*                计划中：主题 token + 中性默认值 + 覆盖
  bul/Binding.*              计划中：值提供者和脏刷新记账

src-client/                  引擎适配层，无单元测试，在游戏里手动验证
  mod/                       LeviLamina 入口、i18n 加载、生命周期
  bul/gfx/Painter.*          立即模式 UI pass 绘制（填充/描边/文字）
  bul/gfx/TextRenderer.*     计划中：TextMetrics 的引擎实现
  bul/UiRoot.*               计划中：图层栈、每帧 update 和 draw
  bul/ScreenHost.*           计划中：事件接线和图层托管
  bul/InputRouter.*          计划中：指针/按键路由、吞事件策略
  bul/Cursor.*               计划中：软件光标策略和记账
  bul/Screens.*              计划中：注册 API 和单次安装保护

src-demo/                    只在 --tests=y，永不发布
  bul/demo/DemoPanel.*       可行性面板（AfterUIRenderEvent 监听器）
```

## 帧模型

1. 开屏 = 在客户端场景栈上推一个**原生场景**（`SceneFactory::createCommonDialogInfoScreen` -> `ISceneStack::pushScreen`）。之后输入、光标、Esc/返回都归引擎管。关闭用 `schedulePopScreen(1)`。
2. 渲染 pass 回调（引擎）-> `ScreenHost` -> `UiRoot::draw(Painter&)`，只有我们的场景在栈顶时才走这条路。
3. `InputRouter` 收到指针/按键事件，用上一帧的命中测试结果同步决定是否吞掉，真正的处理进队列。只有我们自己的控件消费事件；模态场景本来就把输入挡在世界和下层屏幕之外。
4. 客户端线程上每帧一次，`UiRoot::update()` 依次跑队列里的事件、焦点变化和每帧回调。
5. 同一个 update 里跑看门狗。看门狗不能挂在可关闭屏幕的 draw 路径里：UI 一关就停的看门狗来不及清理光标、hook 或捕获的输入，表现出来的症状就是"模组把鼠标搞坏了"。

## 公共 API（计划，尚未实现）

```cpp
#include "bul/BedrockUILib.h"

auto& ui = bul::service();                 // 幂等，调两次也安全

bul::Theme theme = bul::Theme::neutral();  // 占位默认值，全部可覆盖
theme.color.text = 0xE0E0E0;
ui.setTheme(theme);

ui.registerScreen("my_mod.settings", [] (bul::Screen& screen) {
    screen.row([&] {
        screen.label("Title");
        screen.button("Apply", [] { /* ... */ });
    });
});

ui.push("my_mod.settings");                // Esc 弹出，Tab 移动焦点
ui.hudElement("my_mod.hud", [] (bul::Hud& hud) { hud.text("..."); });
```

引擎侧部件落地之前，接口形状都可能调整。

## 测试计划

单元测试在 `src-test/common`（不依赖游戏）：

- 布局求解：尺寸、对齐、换行、滚动，含嵌套行和列；
- 命中测试：z-order、裁剪、禁用控件；
- 焦点顺序和键盘路由（Tab、Shift+Tab、方向键、Enter、Esc）、模态作用域、关闭时焦点恢复；
- 通过 `TextMetrics` 接口做文字度量和截断（假实现，固定字宽）；
- 主题 token 解析和回退到中性默认值；
- 绑定刷新：脏传播，没变化就不重绘；
- 版本和 ABI 宏。

游戏内手动验证，逐步留档：

- 面板画在引擎 UI pass 内，跟随 GUI 缩放设置；
- 指针和键盘正常；光标只出现和消失一次；
- 中文走引擎字体渲染；
- 关闭 UI 后输入还给游戏，无残留。