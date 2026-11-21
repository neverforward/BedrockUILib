# Components

The widget kit lives in `src/bul/widget/`, one file pair per component.
That is what lets the same tree run in the game, in a test with a recording
canvas, or in an offline previewer.

```cpp
#include "bul/Widget.h"   // umbrella header: every component below

auto root = std::make_unique<bul::Stack>(bul::Stack::Direction::Vertical);
root->gap(8.f);
root->add(std::make_unique<bul::Label>("Settings"));
root->add(std::make_unique<bul::Button>("Apply", [] { /* ... */ }));

bul::showModal(std::move(root)); // bul/Modal.h
```
