#include "bul/widget/Input.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

#include "bul/TextMetrics.h"
#include "bul/Theme.h"

namespace bul {

namespace {

constexpr float kBorder         = 1.f;
constexpr float kPadX           = 6.f;
constexpr float kPadY           = 4.f;
constexpr float kMinWidth       = 120.f;
constexpr float kFilePadX       = 6.f;
constexpr float kFileGap        = 6.f;
constexpr float kSelectionAlpha = 0.32f;

[[nodiscard]] std::string_view slice(std::string const& text, std::size_t begin, std::size_t end) {
    return std::string_view{text}.substr(begin, end - begin);
}

/// The part of a path after the last separator: what a file field displays.
[[nodiscard]] std::string fileNameOf(std::string const& path) {
    std::size_t const separator = path.find_last_of("\\/");
    return separator == std::string::npos ? path : path.substr(separator + 1);
}

/// Decodes the UTF-8 code point that starts at `offset`.
[[nodiscard]] char32_t codePointAt(std::string_view text, std::size_t offset) noexcept {
    auto const lead = static_cast<unsigned char>(text[offset]);
    if (lead < 0x80u) {
        return lead;
    }
    std::size_t const left = text.size() - offset;
    if ((lead & 0xE0u) == 0xC0u && left >= 2) {
        return static_cast<char32_t>(((lead & 0x1Fu) << 6) | (static_cast<unsigned char>(text[offset + 1]) & 0x3Fu));
    }
    if ((lead & 0xF0u) == 0xE0u && left >= 3) {
        return static_cast<char32_t>(
            ((lead & 0x0Fu) << 12) | ((static_cast<unsigned char>(text[offset + 1]) & 0x3Fu) << 6)
            | (static_cast<unsigned char>(text[offset + 2]) & 0x3Fu)
        );
    }
    if ((lead & 0xF8u) == 0xF0u && left >= 4) {
        return static_cast<char32_t>(
            ((lead & 0x07u) << 18) | ((static_cast<unsigned char>(text[offset + 1]) & 0x3Fu) << 12)
            | ((static_cast<unsigned char>(text[offset + 2]) & 0x3Fu) << 6)
            | (static_cast<unsigned char>(text[offset + 3]) & 0x3Fu)
        );
    }
    return lead;
}

/// Code points drawn from the "wide" fallback font (CJK, Hangul, fullwidth
/// forms). The engine measures them narrower than the glyphs it draws, so they
/// need their own advance.
[[nodiscard]] bool isWide(char32_t codePoint) noexcept {
    if (codePoint < 0x1100) {
        return false;
    }
    return (codePoint <= 0x115F)                        // Hangul Jamo
        || (codePoint >= 0x2E80 && codePoint <= 0x303E) // CJK radicals, punct.
        || (codePoint >= 0x3041 && codePoint <= 0x33FF) // kana, CJK compat
        || (codePoint >= 0x3400 && codePoint <= 0x4DBF) // CJK extension A
        || (codePoint >= 0x4E00 && codePoint <= 0x9FFF) // CJK unified
        || (codePoint >= 0xA000 && codePoint <= 0xA4CF) // Yi
        || (codePoint >= 0xAC00 && codePoint <= 0xD7A3) // Hangul syllables
        || (codePoint >= 0xF900 && codePoint <= 0xFAFF) // CJK compat ideographs
        || (codePoint >= 0xFE30 && codePoint <= 0xFE4F) // CJK compat forms
        || (codePoint >= 0xFF00 && codePoint <= 0xFF60) // fullwidth forms
        || (codePoint >= 0xFFE0 && codePoint <= 0xFFE6) || (codePoint >= 0x1F300 && codePoint <= 0x1FAFF) // emoji
        || (codePoint >= 0x20000 && codePoint <= 0x3FFFD); // CJK extension B+
}

/// Distance from one code point to the next in this field's own layout.
///
/// A wide code point advances at least one line height. The engine reports a
/// narrower number for them, while the unicode fallback it actually draws with
/// is a square glyph, so trusting the measurement makes the text wider than the
/// box and leaves the caret inside the text instead of at its end.
[[nodiscard]] float advanceOf(Canvas const& canvas, float scale, std::string_view codePoint) noexcept {
    float const measured = canvas.textWidth(codePoint, scale);
    if (!isWide(codePointAt(codePoint, 0))) {
        return measured;
    }
    return std::max(measured, std::max(canvas.lineHeight(scale) - 1.f, 0.f));
}

/// Per-code-point positions of one display string. Every drawing and caret
/// decision of the field goes through this table, so they cannot disagree with
/// each other even when the engine's own measurement is off for some glyphs.
struct GlyphFrame {
    std::vector<std::size_t> offsets; // byte offset of each code point; last = size
    std::vector<float>       prefix;  // width before that code point
};

[[nodiscard]] GlyphFrame buildFrame(Canvas const& canvas, float scale, std::string_view text) {
    GlyphFrame  frame;
    std::size_t offset = 0;
    frame.offsets.push_back(0);
    frame.prefix.push_back(0.f);
    while (offset < text.size()) {
        std::size_t const next = detail::nextCodePoint(text, offset);
        frame.prefix.push_back(frame.prefix.back() + advanceOf(canvas, scale, text.substr(offset, next - offset)));
        frame.offsets.push_back(next);
        offset = next;
    }
    return frame;
}

[[nodiscard]] float frameWidth(GlyphFrame const& frame) noexcept { return frame.prefix.back(); }

/// Longest prefix of `text` that fits in `maxWidth`, with a trailing ellipsis
/// when the whole text does not fit. Uses the same advance table as the caret,
/// so a clipped line really ends where the ellipsis says it does.
[[nodiscard]] std::string fitText(std::string_view text, float maxWidth, float scale, Canvas const& canvas) {
    if (maxWidth <= 0.f) {
        return {};
    }
    GlyphFrame const frame    = buildFrame(canvas, scale, text);
    float const      ellipsis = canvas.textWidth("...", scale);
    if (frameWidth(frame) <= maxWidth) {
        return std::string{text};
    }
    if (ellipsis > maxWidth) {
        return {};
    }
    std::string out;
    for (std::size_t index = 0; index + 1 < frame.offsets.size(); ++index) {
        if (frame.prefix[index + 1] + ellipsis > maxWidth) {
            break;
        }
        out.append(text.substr(frame.offsets[index], frame.offsets[index + 1] - frame.offsets[index]));
    }
    out.append("...");
    return out;
}

/// Draws code points [begin, end) of `text` at `prefix` positions starting at
/// `origin`. Runs of narrow glyphs are drawn as one string (the engine lays them
/// out exactly as measured); a wide glyph is placed on its own, because only its
/// cell position is known.
void drawFragment(
    Canvas&                         canvas,
    std::string const&              text,
    std::vector<std::size_t> const& offsets,
    std::vector<float> const&       prefix,
    std::size_t                     begin,
    std::size_t                     end,
    Point                           origin,
    float                           scale,
    Rgba                            color
) {
    std::size_t index = begin;
    while (index < end) {
        std::size_t runEnd = index;
        if (isWide(codePointAt(text, offsets[index]))) {
            runEnd = index + 1;
        } else {
            while (runEnd < end && !isWide(codePointAt(text, offsets[runEnd]))) {
                ++runEnd;
            }
        }
        float const x     = origin.x + prefix[index];
        float const width = prefix[runEnd] - prefix[index];
        canvas
            .text(slice(text, offsets[index], offsets[runEnd]), {x, origin.y}, width + 2.f, color, scale, Align::Left);
        index = runEnd;
    }
}

/// Draws `text` cut to `maxWidth` (ellipsis included) with the advance table.
void drawFittedText(Canvas& canvas, std::string_view text, Point origin, float maxWidth, float scale, Rgba color) {
    std::string const shown = fitText(text, maxWidth, scale, canvas);
    if (shown.empty()) {
        return;
    }
    GlyphFrame const frame = buildFrame(canvas, scale, shown);
    drawFragment(canvas, shown, frame.offsets, frame.prefix, 0, frame.offsets.size() - 1, origin, scale, color);
}

} // namespace

Input::Input() = default;

Input::Input(std::string value) { mEdit.setText(std::move(value)); }

Input& Input::value(std::string text) {
    mFilePath.clear();
    mEdit.setText(std::move(text));
    return *this;
}

Input& Input::placeholder(std::string text) {
    mPlaceholder = std::move(text);
    return *this;
}

Input& Input::kind(Kind value) noexcept {
    mKind = value;
    return *this;
}

Input& Input::disabled(bool value) {
    mLocalDisabled = value;
    return *this;
}

Input& Input::invalid(bool value) {
    mLocalInvalid = value;
    return *this;
}

Input& Input::required(bool value) noexcept {
    mLocalRequired = value;
    return *this;
}

Input& Input::maxLength(std::size_t codePoints) noexcept {
    mEdit.setMaxLength(codePoints);
    return *this;
}

Input& Input::onChange(std::function<void(std::string const&)> callback) {
    mOnChange = std::move(callback);
    return *this;
}

Input& Input::onSubmit(std::function<void(std::string const&)> callback) {
    mOnSubmit = std::move(callback);
    return *this;
}

Input& Input::validator(std::function<bool(std::string const&)> check) {
    mValidator        = std::move(check);
    mValidationFailed = false; // the next edit decides; programmatic values are not judged
    return *this;
}

void Input::runValidation() {
    if (mValidator) {
        mValidationFailed = !mValidator(mEdit.text());
    }
}

Input& Input::onChoose(std::function<void(std::string const&)> callback) {
    mOnChoose = std::move(callback);
    return *this;
}

Input& Input::filePath(std::string path) {
    mFilePath = std::move(path);
    mEdit.setText(fileNameOf(mFilePath));
    return *this;
}

void Input::chooseFile(std::string_view path) {
    if (path.empty()) {
        return;
    }
    std::string const before = mEdit.text();
    mFilePath.assign(path);
    mEdit.setText(fileNameOf(mFilePath));
    notifyChanged(before);
    if (mOnChoose) {
        mOnChoose(mFilePath);
    }
}

Input& Input::chooseText(std::string text) {
    mChooseText = std::move(text);
    return *this;
}

Input& Input::emptyFileText(std::string text) {
    mEmptyFileText = std::move(text);
    return *this;
}

Input::Metrics Input::metrics(Theme const& theme) const noexcept { return {kPadX, kPadY, theme.fontScale}; }

Rect Input::textArea(Metrics const& metrics) const noexcept {
    return {
        mBounds.x + kBorder + metrics.padX,
        mBounds.y + kBorder + metrics.padY,
        std::max(mBounds.width - 2.f * kBorder - 2.f * metrics.padX, 0.f),
        std::max(mBounds.height - 2.f * kBorder - 2.f * metrics.padY, 0.f)
    };
}

Size Input::measure(float availableWidth, Theme const& theme, Canvas const& canvas) const {
    Metrics const metrics = this->metrics(theme);
    float const   height  = canvas.lineHeight(metrics.scale) + 2.f * metrics.padY + 2.f * kBorder;

    float contentWidth = 0.f;
    if (mKind == Kind::File) {
        float const      segment = canvas.textWidth(mChooseText, metrics.scale) + 2.f * kFilePadX;
        std::string_view shown   = mEdit.empty() ? std::string_view{mEmptyFileText} : std::string_view{mEdit.text()};
        contentWidth             = segment + kFileGap + frameWidth(buildFrame(canvas, metrics.scale, shown));
    } else {
        // Only the placeholder shapes the desired width: the value is clipped and
        // scrolled inside the box, so letting it grow the measurement would make
        // the whole window (and every row above it) jump while the user types.
        contentWidth = frameWidth(buildFrame(canvas, metrics.scale, mPlaceholder)) + 4.f;
    }

    float width = std::max(contentWidth + 2.f * metrics.padX + 2.f * kBorder, kMinWidth);
    if (availableWidth > 0.f) {
        width = std::min(width, availableWidth);
    }
    return {width + mMargin.horizontal(), height + mMargin.vertical()};
}

void Input::layout(Rect const& outer, Theme const& theme, Canvas const& canvas) {
    Widget::layout(outer, theme, canvas);
    if (!mPendingClick) {
        return;
    }
    mPendingClick = false;
    if (mKind == Kind::File || disabled()) {
        return;
    }
    Metrics const    metrics = this->metrics(theme);
    Rect const       area    = textArea(metrics);
    TextLayout const layout  = textLayout(canvas, theme, area);
    mEdit.setCaret(caretAt(mPendingClickX, layout, area));
}

Input::TextLayout Input::textLayout(Canvas const& canvas, Theme const& theme, Rect const& area) const {
    Metrics const metrics = this->metrics(theme);
    TextLayout    out;
    out.display = mKind == Kind::Password ? mEdit.masked() : mEdit.text();

    // All positions come from one per-code-point table: the engine under-reports
    // wide (unicode) glyphs, so a plain string measurement would put the caret
    // and the window left of the characters actually drawn.
    GlyphFrame frame = buildFrame(canvas, metrics.scale, out.display);
    out.offsets      = std::move(frame.offsets);
    out.prefix       = std::move(frame.prefix);

    std::size_t const count = out.offsets.size() - 1;
    out.caret               = std::min(mEdit.codePointIndexOf(mEdit.caret()), count);
    out.selectionBegin      = std::min(mEdit.codePointIndexOf(mEdit.selectionBegin()), count);
    out.selectionEnd        = std::min(mEdit.codePointIndexOf(mEdit.selectionEnd()), count);

    // One pixel stays free so the caret at the end of the text stays visible.
    float const room   = std::max(area.width - 1.f, 0.f);
    float const caretX = out.prefix[out.caret];
    if (mFocused && caretX > room) {
        out.scroll = caretX - room;
    }
    return out;
}

std::size_t Input::caretAt(float pointerX, TextLayout const& layout, Rect const& area) const {
    float const local = pointerX - area.x + layout.scroll;

    std::size_t       best         = 0;
    float             bestDistance = std::abs(local);
    std::size_t const count        = layout.offsets.size() - 1;
    for (std::size_t index = 0; index < count; ++index) {
        float const x        = layout.prefix[index + 1];
        float const distance = std::abs(x - local);
        if (distance < bestDistance) {
            bestDistance = distance;
            best         = index + 1;
        }
        if (x >= local) {
            break;
        }
    }
    return mEdit.offsetOfCodePoint(best);
}

void Input::paint(Canvas& canvas, Theme const& theme) const {
    Metrics const metrics = this->metrics(theme);
    float const   fade    = disabled() ? 0.5f : 1.f;

    Rgba border{};
    if (invalid()) {
        border = theme.invalid;
    } else if (mFocused) {
        border = theme.accent;
    } else {
        border = theme.inputBorder;
    }

    canvas.fill(mBounds, (mHovered && !disabled()) ? theme.inputHover : theme.input, fade);
    if (mKind == Kind::File) {
        paintFile(canvas, theme, metrics, fade);
    } else {
        paintText(canvas, theme, metrics, fade);
    }
    canvas.outline(mBounds, border.withAlpha(fade), kBorder);
}

void Input::paintText(Canvas& canvas, Theme const& theme, Metrics const& metrics, float fade) const {
    Rect const       area       = textArea(metrics);
    float const      lineHeight = canvas.lineHeight(metrics.scale);
    float const      textY      = area.y + (area.height - lineHeight) / 2.f;
    TextLayout const layout     = textLayout(canvas, theme, area);

    if (layout.display.empty()) {
        if (!mPlaceholder.empty()) {
            drawFittedText(
                canvas,
                mPlaceholder,
                {area.x, textY},
                area.width,
                metrics.scale,
                theme.textMuted.withAlpha(fade)
            );
        }
    } else if (!mFocused) {
        // Not editing: the value is a preview, so an overlong one is cut with an
        // ellipsis instead of silently hiding where the text continues.
        drawFittedText(canvas, layout.display, {area.x, textY}, area.width, metrics.scale, theme.text.withAlpha(fade));
    } else {
        if (layout.selectionBegin != layout.selectionEnd) {
            float const beginX = layout.prefix[layout.selectionBegin] - layout.scroll;
            float const endX   = layout.prefix[layout.selectionEnd] - layout.scroll;
            float const left   = std::clamp(area.x + beginX, area.x, area.right());
            float const right  = std::clamp(area.x + endX, area.x, area.right());
            if (right > left) {
                canvas.fill({left, textY, right - left, lineHeight}, theme.accent, kSelectionAlpha);
            }
        }

        // Only the code points that fully fit inside the box are drawn: the
        // canvas has no clip rectangle, so the cut is made here, on the same
        // table that drives the caret.
        float const room  = std::max(area.width - 1.f, 0.f);
        std::size_t start = 0;
        while (start + 1 < layout.offsets.size() && layout.prefix[start] < layout.scroll) {
            ++start;
        }
        std::size_t end = start;
        while (end + 1 < layout.offsets.size() && layout.prefix[end + 1] - layout.scroll <= room) {
            ++end;
        }
        drawFragment(
            canvas,
            layout.display,
            layout.offsets,
            layout.prefix,
            start,
            end,
            {area.x - layout.scroll, textY},
            metrics.scale,
            theme.text.withAlpha(fade)
        );
    }

    if (mFocused && !disabled()) {
        float const caretX     = layout.prefix[std::min(layout.caret, layout.prefix.size() - 1)] - layout.scroll;
        float const caretRight = std::max(area.right() - 1.f, area.x);
        canvas.fill({std::clamp(area.x + caretX, area.x, caretRight), textY, 1.f, lineHeight}, theme.text, fade);
    }
}

void Input::paintFile(Canvas& canvas, Theme const& theme, Metrics const& metrics, float fade) const {
    Rect const  box        = mBounds.inset(kBorder);
    float const lineHeight = canvas.lineHeight(metrics.scale);

    float const segmentWidth =
        std::min(canvas.textWidth(mChooseText, metrics.scale) + 2.f * kFilePadX, std::max(box.width - 2.f, 0.f));
    Rect const segment{box.x + 1.f, box.y + 1.f, segmentWidth, std::max(box.height - 2.f, 0.f)};

    canvas.fill(segment, (mHovered && !disabled()) ? theme.buttonHover : theme.button, fade);
    canvas.outline(segment, theme.inputBorder.withAlpha(fade), kBorder);
    canvas.text(
        mChooseText,
        {segment.x, segment.y + (segment.height - lineHeight) / 2.f},
        segment.width,
        theme.text.withAlpha(fade),
        metrics.scale,
        Align::Center
    );

    float const      labelX     = segment.right() + kFileGap;
    float const      labelWidth = std::max(box.right() - labelX - 4.f, 0.f);
    bool const       empty      = mEdit.empty();
    std::string_view label      = empty ? std::string_view{mEmptyFileText} : std::string_view{mEdit.text()};
    drawFittedText(
        canvas,
        label,
        {labelX, box.y + (box.height - lineHeight) / 2.f},
        labelWidth,
        metrics.scale,
        (empty ? theme.textMuted : theme.text).withAlpha(fade)
    );
}

void Input::activate() {
    // A file input is picked by the host (`wantsFilePicker`), not by the widget:
    // the widget has no way to show a dialog and stays free of the platform.
    if (disabled() || mKind == Kind::File) {
        return;
    }
    mEdit.moveEnd();
}

void Input::activateAt(Point point) {
    if (disabled() || mKind == Kind::File) {
        return;
    }
    // The click position is turned into a caret once text metrics are available
    // again, i.e. during the next layout pass of the same frame.
    mPendingClick  = true;
    mPendingClickX = point.x;
}

void Input::setFocused(bool focused) {
    if (mFocused == focused) {
        return;
    }
    mFocused = focused;
    if (focused && !mEdit.empty()) {
        mEdit.moveEnd();
    }
}

bool Input::key(KeyStroke const& stroke) {
    if (!acceptsTextInput()) {
        return false;
    }
    std::string const before = mEdit.text();

    switch (stroke.key) {
    case Key::Backspace:
        mEdit.eraseBackward();
        notifyChanged(before);
        return true;
    case Key::Delete:
        mEdit.eraseForward();
        notifyChanged(before);
        return true;
    case Key::Left:
        stroke.shift ? mEdit.extendCaret(-1) : mEdit.moveCaret(-1);
        return true;
    case Key::Right:
        stroke.shift ? mEdit.extendCaret(1) : mEdit.moveCaret(1);
        return true;
    case Key::Home:
        if (stroke.shift) {
            mEdit.selectTo(0);
        } else {
            mEdit.moveHome();
        }
        return true;
    case Key::End:
        if (stroke.shift) {
            mEdit.selectTo(mEdit.text().size());
        } else {
            mEdit.moveEnd();
        }
        return true;
    case Key::A:
        if (stroke.ctrl) {
            mEdit.selectAll();
            return true;
        }
        return false;
    case Key::Enter:
        if (mOnSubmit) {
            mOnSubmit(mEdit.text());
        }
        return true;
    default:
        return false;
    }
}

void Input::textInput(std::string_view text) {
    if (!acceptsTextInput()) {
        return;
    }
    std::string const before = mEdit.text();
    mEdit.insert(text);
    notifyChanged(before);
}

std::string Input::selectedText() const { return mEdit.selectedText(); }

void Input::notifyChanged(std::string const& previous) {
    if (mEdit.text() == previous) {
        return;
    }
    // The validator sees every user edit, before the change callback, so a
    // consumer reading `invalid()` inside it already sees the new verdict.
    runValidation();
    if (mOnChange) {
        mOnChange(mEdit.text());
    }
}

} // namespace bul
