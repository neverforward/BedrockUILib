#include "bul/Selector.h"

#include <string>
#include <string_view>
#include <vector>

namespace bul {

namespace {

/// `type.class#id`.
void appendCompound(std::string& out, Compound const& compound) {
    if (compound.universal) {
        out += '*';
    } else {
        out += compound.type;
    }
    if (!compound.className.empty()) {
        out += '.';
        out += compound.className;
    }
    if (!compound.id.empty()) {
        out += '#';
        out += compound.id;
    }
}

/// The `[name op value]` part of a compound.
void appendAttributes(std::string& out, std::vector<Attribute> const& attributes) {
    for (auto const& attribute : attributes) {
        out += '[';
        out += attribute.name;
        switch (attribute.match) {
        case Attribute::Match::Exists:
            break;
        case Attribute::Match::Equals:
            out += '=';
            out += attribute.value;
            break;
        case Attribute::Match::Word:
            out += "~=";
            out += attribute.value;
            break;
        case Attribute::Match::PrefixDash:
            out += "|=";
            out += attribute.value;
            break;
        case Attribute::Match::StartsWith:
            out += "^=";
            out += attribute.value;
            break;
        case Attribute::Match::EndsWith:
            out += "$=";
            out += attribute.value;
            break;
        case Attribute::Match::Contains:
            out += "*=";
            out += attribute.value;
            break;
        }
        out += ']';
    }
}

/// The `:name(argument)` part of a compound.
void appendPseudos(std::string& out, std::vector<Pseudo> const& pseudos) {
    for (auto const& pseudo : pseudos) {
        out += ':';
        out += pseudo.name;
        if (!pseudo.argument.empty()) {
            out += '(';
            out += pseudo.argument;
            out += ')';
        }
    }
}

/// The combinator between two compounds, written the way it is read.
std::string_view combinatorText(Combinator combinator) noexcept {
    switch (combinator) {
    case Combinator::Descendant:
        return " ";
    case Combinator::Child:
        return " > ";
    case Combinator::Adjacent:
        return " + ";
    case Combinator::Sibling:
        return " ~ ";
    }
    return " ";
}

} // namespace

int Compound::specificity() const noexcept {
    int value = universal || type.empty() ? 0 : 1;
    if (!className.empty()) {
        value += 10;
    }
    if (!id.empty()) {
        value += 100;
    }
    value += static_cast<int>(attributes.size()) * 10;
    value += static_cast<int>(pseudos.size()) * 10;
    return value;
}

int Selector::specificity() const noexcept {
    int value = 0;
    for (auto const& step : steps) {
        value += step.compound.specificity();
    }
    return value;
}

std::string Selector::text() const {
    std::string out;
    for (std::size_t i = 0; i < steps.size(); ++i) {
        if (i > 0) {
            out += combinatorText(steps[i].combinator);
        }
        appendCompound(out, steps[i].compound);
        appendAttributes(out, steps[i].compound.attributes);
        appendPseudos(out, steps[i].compound.pseudos);
    }
    return out;
}

} // namespace bul
