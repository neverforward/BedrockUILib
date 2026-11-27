# Input

```cpp
auto field = std::make_unique<bul::Field>();
field->label("Email");
field->description("Shown under the input.");

auto input = std::make_unique<bul::Input>();
input->placeholder("you@example.com");
input->onChange([](std::string const& value) { /* 每次编辑都调 */ });
field->control(std::move(input));
```

或者使用 Spec 树

```cpp
auto tree = bul::spec::build([](bul::spec::Builder& ui) {
    ui.field(
        "Email",
        [&] {
            ui.input("you@example.com", "", [](bul::Input& input) {
                input.onChange([](std::string const& value) { /* 每次编辑都会调用 */ });
            });
        },
        "Shown under the input."
    );
});
```


| 成员 | 含义 |
| --- | --- |
| `value(text)` / `value()` | 文本。代码里设置不触发 `onChange`；用户编辑才触发。 |
| `placeholder(text)` | 值为空时用 `Theme::textMuted` 绘制的占位文字。 |
| `kind(...)` | `Text`、`Password`（每个码点显示一个 `*`）、`Search`、`File`。 |
| `disabled(true)` / `invalid(true)` | 两个状态；field 也能施加。 |
| `required(true)` | 标记为必填（星号由标签画）。 |
| `maxLength(n)` | 按码点计数，不按字节：一个汉字算一个。 |
| `onSubmit(fn)` | 输入框有焦点时按 Enter。 |
| `validator(fn)` | `fn(const std::string&) -> bool`，每次用户编辑后运行。返回 false 期间输入框用 `Theme::invalid` 绘制（`invalid()` 为 true）；第一次通过时清除。初始值和代码设置的值不参与校验。 |
| `kind(File)` + 宿主选择器 | 点击 `Kind::File` 的输入框会向模态宿主请求路径（`wantsFilePicker`）；宿主打开系统文件对话框，拿到路径后调用 `chooseFile(path)`。 |
| `onChoose(fn)` | 文件选好后回调，收到完整路径。 |
| `filePath()` / `filePath(p)` | 选中的路径（完整路径；字段上显示的是文件名）。 |
| `chooseText(text)` / `emptyFileText(text)` | 文件输入框的两个字符串，留作本地化。 |

编辑规则在 `bul/TextEdit` 里，不依赖游戏做单元测试：光标是字节偏移，永远落在码点边界上；Shift 从锚点扩展选区；输入或粘贴替换选区；粘贴内容里的换行被丢掉，因为字段是单行的。

文本事件是**增量流**而不是纯文本：IME 改写组字内容时，每撤回一个字符发一个退格（`0x08`），再发新的组字；`TextEdit::insert` 按增量应用这些编辑，拼音不会堆在已提交的文字前面。

输入框不随内容变宽：有焦点时内容滚动，没焦点时省略号截断，长字符串顶不动窗口和周围的表单。


焦点、光标、剪贴板归宿主管，不归控件：

- 点可聚焦控件即获得焦点（`setFocused`），点别处即失去；
- Tab / Shift+Tab 按视觉顺序在输入框间移动；
- Backspace、Delete、方向键、Home、End、Ctrl+A 以 `KeyStroke` 形式路由到聚焦控件；
- **文本框有焦点时宿主声明持有平台键盘**（`KeyboardManager::tryClaimKeyboardOwnership` + `tryEnableKeyboard`）。不声明的话引擎一个打字事件都不发，这一步让输入框可编辑，也让平台 IME 得以驱动；
- 敲进来的字符走场景自己的文本入口（`UIScene::handleTextChar`），再路由到聚焦控件；
- 焦点离开字段和窗口关闭时都会释放键盘，游戏自带的文本框（聊天）不受影响；
- Ctrl+C / Ctrl+X / Ctrl+V 走系统剪贴板，因为 CJK 输入还依赖引擎的文本服务。

文件输入框另当别论：控件自己不开对话框。它上报 `wantsFilePicker()`，宿主调 `bul::platform::openFileDialog`，选完通过 `chooseFile` 把路径交回来。字段上显示文件名，`filePath()` 留完整路径。
