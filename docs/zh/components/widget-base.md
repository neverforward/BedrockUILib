# 控件基类

所有组件都继承 `Widget`，下面的属性到处都有。

| 成员 | 含义 |
| --- | --- |
| `margin(all)` / `margin(h, v)` | 父控件测量和布局时给控件外侧留的空隙。 |
| `size()` / `width()` / `height()` | 布局后的尺寸，GUI 单位。 |
| `fixedSize(w, h)` / `fixedWidth(w)` / `fixedHeight(h)` | 覆盖测量尺寸；即使父控件给的更多，控件边界也钳制在这个值内。 |
| `minSize(w, h)` / `maxSize(w, h)` | 约束测量出的内容尺寸（0 = 不约束）。 |
| `onClick(fn)` | 让任何控件可点击：`Label` 挂上点击回调也能交互，虽然它本身没有动作。 |
| `onFocus(fn)` / `onBlur(fn)` | 宿主给键盘焦点或拿走焦点时触发。 |
| `onHover(fn)` | 指针进入或离开控件时调用 `fn(bool entered)`。 |
| `notifyClick` / `notifyFocus` / `notifyHover` | 宿主侧入口：先跑控件自己的反应，再跑回调。 |

组件覆盖 `measureContent(...)`；`measure(...)` 套尺寸约束和 margin，所以所有控件走同一条尺寸路径。
