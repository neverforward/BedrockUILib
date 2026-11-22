# 滚动

一个比视口更高的内容区域的视口。内容按测量高度布局，按偏移量上移，裁剪到视口内，右侧的滚动条显示当前看的是哪里。

```cpp
auto list = std::make_unique<bul::Stack>(bul::Stack::Direction::Vertical);
for (int i = 1; i <= 20; ++i) {
    list->add(std::make_unique<bul::Label>("Row " + std::to_string(i)));
}

auto view = std::make_unique<bul::ScrollView>();
view->viewport(120.f); // 可见高度
view->content(std::move(list));
view->scrollBy(24.f);  // 代码控制滚动
```

| 成员 | 含义 |
| --- | --- |
| `content(child)` | 唯一的内容控件，通常是 `Stack`。 |
| `viewport(n)` | 可见窗口的高度；`0`（默认）表示"和内容一样高"，也就是不滚动。 |
| `scrollbar(bool)` | 在右边缘画滚动条，默认开启。 |
| `scrollTo(value)` / `scrollBy(delta)` / `offset()` | 代码控制滚动；偏移会被夹到 `content - viewport` 范围内。 |
| `contentHeight()` / `scrollable()` | 测量出的内容高度，以及是否有内容被挡住。 |

显示滚动条时内容会让出这条通道：通道宽 4 单位，内容再留 4 单位间隔，所以文字和控件不会压在滚动条上；不需要滚动时没有通道，内容用满宽度。

滚动条由 `layout::scrollThumb` 求解：长度按可见比例缩短，位置随偏移滑动。内容画在 `Canvas::pushClip` 里，所以边缘的行会被视口裁掉而不是溢出；游戏里的画布把它映射到引擎的裁剪矩形。

滚动条可交互：按住滑块拖动就是滚动，点在轨道上会跳到那个位置并继续拖动；按下会捕获指针，所以滚动条下面的内容不会收到这次拖动。

滚轮已经接到宿主输入：先交给指针下的控件，沿容器向上冒泡，`ScrollView` 每格滚动一个 `wheelStep`（默认 24 单位）。用户也可以在自己的输入处理里调 `scrollTo` / `scrollBy`。
