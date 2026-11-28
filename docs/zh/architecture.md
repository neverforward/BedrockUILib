# 架构

状态：模型层和模态宿主已经实现并有单测；HUD 元素注册表和带名字的界面服务（`ui.registerScreen`、`ui.push`）尚未实现。

## 分层

```
src/                     纯 C++，不 include mc/ 或 ll/，有单元测试
  bul/Geometry.h         Rect、Point、半开区间包含、内边距
  bul/Color.h            Rgba，fromRgb / fromArgb / withAlpha
  bul/TextMetrics.h      测量接口 + UTF-8 安全的截断
  bul/Canvas.h           模型绘制用的画布接口（fill/outline/text）
  bul/Theme.*            颜色 token 与中性默认值
  bul/Style.*            每控件的颜色覆盖
  bul/Selector.*         选择器：解析、匹配、优先级
  bul/StyleSheet.*       规则、级联与链式构造器
  bul/Layout.*           行/列、网格、表格、滚动求解
  bul/TextEdit.*         编辑模型：光标、选区、插入、删除
  bul/Widget.h           控件套件的总头文件
  bul/widget/*           Widget 基类 + 每个组件一对文件，以及 TextFlow.h
  bul/Spec.*             声明式 spec 节点与建树
  bul/SpecBuilder.*      命令式 builder

src-client/              引擎适配层，在游戏里验证
  bul/gfx/Painter.*      基于 MinecraftUIRenderContext 的即时绘制
  bul/gfx/GameCanvas.*   Canvas 实现：绘制与裁剪
  bul/Modal.*            公共 API：showModal、closeModal、setTheme、theme
  bul/modal/*            宿主内部：状态、场景钩子、渲染、输入、文本
  bul/platform/*         系统剪贴板与文件对话框
  mod/*                  LeviLamina 入口与客户端服务

src-demo/                只在 --tests=y 时构建，不随发布
  bul/demo/DemoPanel.*   画廊：每个组件一页，F6 开关
```

模型层不依赖 `mc/` 和 `ll/`，测试不启动游戏也能编译。只有 `src-client` 接触 SDK。

## 帧模型

1. `showModal(tree)` 创建一个占位界面（`SceneFactory::createCommonDialogInfoScreen("", "")`）并压入客户端场景栈。占位界面提供库本身不掌握的东西：引擎的输入捕获、光标和 Esc 处理。
2. `UIScene::$render` 上的钩子认出这个占位界面，用控件树替换它自己的绘制。`ModalRender` 按窗口尺寸测量、布局并经由 `GameCanvas` 绘制，弹出层在控件树之后画。
3. 关闭时先等占位界面成为栈顶，再 `schedulePopScreen(1)`。退出钩子清空焦点、指针捕获、hover 和待处理的文本编辑，关掉的窗口不留状态。
4. `ModalInput` 监听 `ll::event::input::KeyInputEvent` 和 `MouseInputEvent`。弹窗打开期间鼠标按键事件被取消，移动事件放行，因为引擎要靠指针位置做命中测试。点击先移动焦点到可聚焦控件，再按 `hitTest` 派发；左键按下可以捕获指针（`press`/`drag`/`release`），滚动条拖动就靠它。按键交给焦点控件，Tab 和 Shift+Tab 切换焦点，Esc 关闭。
5. 滚轮事件带的是 Windows 滚轮增量（一格 120）。它先交给指针下的控件，沿容器向上冒泡，`ScrollView` 再乘上 `wheelStep`。
6. 引擎没有"输入文字"的事件。`ModalText` 钩住客户端输入回调，把字符交给焦点输入框；文本框有焦点时宿主通过 `KeyboardManager` 声明持有平台键盘，打字和 IME 才成立。

## 绘制规则

- `Painter` 保持引擎的批处理顺序：矩形、图片、文本。`flush()` 结束一个 z 层，之后画的内容在上面。
- 文本是延迟提交的：`flushText` 用的是**提交那一刻**的裁剪矩形。`ScrollView` 在收窄裁剪前 flush 一次（避免裁掉前面控件已入队的文本），在裁剪内再 flush 一次（让自己的文本被裁），然后恢复裁剪。
- 裁剪按交集嵌套，画家维护矩形栈，把交集交给引擎。
- 坐标是 GUI 单位，`Painter` 在填充和裁剪时都换算成引擎的 `RectangleArea`。

## 公共 API

消费者现在这样用：

```cpp
#include "bul/Modal.h"
#include "bul/SpecBuilder.h"

bul::Theme theme = bul::Theme::neutral();
theme.text = bul::Rgba::fromRgb(0xE0E0E0);
bul::setTheme(theme);

bul::showModal(bul::spec::build([](bul::spec::Builder& ui) {
    ui.label("Settings", 1.4f);
    ui.row([&] {
        ui.button("Apply", [] {});
        ui.button("Cancel", [] {});
    });
}));
```

计划中的服务层给界面一个名字，并加上 HUD 注册：

```cpp
ui.registerScreen("my_mod.settings", [](bul::Screen& screen) { /* builder */ });
ui.push("my_mod.settings");
ui.hudElement("my_mod.hud", [](bul::Hud& hud) { hud.text("..."); });
```

`Screen` 会包住现有的 builder，`Hud` 复用同一棵树并加每帧更新钩子。这些部件落地前，接口形状可能调整。

## 测试计划

`src-test/common` 有 209 个测试，不需要启动游戏：

- 几何与布局求解：行、列、网格、表格、滚动区间；
- 文本排版、CJK 字宽、换行与截断；
- 编辑模型：光标、选区、插入、删除、IME 增量流；
- 每个组件：测量、布局、绘制、命中测试、状态与回调；
- theme、style、stylesheet 的解析与特异性；
- spec 节点与 builder：参数、链式、id、自定义控件；
- 不依赖弹窗的协议部分：滚轮路由、裁剪调用、指针捕获。

在游戏里手验：

- 窗口画在游戏 UI pass 内，跟随 GUI 缩放；
- 鼠标、滚轮、键盘正常，滚动条可拖动，中文与光标走引擎字体；
- 关闭后输入交还游戏，没有残留。
