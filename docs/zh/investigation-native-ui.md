# 原生 UI 调研

状态：**审计记录 + 决策**。维护者于 2026-11-03 选择自绘路线；其后果写在
[architecture.md](architecture.md) 里。本文保留两条路线的证据，这样这个决定可以被重新
审视，而不必重新调研。

范围：一个纯客户端 LeviLamina 模组，如何在这个 GDK 客户端（Minecraft Bedrock 1.26.51.01
+ LeviLamina client）上创建一个*原生*界面 / 面板 / HUD 元素，也就是画在游戏自己的 UI pass
里的 UI，而不是外部渲染器窗口。

## 结论

引擎只提供一种拿到它自己控件的方式，而本项目没有走这条路。

* **引擎原生控件需要 JSON UI 定义。** 控件树只能由引擎从*已解析的 UI 定义*
  （`UIControlFactory` + `UIResolvedDef`）实例化；模组没法 `new UIControl`，所以"用 C++
  搭引擎控件树"在这个 SDK 里不存在。界面随后按名字创建（`SceneFactory::createUIScene`），
  包装成场景（`SceneFactory::_createScreen`）再压入客户端场景栈
  （`ISceneStack::pushScreen`），并用一个 `ScreenController` 子类承载数据与事件。定义放在
  模组目录里的资源包中，由 LeviLamina 自己注入。
* **这条路线调研过，随后被维护者否决（2026-11-03）。** 库改为通过
  `MinecraftUIRenderContext` 在游戏 UI pass 里自绘控件，并自己负责布局、命中测试、焦点、
  光标和文本输入。这个生态里所有复杂的客户端 UI 都走这条路（见"参考项目"）。
* 接受下来的代价列在 [architecture.md](architecture.md) 里。下面关于 JSON UI 的发现保留
  在本文中，作为替代方案的记录，以及"引擎能做但我们刻意不用"的记录。

## 证据

### 1. 路线 A（代码构建控件树）在这个 SDK 上不可行

| 事实 | 证据 |
| --- | --- |
| `UIControl` 无法构造：默认构造函数声明在 "prevent constructor by default" 下，没有 `MCAPI` 实现 | `...\levilamina-client-sdk\26.51.5\70daec...\include\mc\client\gui\controls\UIControl.h:114-117` |
| `UIControlFactory` 只能从 `UIResolvedDef` / `UIControl const& template` 建树 | 同一包 `...\controls\UIControlFactory.h:54-137` |
| `UIScene` 的公开构造函数需要 `VisualTree`、`UIControlFactory`、`UIMeasureStrategy`、`LayoutManager` | `...\screens\UIScene.h:222-232` |
| `ScreenView`（真正负责渲染/测量树的对象）没有可用的构造函数 | `...\screens\ScreenView.h:188-192` |

`UIResolvedDef` 来自 JSON UI 仓库（`IUIDefRepository`，从 `ResourcePackStack` 加载），而
这个接口是只读的（`loadDefsList`、`findDef`、`forEachControl`……）。所以游戏里的每一棵
控件树都源自 JSON。

### 2. 路线 B 是受支持的路线，场景层是公开 API

| 事实 | 证据 |
| --- | --- |
| 用定义名 + controller 创建界面 | `SceneFactory::createUIScene(std::string const& screenName, std::shared_ptr<ScreenController>)` — `...\screens\SceneFactory.h:719-720` |
| 把 `BaseScreen` 包装成场景 | `SceneFactory::_createScreen(std::shared_ptr<BaseScreen>)` — `SceneFactory.h:209` |
| 压入 / 弹出场景 | `ISceneStack::pushScreen(std::shared_ptr<AbstractScene>, bool flush)`、`schedulePopScreen`、`isOnSceneStack(std::string const&)` — `...\screens\interfaces\ISceneStack.h:183-261` |
| 从模组拿到工厂 / 场景栈 | `ClientInstance::getSceneFactory()`、`getClientSceneStack()`、`getMainSceneStack()` — `...\mc\client\game\ClientInstance.h:882,888,896`；`ll::service::getClientInstance()` — `docs\ll\src-client\ll\api\service\TargetedBedrock.h:13` |
| controller 接缝（数据 + 输入 + tick） | `ScreenController::{bind, bindString, bindBool, bindFloat, bind*ForCollection, bind*ForAnyCollection, registerButtonEventHandler, tick, preFrameTick, tryExit, onOpen, onLeave}` — `...\screens\ScreenController.h:391-482`；`MinecraftScreenController(std::shared_ptr<MinecraftScreenModel>, ScreenExitBehavior, bool)` — `...\screens\controllers\MinecraftScreenController.h`；`ScreenExitBehavior` — `...\controllers\ScreenExitBehavior.h` |
| 往绑定属性写值 | `UIPropertyBag::set(std::string_view, mce::Color / glm::vec2 / glm::ivec2)` — `...\controls\UIPropertyBag.h:125-129` |
| 引擎侧的输入/焦点/滚动：键盘、鼠标、手柄、Esc、Tab 顺序、焦点容器、滚动视图都作为组件存在 | `...\controls\{InputComponent,FocusComponent,FocusContainerComponent,ScrollViewComponent,GridComponent,ButtonComponent,ToggleComponent,SliderComponent,TextEditComponent,DropdownComponent,TooltipTriggerComponent}.h` |

### 3. 模组资源包由 LeviLamina 自己注入

`docs\ll\src-client\ll\core\main_win.cpp`

* `InitPacksHook`（131-164 行）把正在运行的模组自己的 `resource_packs` 目录
  （`Mod::getResourceDir()`，定义在 `docs\ll\src\ll\api\mod\Mod.cpp:48`）**以及其它所有
  模组的目录**注册为 `PackType::Resources` / `PackOrigin::User` 目录包来源。
* `InitPacksHook2`（166-195 行）遍历所有已加载、路径包含 mods 路径的包，推进 UI 定义所
  依赖的资源栈。

所以客户端模组只要放 `mods/<Mod>/resource_packs/<Pack>/`，里面的定义在 UI 加载前就能被
`IUIDefRepository` 拿到。模组本身不需要做别的事。

### 4. 这个客户端上已有可运行的先例（就装在本地实例里）

| 模组 | 资源包 | 证明了什么 |
| --- | --- | --- |
| `mods/Advancements` | `resource_packs/Advancements/{ui/advancements_screen.json (25 KB), ui/advancements_toast.json, ui/hud_screen.json, ui/pause_screen.json, texts/zh_CN.lang}` | 一个完整的自定义界面（`advancements.advancements_screen@common.base_screen`）、一个 HUD "信息框"（往 `hud_screen` 里插 toast）、`pause_screen` 里基于 patch 的按钮，以及通过 `texts/zh_CN.lang` 的中文标签文本。它的 DLL 导入了 `SceneFactory::createUIScene`、`SceneFactory::_createScreen`、`MinecraftScreenController::ctor`、`ScreenController::bindString` / `bindStringForAnyCollection` / `bindItemDataForAnyCollection` / `registerButtonEventHandler`、`UIPropertyBag::set`。 |
| `mods/LL-ModMenu` | `resource_packs/ModMenu/ui/start_screen.json` | 最小注入：`main_buttons_panel/stacked_rows` + `modifications`/`insert_after` 给开始界面加一个原生按钮，由 `button.mods_button_clicked` 处理。 |
| `mods/playback` | `resource_packs/playback-ui/ui/start_screen.json` | 同样的模式，第三个独立实现。 |

Advancements 的包也是目前最好的 JSON UI 踩坑笔记（`advancements_screen.json` /
`advancements_toast.json` 里有中文注释）：`layer` 顺序与物品图标、`keep_ratio`、
`#size_binding_*` 单位、集合虚拟化。

### 5. 参考项目：生态实际怎么做

下面五个项目用了两种机制之一，没有一个是用代码构建引擎控件树的（从另一个方向印证了
第 1 节）。

| 项目 | UI 机制 | 证据 |
| --- | --- | --- |
| `MiracleForest/BarelyEnoughItems` | 通过 `MinecraftUIRenderContext` 自绘 | `src/ui/gfx/Painter.h` —— "thin batching wrapper over `MinecraftUIRenderContext`"，严格顺序 rects -> `flushImages` -> item icons -> text -> `flushText`；控件在 `src/ui/widgets/*` |
| `MiracleForest/Bedstones` | 同一套框架，外加注入原版界面 | `src/client/ui/{gfx/Painter, UiRoot, screen/Screens.h, widgets/{Style,Widgets,TextBox,Tooltip}}`、`src/client/input/InputRouter.h`、`src/client/inventorybutton/PanelLocator.h`（找到原版 176x166 的 `player_inventory` 面板矩形并在上面放一个按钮） |
| `MiracleForest/AlleviatedBackpacks` | 同一套框架 | `src/client/ui/{gfx/Painter, screen/BackpackScreen, screen/InventoryParts, screen/Tabs, widgets/*}`、`src/client/input/InputRouter.cpp`；客户端伴侣通过自己服务端发来的包接收界面状态 |
| `amatouhake/Lamium` | 同一套框架 + 世界空间覆盖层 | `src/ui/{Widgets,SettingsScreen,HudEditor,Toast,TextFit,TranslationsZhCN}`、`src/overlay/*`；`tests/` 纯逻辑 + `tests-native/` SDK 类型（记录在 `AGENTS.md`） |
| `honeymirror/RenderDragonShaderLoader` | 都不是 —— RenderDragon 着色器/材质管线 | 只有在我们需要在 UI 系统之外渲染时才相关；界面/HUD 用不到 |

前四个项目共用的客户端 UI 框架（`Painter` + `UiRoot` + `UiLayer` + 控件 + `InputRouter`）
在这里是事实标准，而且是*全有或全无*的：模组既然自己画像素，就也得自己负责

* 命中测试（`InputRouter` 在窗口线程决定是否吞掉事件，依据是上一帧的命中结果，然后在客户端
  线程从 `ClientInstance::update` 重放），
* 焦点 / Tab 顺序 / 文本焦点按键捕获，
* 光标与 GUI 缩放处理，
* 文本输入（它们自己的 `TextInput`/`TextBox` + 剪贴板辅助，因为这里 IME 不工作 —— 见项目
  笔记里"只能粘贴"的说明）。

这就是自绘的代价。它一开始被任务排除（引擎应当拥有输入/焦点/光标），后来在 2026-11-03 被
维护者接受，现在是这个库最核心的工程问题 —— 见"所选路线的风险"。它们的代码仍是目前最好的
参考：**UI pass 内的绘制顺序**、控件外观（`Style.h` = 原版外观）、布局数学和测试结构。

| 借鉴（与机制无关） | 不采用 |
| --- | --- |
| `Painter` 的严格分层顺序（背景 -> 物品 -> 前景 -> 文本）和"每个 z 层只 flush 一次"的规则 | 整套 `InputRouter` 和控件树（我们是带公开 API 的库，它们是每个模组各自一套） |
| `PanelLocator` 的思路：从上一帧查原版控件矩形，把东西放在它旁边 | 服务端权威的伴侣协议（本库是纯客户端） |
| 仓库布局：纯逻辑核心（不 include `mc/`/`ll/`，有单测）+ gtest 目标 + 游戏内自测 + 构建后合并 `lang/` | 每个模组自己的贴图/资源管线 |
| i18n 拆分：模组字符串在 `lang/<locale>.json`，资源包字符串在 `texts.d/<locale>/*.lang`，重复键让构建失败 | |

### 6. DDUI（新的数据驱动表单）曾作为底座被考虑，最终否决

* `GameplayUI::DataDrivenScreenAPI::showDataDrivenScreen(screenId, formId, dataInstanceId)`
  在客户端存在（`...\mc\client\gui\gameplay\api\gameplayui\DataDrivenScreenAPI.h:60`），
  schema 层级很高（`Panel`、`FormButton`、`FormSwitch`、`FormSlider`、`FormDropdown`、
  `FormTextField`、`FormScrollView`、`ScrollableGridLayout`……）—— 很有吸引力。
* 但它的数据路径是一个通常由服务端创建的 datastore 实例 id
  （`ll::ui::ScreenSession` 仅服务端：`ll\api\ui\base\ScreenSession.h:20,48-50`），而且它是
  *表单*界面（模态对话框），不是通用的面板/HUD 表面。以后最多作为表单的便利层再考虑。

## 所选路线的钩子与引擎入口

JSON UI 路线一个都不需要。所选路线需要引擎入口，并且由库来拥有它们，这样消费者不用自己
hook（两个依赖方 hook 同一个函数，正是这条规则要避免的故障）。这里的东西都还没实现；
第 3 步会选定并验证确切的输入 hook。

| 需求 | 入口 | 状态 |
| --- | --- | --- |
| 在 UI pass 内绘制 | `ll::event::BeforeUIRenderEvent` / `AfterUIRenderEvent`（LL 事件，给出 `ScreenView&` + `MinecraftUIRenderContext&`；`docs\ll\src-client\ll\api\event\render\UIRenderEvent.h`） | 候选，不需要 MinHook |
| 按键事件 | `ll::input::KeyRegistry::getOrCreateKey` + `KeyHandle` 回调（`docs\ll\src-client\ll\api\input\KeyHandle.h`） | 热键已定 |
| 指针 / 滚轮事件 | 尚未选定：`ClientInputCallbacks::handlePointerLocationWithId` / `MinecraftInputHandler` 是候选；需要 MinHook 或 thunk | 第 3 步 |
| 每帧更新（输入重放、看门狗） | 客户端线程每帧一次，由同一个渲染事件或客户端 update 事件驱动 | 第 3 步 |
| 光标 | 不 hook：画软件光标，并处理 `ShowCursor` 调用 | 第 3 步 |

规则：在 enable 时装一次，disable 时移除，并让看门狗跑在每帧都会执行的路径上，无论我们的 UI
是否可见。

## API 后果（所选路线）

控件树是我们的，所以 API 是 C++ 控件树，不是 JSON schema，也不是资源包：注册一个界面或 HUD
元素、加行/标签/按钮、设主题、拿每帧回调。因此布局求解、命中测试、焦点顺序、滚动和文本测量
都放在 `src/` 里作为纯逻辑并做单测；只有绘制、输入源和文本测量实现放在 `src-client/`。
计划中的形态见 [architecture.md](architecture.md)；尚未实现。

## JSON UI 路线的风险（记录，不采用）

1. **JSON UI 没有文档且与版本绑定。** `common.base_screen`、`common.item_renderer`、
   `pause.pause_button_template`、`start.main_button` 这类名字是引擎内部符号，游戏更新可能
   改名。缓解：库的包保持最小、只复制在这个客户端上验证过的模式、每次 Bedrock 更新后复验，
   核心功能绝不依赖 patch 原版界面。
2. **包加载时机。** 包在 UI 加载前注册；新增/修改定义需要重启游戏。对未知名字调用
   `createUIScene` 必须当作运行时失败处理：校验 + 日志 + i18n 错误，绝不崩溃。
3. **原生标签里的中文文本在这个具体构建上未验证。** Advancements 模组带了
   `texts/zh_CN.lang` 并把那些字符串绑进标签，这是强证据，但仍需在屏幕上亲眼见到一次。这是
   可行性 demo 的验收点 #1，因为这里失败会改变整个产品（任务里明确写了）。
4. **HUD z 顺序。** Advancements 发现 `layer` 100 的面板会盖住物品图标，而面板 1 + 图标
   `layer` 60 正常。我们的 HUD 默认值必须留在已知可用的区间内，HUD 的数据推送路径也还开着
   （见"需要的钩子"）。
5. **集合是虚拟化的：** 只有视口内的项会被实例化。不要假设每个模型行都有对应控件。
6. **JSON UI 单位陷阱：** `#size_binding_x/y` 是父尺寸的倍数而不是像素；`keep_ratio` 默认
   为 true（拉伸的 1px 图片会消失）；`#texture` 需要有默认贴图。
7. **界面名冲突**：库用一个命名空间前缀（`bul.*`），消费者界面用自己的前缀，注册时校验
   唯一性。
8. **打包：** `@levibuildscript/modpacker` 只复制 DLL、PDB 和 `manifest.json`
   （`...\levibuildscript\0.6.2\...\rules\modpacker.lua:25-53`）。`resource_packs/` 和
   `lang/` 必须由 `after_build` 步骤和发布流程复制，否则发出去的 DLL 没有 UI 定义。
9. **`tooth.json` 必须带上资源包**（`files`/placements），否则 Lip 装出来的 DLL 没有定义。
10. **在 JSON UI 控件里自绘未验证。** `MinecraftUICustomRenderer` 存在
    （`...\controls\renderers\MinecraftUICustomRenderer.h`），原版控件也在用
    （`inventory_item_renderer`、`hud_*_renderer`），但 `UIControlFactory` 没有公开的注册
    hook，所以第三方渲染器名字不可假设。Tier 1 严格保持声明式；如果消费者需要自定义视觉
    （图表、预览），退路是用 `MinecraftUIRenderContext` 画一个 HUD 层，而不是往面板里塞
    控件。
11. **两种视觉方言。** 生态模组看起来像 Java/JEI（自绘），引擎 JSON UI 看起来像 Bedrock。
    期待前者的消费者需要 Tier 2（自己的 JSON UI），那是另一项技能，不是 C++。

## 所选路线（自绘）的风险

1. **光标归我们管。** 独占全屏不会合成硬件光标（`GetCursorInfo` 说在显示，但屏幕上什么
   都没有），而且游戏每帧把 `ShowCursor` 计数往负里推。指针 UI 必须画软件光标，并精确地
   处理自己发出的每一次 `ShowCursor`。Insight 在这里烧掉了好几轮。
2. **文本输入无法自造。** 这个 GDK 客户端从不产生 IME 组合（`GCS_COMPSTR` 为空，
   `WM_IME_*` 不存在），所以文本录入必须走游戏自己的文本框或剪贴板粘贴。文本框因此是这套
   控件里最贵的原语。
3. **抢焦点是陷阱。** `SetForegroundWindow` / `AttachThreadInput` 链式调用会跟客户端打架，
   最终把整个游戏的输入搞坏。
4. **吞事件的策略归我们。** 每个被吞掉的事件都必须再放出来，而且只有按下被吞时才吞抬起；
   否则世界收不到点击，或者 UI 在拖拽中途关闭。
5. **通过引擎字体渲染中文，对我们自己的绘制仍未验证。** 引擎在原版文本里渲染中文；我们的
   测量和绘制必须走同一条字体路径（`MinecraftUIRenderContext` 文本），第一个 demo 会在
   屏幕上确认它。
6. **原版还原度归我们。** 没有引擎控件，"看起来像游戏"就是复刻工作：度量、九宫格、
   hover/按下状态、焦点环。
7. **性能归我们。** 命中测试在窗口线程用上一帧的布局跑；长列表需要虚拟化，布局必须缓存，
   不能每个事件都重算。
8. **hook 生命周期。** 库的输入/指针 hook 是进程级的：必须在 disable 时移除，并由一条
   始终运行的路径上的看门狗保护，否则拆卸失败看起来就是"模组把鼠标搞坏了"（项目笔记里
   记录的症状）。
9. **GUI 缩放和 DPI。** 所有几何都在 GUI 单位里；与屏幕像素的互转必须集中处理，否则命中
   测试会和画出来的东西错位。

## 最小可运行验证（第 3 步的可行性闸门）

1. 通过 `MinecraftUIRenderContext` 在 UI pass 里画一个面板：标题、两个标签、两个按钮、
   一段中文。
2. 只有在游戏里满足以下条件才接受：
   * 面板跟随 GUI 缩放设置（改 `视频 -> GUI 缩放` 时它跟着移动和缩放），
   * 面板打开时光标只出现一次、关闭时消失，之后世界立刻可以正常操作，
   * hover 高亮光标下的控件、点击能激活它，点击面板外不会传到世界，
   * 键盘焦点用 Tab 在两个按钮之间移动，Esc 关闭面板，
   * 中文渲染成字形而不是方块，且与原版文本同样大小，
   * 面板打开时没有每帧日志刷屏，也没有可测量的帧时间上升。
3. 一项负向检查：面板打开时禁用模组 —— 没有残留的光标状态，没有残留的 hook。
