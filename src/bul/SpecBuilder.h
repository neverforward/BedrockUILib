#pragma once

#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "bul/Spec.h"
#include "bul/Widget.h"

/// The imperative spelling of a spec tree, the shape of the planned `Screen` API:
/// a builder object whose methods append to the container that is open.
///
/// ```cpp
/// auto tree = bul::spec::build([](bul::spec::Builder& ui) {
///     ui.label("Settings", 1.4f);
///     ui.row([&] {
///         ui.button("Apply", [] {});
///         ui.button("Cancel", [] {});
///     });
/// });
/// ```
namespace bul::spec {

/// Appends widgets to a tree. The struct nodes in `bul/Spec.h` are the
/// declarative alternative; both produce the same widgets.
class BUL_API Builder {
public:
    Builder();
    Builder(Builder const&)            = delete;
    Builder& operator=(Builder const&) = delete;

    // Chainable common properties: they apply to the widget added last, so a
    // chain reads left to right (ui.button("OK").margin(2.f).className("x")).
    Builder& margin(float value);
    Builder& fixedSize(float width, float height);
    Builder& minSize(float width, float height);
    Builder& maxSize(float width, float height);
    Builder& className(std::string value);
    Builder& style(Style value);
    Builder& onClick(std::function<void()> handler);

    /// Registers the next widget under `id`, so it can be found again after the
    /// tree is built (`find`).
    Builder& id(std::string value);

    /// Applies these base properties to the next widget added (`margin`, `style`,
    /// `className`, the size constraints, the event handlers).
    Builder& with(Common const& common);

    // Containers: `body` runs with that container open, so the calls inside it
    // append to it.
    Builder& row(std::function<void()> body, std::function<void(bul::Stack&)> configure = {});
    Builder& column(std::function<void()> body, std::function<void(bul::Stack&)> configure = {});
    Builder& grid(std::size_t columns, std::function<void()> body, std::function<void(bul::Grid&)> configure = {});
    /// A scroll viewport; the body builds its content column.
    Builder& scroll(float viewport, std::function<void()> body, std::function<void(bul::ScrollView&)> configure = {});
    /// A table; configure fills in its tracks and cells.
    Builder& table(std::function<void(bul::Table&)> configure);
    /// A label / control / description field; the body builds the control.
    Builder& field(
        std::string                      label,
        std::function<void()>            body,
        std::string                      description = {},
        std::function<void(bul::Field&)> configure   = {}
    );

    /// Adds any widget, including one the kit does not know: a custom component
    /// needs no spec node to join a builder tree. stretch > 0 makes it share
    /// the room the fixed siblings leave, like Stack::add(child, stretch).
    Builder& add(std::unique_ptr<Widget> widget, float stretch = 0.f);

    // Leaves.
    Builder& label(std::string text, float size = 1.f, bool muted = false, std::function<void(bul::Label&)> = {});
    Builder&
    button(std::string text, std::function<void()> onClick = {}, std::function<void(bul::Button&)> configure = {});
    Builder& badge(std::string text, std::function<void(bul::Badge&)> configure = {});
    Builder&
    input(std::string placeholder = {}, std::string value = {}, std::function<void(bul::Input&)> configure = {});
    Builder& checkbox(std::string text, bool checked = false, std::function<void(bul::Checkbox&)> configure = {});
    /// The on/off switch (`switch` is a keyword).
    Builder& toggle(std::string text, bool checked = false, std::function<void(bul::Switch&)> configure = {});
    Builder& select(
        std::string                          placeholder,
        std::vector<bul::spec::Select::Item> items     = {},
        std::function<void(bul::Select&)>    configure = {}
    );
    Builder& radio(
        std::vector<bul::spec::RadioGroup::Option> options,
        std::string                                value     = {},
        std::function<void(bul::RadioGroup&)>      configure = {}
    );
    Builder& fieldLabel(std::string text, float scale = 1.f, bool required = false, bool muted = false);
    Builder& fieldDescription(std::string text, float scale = 0.85f);

    /// The widget registered under id, or nullptr; valid while the tree lives.
    [[nodiscard]] Widget* find(std::string_view id) const;
    template <class T>
    [[nodiscard]] T* find(std::string_view id) const {
        return dynamic_cast<T*>(find(id));
    }

    /// The finished tree; the builder is empty afterwards. build() is the same
    /// call, for closing a chain.
    [[nodiscard]] std::unique_ptr<Widget> take();
    /// Same as 	ake(), for closing a chain.
    [[nodiscard]] std::unique_ptr<Widget> build();

private:
    using Sink = std::function<void(std::unique_ptr<Widget>, float stretch)>;

    void emit(std::unique_ptr<Widget> widget, float stretch = 0.f);
    void scoped(Sink sink, std::function<void()> const& body);

    std::unique_ptr<bul::Stack>                  mRoot;
    std::vector<Sink>                            mSinks;
    Common                                       mPending{};
    bool                                         mPendingSet{};
    std::vector<std::pair<std::string, Widget*>> mById;
    Widget*                                      mLast{};
};

/// Builds a tree from a builder body.
template <class F>
    requires std::invocable<F&, Builder&>
[[nodiscard]] std::unique_ptr<Widget> build(F&& body) {
    Builder builder;
    body(builder);
    return builder.take();
}

} // namespace bul::spec
