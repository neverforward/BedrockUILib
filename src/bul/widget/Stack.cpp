#include "bul/Spec.h"

#include "bul/widget/Stack.h"

#include <algorithm>
#include <utility>

#include "bul/Layout.h"
#include "bul/Theme.h"

namespace bul {

Stack::Stack(Direction direction) : mDirection(direction) {}

Stack& Stack::add(std::unique_ptr<Widget> child) { return add(std::move(child), 0.f); }

Stack& Stack::add(std::unique_ptr<Widget> child, float stretch) {
    child->setParent(this);
    mEntries.push_back(Entry{std::move(child), stretch});
    return *this;
}

Size Stack::measureContent(float availableWidth, Theme const& theme, Canvas const& canvas) const {
    Insets const padding = theme.padding.value_or(Insets{});
    float const  gap     = mGap >= 0.f ? mGap : theme.gap;
    float const  inner   = std::max(availableWidth - padding.horizontal(), 0.f);
    float        main    = 0.f;
    float        cross   = 0.f;

    // `display: none` children take no space and do not get a gap either.
    std::vector<Entry const*> shown;
    shown.reserve(mEntries.size());
    for (auto const& entry : mEntries) {
        if (entry.child->displayed(theme)) {
            shown.push_back(&entry);
        }
    }

    for (std::size_t i = 0; i < shown.size(); ++i) {
        Size const size = shown[i]->child->measure(inner, theme, canvas);
        if (mDirection == Direction::Vertical) {
            main  += size.height;
            cross  = std::max(cross, size.width);
        } else {
            main  += size.width;
            cross  = std::max(cross, size.height);
        }
        if (i + 1 < shown.size()) {
            main += gap;
        }
    }
    if (mDirection == Direction::Vertical) {
        return {
            cross + padding.horizontal() + mMargin.horizontal(),
            main + padding.vertical() + mMargin.vertical()
        };
    }
    return {main + padding.horizontal() + mMargin.horizontal(), cross + padding.vertical() + mMargin.vertical()};
}

void Stack::layout(Rect const& outer, Theme const& theme, Canvas const& canvas) {
    Theme const  resolved = resolvedTheme(theme);
    mBounds               = contentRect(outer);
    Insets const padding  = resolved.padding.value_or(Insets{});
    float const  gap      = mGap >= 0.f ? mGap : resolved.gap;
    if (mEntries.empty()) {
        return;
    }

    bool const vertical = mDirection == Direction::Vertical;

    std::vector<Entry const*> shown;
    shown.reserve(mEntries.size());
    for (auto const& entry : mEntries) {
        if (entry.child->displayed(resolved)) {
            shown.push_back(&entry);
        }
    }

    std::vector<layout::Item> items;
    items.reserve(shown.size());
    for (Entry const* entry : shown) {
        items.push_back(layout::Item{
            entry->child->measure(mBounds.width - padding.horizontal(), resolved, canvas),
            entry->stretch
        });
    }

    // A column stretches its children across the width (the usual full-width
    // row), but each child keeps its own height; a row must not stretch the
    // cross axis, or buttons of different sizes would all grow to the tallest.
    layout::BoxSpec const spec{
        .axis    = vertical ? layout::Axis::Vertical : layout::Axis::Horizontal,
        .gap     = gap,
        .padding = padding,
        .align   = vertical ? layout::Align::Stretch : layout::Align::Start,
    };
    std::vector<Rect> const rects = layout::box(mBounds, items, spec);
    for (std::size_t i = 0; i < shown.size(); ++i) {
        shown[i]->child->layout(rects[i], resolved, canvas);
    }
}
bool Stack::wheel(Point point, float delta) {
    if (!mBounds.contains(point)) {
        return false;
    }
    for (auto it = mEntries.rbegin(); it != mEntries.rend(); ++it) {
        if (it->child->bounds().contains(point) && it->child->wheel(point, delta)) {
            return true;
        }
    }
    return false;
}
void Stack::paintContent(Canvas& canvas, Theme const& theme) const {
    // A stack is also a box: a style that sets a surface or a border paints it,
    // the way a div with a background-color does.
    if (theme.background) {
        Widget::paintBox(canvas, mBounds, theme.borderRadius, *theme.background, 1.f, theme.borderWidth.value_or(0.f));
    }
    if (theme.border) {
        Widget::paintBorder(canvas, mBounds, theme.borderRadius, *theme.border, theme.borderWidth.value_or(1.f), 1.f);
    }
    for (auto const& entry : mEntries) {
        entry.child->paint(canvas, theme);
    }
}

Widget* Stack::hitTest(Point point) {
    for (auto it = mEntries.rbegin(); it != mEntries.rend(); ++it) {
        if (Widget* hit = it->child->hitTest(point)) {
            return hit;
        }
    }
    return mBounds.contains(point) ? this : nullptr;
}

void Stack::setControlState(ControlState const& state) {
    for (auto& entry : mEntries) {
        entry.child->setControlState(state);
    }
}

void Stack::collectFocusables(std::vector<Widget*>& out) {
    for (auto& entry : mEntries) {
        entry.child->collectFocusables(out);
    }
}

void Stack::collectOverlays(std::vector<Widget*>& out) {
    for (auto& entry : mEntries) {
        entry.child->collectOverlays(out);
    }
}


Stack& Stack::apply(spec::Stack const& node) {
    if (node.gap >= 0.f) {
        gap(node.gap);
    }
    for (auto const& child : node.children) {
        if (child.node) {
            add(child.node->build(), child.stretch);
        }
    }
    return *this;
}
std::vector<Widget*> Stack::children() const {
    std::vector<Widget*> out;
    out.reserve(mEntries.size());
    for (auto const& entry : mEntries) {
        out.push_back(entry.child.get());
    }
    return out;
}

} // namespace bul
