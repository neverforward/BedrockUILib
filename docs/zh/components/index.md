# 组件

控件套件在 `src/bul/widget/`，每个组件一对文件。同一棵树因此可以同时跑在游戏里、用录制画布跑的测试里、或者离线预览器里。

```cpp
#include "bul/Widget.h"   // 总头文件：包含下面所有组件

auto root = std::make_unique<bul::Stack>(bul::Stack::Direction::Vertical);
root->gap(8.f);
root->add(std::make_unique<bul::Label>("Settings"));
root->add(std::make_unique<bul::Button>("Apply", [] { /* ... */ }));

bul::showModal(std::move(root)); // bul/Modal.h
```
