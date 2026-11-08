#pragma once

#include <string>
#include <string_view>

#include "bul/widget/Widget.h"

namespace bul {

/// A small label that annotates other content, e.g. a "Recommended" marker next
/// to a field label. It is decoration only: not focusable and not interactive.
class BUL_API Badge final : public Widget {
public:
    enum class Variant { Default, Secondary, Outline, Destructive };

    explicit Badge(std::string text, Variant variant = Variant::Default);

    Badge& variant(Variant value) noexcept {
        mVariant = value;
        return *this;
    }

    [[nodiscard]] Variant            variant() const noexcept { return mVariant; }
    [[nodiscard]] std::string const& text() const noexcept { return mText; }

    [[nodiscard]] Size measure(float availableWidth, Theme const&, Canvas const&) const override;
    void               paint(Canvas&, Theme const&) const override;

private:
    std::string mText;
    Variant     mVariant{Variant::Default};
};

} // namespace bul
