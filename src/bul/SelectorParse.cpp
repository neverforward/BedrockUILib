#include "bul/Selector.h"

#include <cctype>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace bul {

namespace {

// ----------------------------------------------------------------- scanning
//
// The parser is hand written because the syntax is tiny and the error path is
// "return nothing": no exceptions, and no allocation beyond the strings it
// builds. A cursor carries the position and the text; `peek` returns '\0' past
// the end so the callers do not repeat the bounds check on every character.

/// Characters that may appear in an element name, a class name or an id.
bool isIdentChar(char c) noexcept {
    auto const u = static_cast<unsigned char>(c);
    return std::isalnum(u) != 0 || c == '-' || c == '_';
}

bool isSpace(char c) noexcept { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }

/// Trims both ends; used for the text inside `:pseudo(...)`.
std::string_view trim(std::string_view text) noexcept {
    while (!text.empty() && isSpace(text.front())) {
        text.remove_prefix(1);
    }
    while (!text.empty() && isSpace(text.back())) {
        text.remove_suffix(1);
    }
    return text;
}

/// A position in the selector text.
struct Cursor {
    std::string_view text;
    std::size_t      pos{};

    [[nodiscard]] bool eof() const noexcept { return pos >= text.size(); }
    [[nodiscard]] char peek() const noexcept { return eof() ? '\0' : text[pos]; }
    char               get() noexcept { return text[pos++]; }

    /// Skips whitespace and reports whether it saw any.
    bool skipSpace() noexcept {
        bool seen = false;
        while (!eof() && isSpace(peek())) {
            get();
            seen = true;
        }
        return seen;
    }

    /// Takes the longest run of identifier characters.
    std::string_view ident() noexcept {
        std::size_t const start = pos;
        while (!eof() && isIdentChar(peek())) {
            get();
        }
        return text.substr(start, pos - start);
    }

    /// The text between balanced parentheses, both consumed.
    std::string_view parenthesised() noexcept {
        std::size_t const start = pos;
        int               depth = 0;
        while (!eof()) {
            char const c = get();
            if (c == '(') {
                ++depth;
            } else if (c == ')') {
                --depth;
                if (depth == 0) {
                    return text.substr(start + 1, pos - start - 2);
                }
            }
        }
        return text.substr(start + 1);
    }
};

// ---------------------------------------------------------------- attributes

/// Reads `=`, `~=`, `|=`, `^=`, `$=` or `*=`, the operator after the name.
std::optional<Attribute::Match> matchOperator(char first) noexcept {
    switch (first) {
    case '=':
        return Attribute::Match::Equals;
    case '~':
        return Attribute::Match::Word;
    case '|':
        return Attribute::Match::PrefixDash;
    case '^':
        return Attribute::Match::StartsWith;
    case '$':
        return Attribute::Match::EndsWith;
    case '*':
        return Attribute::Match::Contains;
    default:
        return std::nullopt;
    }
}

/// Reads the right-hand side of an attribute condition, quoted or bare.
void readAttributeValue(Cursor& cursor, std::string& out) {
    if (cursor.peek() == '"' || cursor.peek() == '\'') {
        char const        quote = cursor.get();
        std::size_t const start = cursor.pos;
        while (!cursor.eof() && cursor.peek() != quote) {
            cursor.get();
        }
        out = std::string{cursor.text.substr(start, cursor.pos - start)};
        if (!cursor.eof()) {
            cursor.get();
        }
        return;
    }
    out = std::string{cursor.ident()};
}

/// `[name]` or `[name op value]`, the brackets consumed.
bool parseAttribute(Cursor& cursor, Compound& compound) {
    cursor.get(); // '['
    cursor.skipSpace();
    Attribute attribute;
    attribute.name = std::string{cursor.ident()};
    if (attribute.name.empty()) {
        return false;
    }
    cursor.skipSpace();
    if (cursor.peek() == ']') {
        cursor.get();
        compound.attributes.push_back(std::move(attribute));
        return true;
    }

    auto const op = matchOperator(cursor.peek());
    if (!op) {
        return false;
    }
    cursor.get();
    if (*op != Attribute::Match::Equals && cursor.peek() == '=') {
        cursor.get();
    }
    cursor.skipSpace();
    readAttributeValue(cursor, attribute.value);
    cursor.skipSpace();
    if (cursor.peek() != ']') {
        return false;
    }
    cursor.get();
    attribute.match = *op;
    compound.attributes.push_back(std::move(attribute));
    return true;
}

// ----------------------------------------------------------------- compounds

/// The leading element name or `*`.
bool parseType(Cursor& cursor, Compound& compound) {
    if (cursor.peek() == '*') {
        cursor.get();
        compound.universal = true;
        return true;
    }
    if (!isIdentChar(cursor.peek())) {
        return false;
    }
    compound.type = std::string{cursor.ident()};
    return true;
}

/// `.class` or `#id`; the marker has already been identified by the caller.
bool parseClassOrId(Cursor& cursor, Compound& compound, char marker) {
    cursor.get();
    std::string name{cursor.ident()};
    if (name.empty()) {
        return false;
    }
    std::string& target = marker == '.' ? compound.className : compound.id;
    if (!target.empty()) {
        return false;
    }
    target = std::move(name);
    return true;
}

/// `:name` or `:name(argument)`; the colon has already been consumed.
bool parsePseudo(Cursor& cursor, Compound& compound) {
    Pseudo pseudo;
    pseudo.name = std::string{cursor.ident()};
    if (pseudo.name.empty()) {
        return false;
    }
    if (cursor.peek() == '(') {
        pseudo.argument = std::string{trim(cursor.parenthesised())};
    }
    compound.pseudos.push_back(std::move(pseudo));
    return true;
}

/// One compound: an optional type followed by any number of `.[]` or `:#`.
bool parseCompound(Cursor& cursor, Compound& compound) {
    bool any = parseType(cursor, compound);
    for (;;) {
        char const c  = cursor.peek();
        bool       ok = true;
        if (c == '.' || c == '#') {
            ok = parseClassOrId(cursor, compound, c);
        } else if (c == '[') {
            ok = parseAttribute(cursor, compound);
        } else if (c == ':') {
            cursor.get();
            // `::before` and friends are pseudo elements, which this model does
            // not have: the kit paints whole widgets, not text fragments.
            ok = cursor.peek() != ':' && parsePseudo(cursor, compound);
        } else {
            break;
        }
        if (!ok) {
            return false;
        }
        any = true;
    }
    return any;
}

/// Reads `>` / `+` / `~`; nothing when the next character is not one of them.
std::optional<Combinator> readCombinator(Cursor& cursor) noexcept {
    switch (cursor.peek()) {
    case '>':
        cursor.get();
        return Combinator::Child;
    case '+':
        cursor.get();
        return Combinator::Adjacent;
    case '~':
        cursor.get();
        return Combinator::Sibling;
    default:
        return std::nullopt;
    }
}

/// One selector: compounds separated by whitespace or an explicit combinator.
std::optional<Selector> parseOneSelector(std::string_view text) {
    Cursor    cursor{text};
    Selector  selector;
    Combinator next = Combinator::Descendant;

    cursor.skipSpace();
    for (;;) {
        Selector::Step step;
        step.combinator = selector.steps.empty() ? Combinator::Descendant : next;
        if (!parseCompound(cursor, step.compound)) {
            return std::nullopt;
        }
        selector.steps.push_back(std::move(step));

        bool const spaced = cursor.skipSpace();
        if (cursor.eof()) {
            return selector;
        }
        if (auto const explicitCombinator = readCombinator(cursor)) {
            next = *explicitCombinator;
        } else if (!spaced) {
            return std::nullopt;
        } else {
            next = Combinator::Descendant;
        }
        cursor.skipSpace();
    }
}

/// Splits on the commas that are not inside brackets or parentheses.
std::vector<std::string_view> splitTopLevel(std::string_view text) {
    std::vector<std::string_view> parts;
    std::size_t                   start = 0;
    int                           depth = 0;
    for (std::size_t i = 0; i < text.size(); ++i) {
        char const c = text[i];
        if (c == '(' || c == '[') {
            ++depth;
        } else if (c == ')' || c == ']') {
            --depth;
        } else if (c == ',' && depth == 0) {
            parts.push_back(text.substr(start, i - start));
            start = i + 1;
        }
    }
    parts.push_back(text.substr(start));
    return parts;
}

} // namespace

std::vector<Selector> Selector::parseList(std::string_view text) {
    std::vector<Selector> selectors;
    if (trim(text).empty()) {
        return selectors;
    }
    for (std::string_view part : splitTopLevel(text)) {
        if (auto const selector = parseOneSelector(part)) {
            selectors.push_back(*selector);
        }
    }
    return selectors;
}

std::optional<Selector> Selector::parse(std::string_view text) {
    std::vector<Selector> const list = parseList(text);
    if (list.size() != 1) {
        return std::nullopt;
    }
    return list.front();
}

} // namespace bul
