#include "bul/widget/Field.h"

#include <algorithm>
#include <utility>

#include "bul/Theme.h"
#include "bul/widget/FieldDescription.h"
#include "bul/widget/FieldLabel.h"

namespace bul {

namespace {

constexpr float kFieldGap = 3.f;

} // namespace

Field::Field() = default;

Field& Field::orientation(Orientation value) noexcept {
    mOrientation = value;
    return *this;
}

Field& Field::gap(float value) noexcept {
    mGap = value;
    return *this;
}

Field& Field::label(std::unique_ptr<Widget> widget) {
    adopt(mLabel, std::move(widget));
    return *this;
}

Field& Field::label(std::string text) { return label(std::make_unique<FieldLabel>(std::move(text))); }

Field& Field::control(std::unique_ptr<Widget> widget) {
    adopt(mControl, std::move(widget));
    return *this;
}

Field& Field::description(std::unique_ptr<Widget> widget) {
    adopt(mDescription, std::move(widget));
    return *this;
}

Field& Field::description(std::string text) { return description(std::make_unique<FieldDescription>(std::move(text))); }

Field& Field::disabled(bool value) {
    mLocal.disabled = value;
    propagate();
    return *this;
}

Field& Field::invalid(bool value) {
    mLocal.invalid = value;
    propagate();
    return *this;
}

Field& Field::required(bool value) {
    mLocal.required = value;
    propagate();
    return *this;
}

void Field::setControlState(ControlState const& state) {
    mInherited = state;
    propagate();
}

void Field::adopt(std::unique_ptr<Widget>& slot, std::unique_ptr<Widget> widget) {
    slot = std::move(widget);
    if (slot) {
        slot->setControlState(mEffective);
    }
}

void Field::propagate() {
    ControlState const effective = mLocal.merged(mInherited);
    if (effective == mEffective) {
        return;
    }
    mEffective = effective;
    for (auto* child : {mLabel.get(), mControl.get(), mDescription.get()}) {
        if (child) {
            child->setControlState(mEffective);
        }
    }
}

float Field::gapOf(Theme const&) const noexcept { return mGap >= 0.f ? mGap : kFieldGap; }

Size Field::measure(float availableWidth, Theme const& theme, Canvas const& canvas) const {
    float const gap = gapOf(theme);

    if (mOrientation == Orientation::Horizontal) {
        Size  labelSize{};
        Size  descriptionSize{};
        Size  controlSize{};
        float leftWidth  = 0.f;
        float leftHeight = 0.f;

        if (mLabel) {
            labelSize  = mLabel->measure(availableWidth, theme, canvas);
            leftWidth  = labelSize.width;
            leftHeight = labelSize.height;
        }
        if (mDescription) {
            descriptionSize  = mDescription->measure(availableWidth, theme, canvas);
            leftWidth        = std::max(leftWidth, descriptionSize.width);
            leftHeight      += (mLabel ? gap : 0.f) + descriptionSize.height;
        }

        float const reserve = leftWidth > 0.f ? leftWidth + gap : 0.f;
        if (mControl) {
            float const controlWidth = availableWidth > 0.f ? std::max(availableWidth - reserve, 0.f) : 0.f;
            controlSize              = mControl->measure(controlWidth, theme, canvas);
        }
        return {
            reserve + controlSize.width + mMargin.horizontal(),
            std::max(leftHeight, controlSize.height) + mMargin.vertical()
        };
    }

    float      width   = 0.f;
    float      height  = 0.f;
    int        count   = 0;
    auto const include = [&](Widget const* widget) {
        if (!widget) {
            return;
        }
        Size const size  = widget->measure(availableWidth, theme, canvas);
        width            = std::max(width, size.width);
        height          += size.height;
        ++count;
    };
    include(mLabel.get());
    include(mControl.get());
    include(mDescription.get());
    if (count > 1) {
        height += gap * static_cast<float>(count - 1);
    }
    return {width + mMargin.horizontal(), height + mMargin.vertical()};
}

void Field::layout(Rect const& outer, Theme const& theme, Canvas const& canvas) {
    mBounds         = contentRect(outer);
    float const gap = gapOf(theme);

    if (mOrientation == Orientation::Vertical) {
        float y = mBounds.y;
        for (auto* child : {mLabel.get(), mControl.get(), mDescription.get()}) {
            if (!child) {
                continue;
            }
            Size const size = child->measure(mBounds.width, theme, canvas);
            child->layout({mBounds.x, y, mBounds.width, size.height}, theme, canvas);
            y += size.height + gap;
        }
        return;
    }

    Size  labelSize{};
    Size  descriptionSize{};
    Size  controlSize{};
    float leftWidth = 0.f;
    if (mLabel) {
        labelSize = mLabel->measure(mBounds.width, theme, canvas);
        leftWidth = labelSize.width;
    }
    if (mDescription) {
        descriptionSize = mDescription->measure(mBounds.width, theme, canvas);
        leftWidth       = std::max(leftWidth, descriptionSize.width);
    }

    float const reserve      = leftWidth > 0.f ? leftWidth + gap : 0.f;
    float const controlX     = mBounds.x + reserve;
    float const controlWidth = std::max(mBounds.right() - controlX, 0.f);
    if (mControl) {
        controlSize = mControl->measure(controlWidth, theme, canvas);
    }

    float const centerY    = mBounds.y + mBounds.height / 2.f;
    float const leftHeight = labelSize.height + (mLabel && mDescription ? gap : 0.f) + descriptionSize.height;
    float       leftY      = centerY - leftHeight / 2.f;

    if (mLabel) {
        mLabel->layout({mBounds.x, leftY, leftWidth, labelSize.height}, theme, canvas);
        leftY += labelSize.height + gap;
    }
    if (mDescription) {
        mDescription->layout({mBounds.x, leftY, leftWidth, descriptionSize.height}, theme, canvas);
    }
    if (mControl) {
        mControl
            ->layout({controlX, centerY - controlSize.height / 2.f, controlWidth, controlSize.height}, theme, canvas);
    }
}

void Field::paint(Canvas& canvas, Theme const& theme) const {
    if (mLabel) {
        mLabel->paint(canvas, theme);
    }
    if (mControl) {
        mControl->paint(canvas, theme);
    }
    if (mDescription) {
        mDescription->paint(canvas, theme);
    }
}

Widget* Field::hitTest(Point point) {
    for (auto* child : {mDescription.get(), mControl.get(), mLabel.get()}) {
        if (child) {
            if (Widget* hit = child->hitTest(point)) {
                return hit;
            }
        }
    }
    return mBounds.contains(point) ? this : nullptr;
}

void Field::collectFocusables(std::vector<Widget*>& out) {
    for (auto* child : {mLabel.get(), mControl.get(), mDescription.get()}) {
        if (child) {
            child->collectFocusables(out);
        }
    }
}

void Field::collectOverlays(std::vector<Widget*>& out) {
    for (auto* child : {mLabel.get(), mControl.get(), mDescription.get()}) {
        if (child) {
            child->collectOverlays(out);
        }
    }
}

} // namespace bul
