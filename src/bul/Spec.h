#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "bul/Color.h"
#include "bul/Widget.h" // every component, so the builders below can reach them

namespace bul::spec {

/// The properties every node can carry: they map to the `Widget` base class and
/// are applied after the component itself is built. It is the first member of
/// every node, so designated initializers can reach it.
struct Common {
    float                     margin{};
    float                     fixedWidth{};
    float                     fixedHeight{};
    float                     minWidth{};
    float                     minHeight{};
    float                     maxWidth{};
    float                     maxHeight{};
    std::function<void()>     onClick{};
    std::function<void()>     onFocus{};
    std::function<void()>     onBlur{};
    std::function<void(bool)> onHover{};
    Style                     style{};
};

/// A text run's font: the size is relative to the theme base size, and the colour
/// is optional (muted nodes use the theme's muted colour).
struct Font {
    float               size{1.f};
    std::optional<Rgba> color{};
};

/// A child node: it holds the builder of one concrete node, so the tree can mix
/// node types without making the nodes themselves polymorphic (which would cost
/// them their aggregate initializers).
struct Node {
    std::function<std::unique_ptr<Widget>()> builder{};

    [[nodiscard]] std::unique_ptr<Widget> build() const { return builder ? builder() : nullptr; }
};

namespace detail {

/// Applies the shared `Widget` properties after a component is built.
[[nodiscard]] inline std::unique_ptr<Widget> finish(std::unique_ptr<Widget> widget, Common const& common) {
    if (common.margin > 0.f) {
        widget->margin(common.margin);
    }
    if (common.fixedWidth > 0.f || common.fixedHeight > 0.f) {
        widget->fixedSize(common.fixedWidth, common.fixedHeight);
    }
    if (common.minWidth > 0.f || common.minHeight > 0.f) {
        widget->minSize(common.minWidth, common.minHeight);
    }
    if (common.maxWidth > 0.f || common.maxHeight > 0.f) {
        widget->maxSize(common.maxWidth, common.maxHeight);
    }
    if (common.onClick) {
        widget->onClick(common.onClick);
    }
    if (common.onFocus) {
        widget->onFocus(common.onFocus);
    }
    if (common.onBlur) {
        widget->onBlur(common.onBlur);
    }
    if (common.onHover) {
        widget->onHover(common.onHover);
    }
    if (!common.style.empty()) {
        widget->style(common.style);
    }
    return widget;
}

} // namespace detail

/// Wraps a node value into the owning pointer the `children` lists take.
template <class T>
[[nodiscard]] std::shared_ptr<Node> node(T value) {
    // std::function needs a copyable target and a node may own unique_ptr
    // children, so the spec is held through a shared pointer.
    auto held = std::make_shared<T>(std::move(value));
    return std::make_shared<Node>(Node{[held] { return detail::finish(held->buildContent(), held->common); }});
}

/// Builds a whole tree from its root node.
[[nodiscard]] inline std::unique_ptr<Widget> build(Node const& root) { return root.build(); }

/// Builds a spec value directly.
template <class T>
[[nodiscard]] std::unique_ptr<Widget> build(T const& spec) {
    return detail::finish(spec.buildContent(), spec.common);
}

/// A block of text, the declarative `Label`.
///
/// ```cpp
/// auto tree = bul::spec::build(bul::spec::Label{.text = "Hello", .font = {.size = 1.4f}});
/// ```
struct Label {
    Common      common{};
    std::string text{};
    Font        font{};
    Align       align{Align::Left};
    VAlign      verticalAlign{VAlign::Top};
    bool        wrap{};
    float       wrapWidth{};
    std::size_t maxLines{};
    bool        muted{};

    [[nodiscard]] BUL_API std::unique_ptr<Widget> buildContent() const;
};

/// A button with its type, size and click action.
struct Button {
    Common                common{};
    std::string           text{};
    bul::Button::Type     type{bul::Button::Type::Default};
    bul::Button::Size     size{bul::Button::Size::Default};
    bool                  disabled{};
    std::function<void()> onClick{};

    [[nodiscard]] BUL_API std::unique_ptr<Widget> buildContent() const;
};

/// A small annotation pill.
struct Badge {
    Common              common{};
    std::string         text{};
    bul::Badge::Variant variant{bul::Badge::Variant::Default};

    [[nodiscard]] BUL_API std::unique_ptr<Widget> buildContent() const;
};

/// A single line text field.
struct Input {
    Common                                  common{};
    std::string                             value{};
    std::string                             placeholder{};
    bul::Input::Kind                        kind{bul::Input::Kind::Text};
    bool                                    disabled{};
    bool                                    invalid{};
    bool                                    required{};
    std::size_t                             maxLength{};
    std::function<void(std::string const&)> onChange{};
    std::function<void(std::string const&)> onSubmit{};

    [[nodiscard]] BUL_API std::unique_ptr<Widget> buildContent() const;
};

/// A label / control / description composition.
struct Field {
    using Orientation = bul::Field::Orientation;

    Common                common{};
    std::string           label{};
    std::string           description{};
    std::shared_ptr<Node> control{};
    Orientation           orientation{Orientation::Vertical};
    float                 gap{-1.f};
    bool                  disabled{};
    bool                  invalid{};
    bool                  required{};

    [[nodiscard]] BUL_API std::unique_ptr<Widget> buildContent() const;
};

/// A row or column of children.
struct Stack {
    using Direction = bul::Stack::Direction;

    Common                             common{};
    Direction                          direction{Direction::Vertical};
    float                              gap{-1.f};
    std::vector<std::shared_ptr<Node>> children{};

    [[nodiscard]] BUL_API std::unique_ptr<Widget> buildContent() const;
};

/// A dropdown.
struct Select {
    struct Item {
        std::string value{};
        std::string label{};
        bool        disabled{};
    };

    Common                                  common{};
    std::string                             placeholder{};
    std::string                             value{};
    std::vector<Item>                       items{};
    bool                                    disabled{};
    bool                                    required{};
    std::function<void(std::string const&)> onValueChange{};

    [[nodiscard]] BUL_API std::unique_ptr<Widget> buildContent() const;
};

/// A tick box with its mixed state.
struct Checkbox {
    Common                    common{};
    std::string               text{};
    bool                      checked{};
    bool                      indeterminate{};
    bool                      readOnly{};
    bool                      disabled{};
    bool                      invalid{};
    bool                      required{};
    std::function<void(bool)> onCheckedChange{};

    [[nodiscard]] BUL_API std::unique_ptr<Widget> buildContent() const;
};

/// An on/off switch.
struct Switch {
    Common                    common{};
    std::string               text{};
    bool                      checked{};
    bool                      readOnly{};
    bool                      disabled{};
    bool                      invalid{};
    bool                      required{};
    std::function<void(bool)> onCheckedChange{};

    [[nodiscard]] BUL_API std::unique_ptr<Widget> buildContent() const;
};

/// A group of radio buttons.
struct RadioGroup {
    struct Option {
        std::string value{};
        std::string text{};
        bool        disabled{};
    };

    Common                                  common{};
    std::string                             value{};
    std::vector<Option>                     options{};
    bul::Stack::Direction                   direction{bul::Stack::Direction::Vertical};
    float                                   gap{-1.f};
    bool                                    disabled{};
    bool                                    readOnly{};
    bool                                    invalid{};
    bool                                    required{};
    std::function<void(std::string const&)> onValueChange{};

    [[nodiscard]] BUL_API std::unique_ptr<Widget> buildContent() const;
};

/// A grid of equal cells, the declarative Grid.
struct Grid {
    Common                             common{};
    std::size_t                        columns{2};
    float                              gap{-1.f};
    std::vector<std::shared_ptr<Node>> children{};

    [[nodiscard]] BUL_API std::unique_ptr<Widget> buildContent() const;
};

/// A table of cells on fixed and flexible tracks, the declarative Table.
struct Table {
    struct Cell {
        std::size_t           row{};
        std::size_t           column{};
        std::shared_ptr<Node> control{};
    };

    Common                     common{};
    std::vector<layout::Track> columns{};
    std::vector<layout::Track> rows{};
    float                      columnGap{};
    float                      rowGap{};
    float                      padding{};
    float                      cellPadding{4.f};
    bool                       border{true};
    std::optional<Rgba>        borderColor{};
    std::vector<Cell>          cells{};

    [[nodiscard]] BUL_API std::unique_ptr<Widget> buildContent() const;
};

/// A scroll viewport over one content node.
struct ScrollView {
    Common                common{};
    float                 viewport{};
    bool                  scrollbar{true};
    std::shared_ptr<Node> content{};

    [[nodiscard]] BUL_API std::unique_ptr<Widget> buildContent() const;
};
/// The label of a field (usually built by Field).
struct FieldLabel {
    Common                common{};
    std::string           text{};
    float                 scale{1.f};
    bool                  required{};
    bool                  muted{};
    std::shared_ptr<Node> badge{};

    [[nodiscard]] BUL_API std::unique_ptr<Widget> buildContent() const;
};

/// The helper line of a field (usually built by Field).
struct FieldDescription {
    Common      common{};
    std::string text{};
    float       scale{0.85f};

    [[nodiscard]] BUL_API std::unique_ptr<Widget> buildContent() const;
};

} // namespace bul::spec
