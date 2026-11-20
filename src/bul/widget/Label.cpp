#include "bul/widget/Label.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "bul/Theme.h"
#include "bul/widget/TextFlow.h"

namespace bul {

using namespace detail;

namespace {

constexpr std::size_t kNone = static_cast<std::size_t>(-1);

[[nodiscard]] bool isBreakSpace(char32_t codePoint) noexcept { return codePoint == U' ' || codePoint == U'\t'; }

/// The text split on explicit newlines; wrapping happens inside each paragraph.
[[nodiscard]] std::vector<std::string_view> splitParagraphs(std::string const& text) {
    std::vector<std::string_view> paragraphs;
    std::string_view const        view{text};
    std::size_t                   start = 0;
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\n') {
            paragraphs.push_back(view.substr(start, i - start));
            start = i + 1;
        }
    }
    paragraphs.push_back(view.substr(start));
    return paragraphs;
}

/// Greedy wrap of one paragraph: break after a space, or after a wide (CJK)
/// character, because those scripts do not separate words with spaces.
[[nodiscard]] std::vector<std::string>
wrapParagraph(Canvas const& canvas, float size, std::string_view paragraph, float maxWidth) {
    std::vector<std::string> lines;
    GlyphFrame const         frame = buildFrame(canvas, size, paragraph);
    std::size_t const        count = frame.offsets.size() - 1;

    std::size_t start = 0;
    while (start < count) {
        float const base        = frame.prefix[start];
        std::size_t end         = start;
        std::size_t opportunity = kNone;
        while (end < count) {
            float const width = frame.prefix[end + 1] - base;
            if (width > maxWidth) {
                if (end > start) {
                    break;
                }
                ++end; // one glyph wider than the line: let it overflow
                break;
            }
            char32_t const codePoint = codePointAt(paragraph, frame.offsets[end]);
            if (isBreakSpace(codePoint) || isWide(codePoint)) {
                opportunity = end + 1;
            }
            ++end;
        }

        std::size_t cut = end;
        if (end < count && opportunity != kNone && opportunity > start) {
            cut = opportunity;
        }
        std::size_t trimmed = cut;
        while (trimmed > start && isBreakSpace(codePointAt(paragraph, frame.offsets[trimmed - 1]))) {
            --trimmed;
        }
        lines.push_back(std::string{slice(paragraph, frame.offsets[start], frame.offsets[trimmed])});

        start = cut;
        while (start < count && isBreakSpace(codePointAt(paragraph, frame.offsets[start]))) {
            ++start;
        }
    }
    if (lines.empty()) {
        lines.emplace_back();
    }
    return lines;
}

/// Longest code point prefix of `text` that fits in `maxWidth`, without any
/// ellipsis of its own.
[[nodiscard]] std::string fitPrefix(Canvas const& canvas, float size, std::string const& text, float maxWidth) {
    if (maxWidth <= 0.f || text.empty()) {
        return {};
    }
    GlyphFrame const frame = buildFrame(canvas, size, text);
    std::size_t      best  = 0;
    for (std::size_t i = 0; i + 1 < frame.offsets.size(); ++i) {
        if (frame.prefix[i + 1] <= maxWidth) {
            best = i + 1;
        } else {
            break;
        }
    }
    return std::string{slice(text, 0, frame.offsets[best])};
}

/// The room the text may use: the parent's offer, capped by the label's own
/// maxWidth when it has one. Zero means "no limit".
[[nodiscard]] float limitFor(float availableWidth, float cap) noexcept {
    float const offered = availableWidth > 0.f ? availableWidth : 0.f;
    if (cap <= 0.f) {
        return offered;
    }
    return offered > 0.f ? std::min(offered, cap) : cap;
}

struct Line {
    std::string text;
    float       width{};
};

struct TextLayout {
    std::vector<Line> lines;
    float             width{};
    float             height{};
};

[[nodiscard]] TextLayout buildLayout(
    Canvas const&      canvas,
    float              size,
    std::string const& text,
    float              maxWidth,
    bool               wrap,
    std::size_t        maxLines,
    bool               elide,
    float              spacing
) {
    TextLayout               layout;
    std::vector<std::string> lines;

    for (std::string_view const paragraph : splitParagraphs(text)) {
        bool const tooWide = maxWidth > 0.f && frameWidth(buildFrame(canvas, size, paragraph)) > maxWidth;
        if (wrap && tooWide) {
            std::vector<std::string> wrapped = wrapParagraph(canvas, size, paragraph, maxWidth);
            lines.insert(lines.end(), wrapped.begin(), wrapped.end());
        } else {
            lines.emplace_back(paragraph);
        }
    }

    if (elide && maxWidth > 0.f) {
        for (auto& line : lines) {
            line = fitText(line, maxWidth, size, canvas);
        }
    }

    if (maxLines > 0 && lines.size() > maxLines) {
        lines.resize(maxLines);
        if (elide && maxWidth > 0.f) {
            float const ellipsis = canvas.textWidth("...", size);
            lines.back()         = fitPrefix(canvas, size, lines.back(), std::max(maxWidth - ellipsis, 0.f)) + "...";
        }
    }

    float const lineHeight = canvas.lineHeight(size);
    float       widest     = 0.f;
    layout.lines.reserve(lines.size());
    for (auto& line : lines) {
        float const width = frameWidth(buildFrame(canvas, size, line));
        widest            = std::max(widest, width);
        layout.lines.push_back(Line{std::move(line), width});
    }
    layout.width  = wrap && maxWidth > 0.f ? maxWidth : widest;
    layout.height = static_cast<float>(layout.lines.size()) * lineHeight
                  + (layout.lines.size() > 1 ? spacing * static_cast<float>(layout.lines.size() - 1) : 0.f);
    return layout;
}

} // namespace

Label::Label(std::string text, float scale, Align align) : mText(std::move(text)), mScale(scale), mAlign(align) {}

Label& Label::text(std::string value) {
    mText = std::move(value);
    return *this;
}

Label& Label::size(float scale) {
    mScale = scale > 0.f ? scale : 1.f;
    return *this;
}

Label& Label::align(Align value) noexcept {
    mAlign = value;
    return *this;
}

Label& Label::verticalAlign(VAlign value) noexcept {
    mVAlign = value;
    return *this;
}

Label& Label::wrap(bool value) noexcept {
    mWrap = value;
    return *this;
}

Label& Label::lineSpacing(float value) noexcept {
    mLineSpacing = value;
    return *this;
}

Label& Label::maxWidth(float value) noexcept {
    mMaxWidth = value;
    return *this;
}

Label& Label::maxLines(std::size_t value) noexcept {
    mMaxLines = value;
    return *this;
}

Label& Label::elide(bool value) noexcept {
    mElide = value;
    return *this;
}

Size Label::measure(float availableWidth, Theme const& theme, Canvas const& canvas) const {
    float const      size   = theme.fontScale * mScale;
    TextLayout const layout = buildLayout(
        canvas,
        size,
        mText,
        limitFor(availableWidth, mMaxWidth),
        mWrap,
        mMaxLines,
        mElide,
        mLineSpacing * theme.fontScale
    );
    return {layout.width + mMargin.horizontal(), layout.height + mMargin.vertical()};
}

void Label::paint(Canvas& canvas, Theme const& theme) const {
    float const      size    = theme.fontScale * mScale;
    float const      spacing = mLineSpacing * theme.fontScale;
    TextLayout const layout =
        buildLayout(canvas, size, mText, limitFor(mBounds.width, mMaxWidth), mWrap, mMaxLines, mElide, spacing);

    float const lineHeight = canvas.lineHeight(size);
    float       y          = mBounds.y;
    if (mVAlign == VAlign::Center) {
        y += (mBounds.height - layout.height) / 2.f;
    } else if (mVAlign == VAlign::Bottom) {
        y = mBounds.bottom() - layout.height;
    }

    Rgba color = mMuted ? theme.textMuted : theme.text;
    if (mHasColor) {
        color = mColor;
    }
    for (Line const& line : layout.lines) {
        float x = mBounds.x;
        if (mAlign == Align::Center) {
            x += (mBounds.width - line.width) / 2.f;
        } else if (mAlign == Align::Right) {
            x = mBounds.right() - line.width;
        }
        if (!line.text.empty()) {
            GlyphFrame const frame = buildFrame(canvas, size, line.text);
            drawFragment(
                canvas,
                line.text,
                frame.offsets,
                frame.prefix,
                0,
                frame.offsets.size() - 1,
                {x, y},
                size,
                color
            );
        }
        y += lineHeight + spacing;
    }
}

} // namespace bul
