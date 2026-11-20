#pragma once

#include <cstddef>
#include <string>

#include "bul/widget/Widget.h"

namespace bul {

/// Vertical placement of a text block inside the label's bounds.
enum class VAlign { Top, Center, Bottom };

/// A block of text, in the spirit of Qt's `QLabel`:
///
/// - `size` scales the font around the theme's base size;
/// - `align` / `verticalAlign` place each line and the whole block;
/// - `wrap` breaks the text at the available width (at spaces, and between CJK
///   characters, which have no spaces), honouring explicit newlines;
/// - `lineSpacing` adds room between wrapped lines;
/// - `maxLines` caps the block, and the last line ends with an ellipsis;
/// - `elide` cuts a single line that does not fit with a trailing ellipsis.
///
/// The same layout is computed for measuring and for painting, so the widget and
/// the text agree on where every line starts.
class BUL_API Label final : public Widget {
public:
    explicit Label(std::string text, float scale = 1.f, Align align = Align::Left);

    Label&                           text(std::string value);
    [[nodiscard]] std::string const& text() const noexcept { return mText; }

    /// Font scale relative to the theme's base text size (`QLabel`'s font size).
    Label&              size(float scale);
    [[nodiscard]] float size() const noexcept { return mScale; }

    /// Horizontal alignment of each line inside the bounds.
    Label&              align(Align value) noexcept;
    [[nodiscard]] Align align() const noexcept { return mAlign; }

    /// Vertical placement of the whole block inside the bounds.
    Label&               verticalAlign(VAlign value) noexcept;
    [[nodiscard]] VAlign verticalAlign() const noexcept { return mVAlign; }

    /// Word wrap at the available width. CJK text breaks between characters.
    Label&             wrap(bool value) noexcept;
    [[nodiscard]] bool wrap() const noexcept { return mWrap; }

    /// Extra space between lines, in units at theme scale 1 (default 0).
    Label&              lineSpacing(float value) noexcept;
    [[nodiscard]] float lineSpacing() const noexcept { return mLineSpacing; }

    /// Cap on the room the text may use: wraps at this width even when the parent
    /// offers more (0 means "whatever the parent offers"). This is how a
    /// paragraph keeps a readable column width.
    Label&              maxWidth(float value) noexcept;
    [[nodiscard]] float maxWidth() const noexcept { return mMaxWidth; }

    /// Maximum number of lines; 0 means no limit. The last line is elided.
    Label&                    maxLines(std::size_t value) noexcept;
    [[nodiscard]] std::size_t maxLines() const noexcept { return mMaxLines; }

    /// Cut a line that does not fit with a trailing ellipsis (default on).
    Label&             elide(bool value) noexcept;
    [[nodiscard]] bool elide() const noexcept { return mElide; }

    /// Explicit text colour; when set it wins over `muted`.
    Label& color(Rgba value) noexcept {
        mColor    = value;
        mHasColor = true;
        return *this;
    }
    [[nodiscard]] Rgba color() const noexcept { return mColor; }
    [[nodiscard]] bool hasColor() const noexcept { return mHasColor; }

    Label& muted(bool value = true) noexcept {
        mMuted = value;
        return *this;
    }

    [[nodiscard]] Size measure(float availableWidth, Theme const&, Canvas const&) const override;
    void               paint(Canvas&, Theme const&) const override;

private:
    std::string mText;
    std::size_t mMaxLines{};
    float       mScale{1.f};
    float       mLineSpacing{};
    float       mMaxWidth{};
    Align       mAlign{Align::Left};
    VAlign      mVAlign{VAlign::Top};
    bool        mWrap{};
    bool        mElide{true};
    bool        mMuted{};
    bool        mHasColor{};
    Rgba        mColor{};
};

} // namespace bul
