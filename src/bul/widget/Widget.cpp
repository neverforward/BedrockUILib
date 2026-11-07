#include "bul/widget/Widget.h"

namespace bul {

void Widget::layout(Rect const& outer, Theme const&, Canvas const&) { mBounds = contentRect(outer); }

Widget* Widget::hitTest(Point point) { return mBounds.contains(point) ? this : nullptr; }

} // namespace bul
