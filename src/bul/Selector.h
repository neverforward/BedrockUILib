#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "bul/Export.h"
#include "bul/Style.h"

namespace bul {

class Widget;

/// How two compounds of a selector relate: `A B` (descendant), `A > B` (child),
/// `A + B` (next sibling), `A ~ B` (any following sibling).
enum class Combinator { Descendant, Child, Adjacent, Sibling };

/// One `[name op value]` condition. `value` is unused for `Exists`.
struct BUL_API Attribute {
    /// The operator of an attribute condition, one per CSS spelling.
    enum class Match { Exists, Equals, Word, PrefixDash, StartsWith, EndsWith, Contains };

    std::string name{};
    std::string value{};
    Match       match{Match::Exists};

    friend bool operator==(Attribute const&, Attribute const&) = default;
};

/// One `:name` or `:name(argument)` condition.
struct BUL_API Pseudo {
    std::string name{};
    std::string argument{};

    friend bool operator==(Pseudo const&, Pseudo const&) = default;
};

/// One simple selector, the part without combinators: `Button.danger#submit:hover`.
struct BUL_API Compound {
    bool                    universal{}; // an explicit `*`
    std::string             type{};      // element name; empty matches any
    std::string             className{};
    std::string             id{};
    std::vector<Attribute>  attributes{};
    std::vector<Pseudo>     pseudos{};

    /// CSS specificity of this compound: id 100, class / attribute / pseudo
    /// class 10, element 1.
    [[nodiscard]] int specificity() const noexcept;

    friend bool operator==(Compound const&, Compound const&) = default;
};

/// A selector, the kit's spelling of
/// `Stack > Label.danger#submit[data-kind="text"]:hover`:
///
/// * `steps` are compounds with the combinator that links each to the one
///   before it; the last compound is the widget the rule applies to.
/// * Matching walks the tree: ancestors for `Descendant`, the parent for
///   `Child`, and the siblings for `Adjacent` / `Sibling`.
///
/// Parsing and matching are implementation details of `Selector.cpp`; this
/// header only declares the shape so a rule can be built and stored.
struct BUL_API Selector {
    /// One compound and how it connects to the compound before it.
    struct Step {
        Combinator combinator{Combinator::Descendant};
        Compound   compound{};
    };

    std::vector<Step> steps{};

    /// Reads one selector written the way a stylesheet writes it (attributes and
    /// pseudo classes included). Returns nothing when the text is not a
    /// selector.
    [[nodiscard]] static std::optional<Selector> parse(std::string_view text);

    /// Reads a comma-separated selector list, `Button, .danger`.
    [[nodiscard]] static std::vector<Selector> parseList(std::string_view text);

    /// CSS specificity: the sum over the compounds.
    [[nodiscard]] int specificity() const noexcept;

    /// Whether `widget` matches, with its ancestors and siblings.
    [[nodiscard]] bool matches(Widget const& widget) const;

    /// The same for a widget described by its three attributes: only a single
    /// compound is considered.
    [[nodiscard]] bool
    matches(std::string_view type, std::string_view className, std::string_view id) const noexcept;

    /// The selector written back as text, for logs and tests.
    [[nodiscard]] std::string text() const;
};

/// One selector with the style it applies.
struct BUL_API Rule {
    Selector selector{};
    Style    style{};
};

} // namespace bul
