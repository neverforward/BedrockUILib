// Demo gallery for the widget kit.
//
// This is the demo as documentation: build a widget tree and ask the library for
// a modal window. The gallery is a tiny router written with the public widgets
// only -- a home screen lists the components, each entry opens its own page, and
// every page is short enough to read as the reference for that component.

#include "bul/demo/DemoPanel.h"

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ll/api/event/EventBus.h"
#include "ll/api/event/client/ClientJoinLevelEvent.h"
#include "ll/api/event/input/KeyInputEvent.h"
#include "ll/api/i18n/I18n.h"

#include "bul/Modal.h"
#include "bul/Theme.h"
#include "bul/Widget.h"
#include "mod/Mod.h"

using namespace ll::literals;

namespace bul::demo {

namespace {

// mc/deps/input/Keyboard.h: F6 = 117.
constexpr int kToggleKeyCode = 117;

ll::event::ListenerPtr gKeyListener;
ll::event::ListenerPtr gJoinListener;

[[nodiscard]] ll::io::Logger& log() { return BedrockUILib::getInstance().getSelf().getLogger(); }

enum class Page : std::size_t { Home = 0, Buttons = 1, Input = 2, Select = 3 };

/// The gallery's router: one page is live at a time and the widget protocol is
/// forwarded to it. Keeping it in the demo leaves the library free of navigation
/// policy, and writing it with the public widgets doubles as an example of a
/// custom container.
class Router final : public Widget {
public:
    Router() = default;

    Router& add(std::unique_ptr<Widget> page) {
        mPages.push_back(std::move(page));
        return *this;
    }

    void show(std::size_t index) noexcept {
        if (index < mPages.size()) {
            mCurrent = index;
        }
    }

    [[nodiscard]] Size measure(float availableWidth, Theme const& theme, Canvas const& canvas) const override {
        return mPages[mCurrent]->measure(availableWidth, theme, canvas);
    }

    void layout(Rect const& outer, Theme const& theme, Canvas const& canvas) override {
        Widget::layout(outer, theme, canvas);
        mPages[mCurrent]->layout(mBounds, theme, canvas);
    }

    void paint(Canvas& canvas, Theme const& theme) const override { mPages[mCurrent]->paint(canvas, theme); }

    [[nodiscard]] Widget* hitTest(Point point) override {
        if (Widget* hit = mPages[mCurrent]->hitTest(point)) {
            return hit;
        }
        return mBounds.contains(point) ? this : nullptr;
    }

    void setControlState(ControlState const& state) override { mPages[mCurrent]->setControlState(state); }

    void collectFocusables(std::vector<Widget*>& out) override { mPages[mCurrent]->collectFocusables(out); }

    void collectOverlays(std::vector<Widget*>& out) override { mPages[mCurrent]->collectOverlays(out); }

private:
    std::vector<std::unique_ptr<Widget>> mPages{};
    std::size_t                          mCurrent{};
};

// ---------------------------------------------------------------- tiny layout DSL

[[nodiscard]] std::unique_ptr<Stack> row(float gap = 6.f) {
    auto stack = std::make_unique<Stack>(Stack::Direction::Horizontal);
    stack->gap(gap);
    return stack;
}

[[nodiscard]] std::unique_ptr<Stack> column(float gap = 6.f) {
    auto stack = std::make_unique<Stack>(Stack::Direction::Vertical);
    stack->gap(gap);
    return stack;
}

/// The small muted line used for explanations.
[[nodiscard]] std::unique_ptr<Label> caption(std::string text) {
    auto label = std::make_unique<Label>(std::move(text), 0.85f);
    label->muted();
    return label;
}

/// A page: title, one-line explanation, body, and the Back / Close footer.
[[nodiscard]] std::unique_ptr<Widget>
page(std::string title, std::string intro, std::unique_ptr<Widget> body, std::function<void()> back) {
    auto content = column(8.f);
    content->add(std::make_unique<Label>(std::move(title), 1.2f));
    content->add(caption(std::move(intro)));
    content->add(std::move(body));

    auto actions = row(8.f);
    if (back) {
        auto backButton = std::make_unique<Button>("Back", back);
        backButton->type(Button::Type::Outline);
        actions->add(std::move(backButton));
    }
    actions->add(std::make_unique<Button>("Close", [] { closeModal(); }));
    content->add(std::move(actions));
    return content;
}

// --------------------------------------------------------------------- home page

[[nodiscard]] std::unique_ptr<Widget> homePage(std::function<void(std::size_t)> navigate) {
    auto menu  = column(6.f);
    auto entry = [&menu, &navigate](char const* text, Page target) {
        auto button =
            std::make_unique<Button>(text, [navigate, target] { navigate(static_cast<std::size_t>(target)); });
        button->type(Button::Type::Outline);
        menu->add(std::move(button));
    };
    entry("Buttons", Page::Buttons);
    entry("Input", Page::Input);
    entry("Select", Page::Select);
    return page("BedrockUILib", "A native widget kit. Pick a component.", std::move(menu), {});
}

// ------------------------------------------------------------------- button page

[[nodiscard]] std::unique_ptr<Widget> buttonsPage(std::function<void()> back) {
    auto body = column(6.f);

    body->add(caption("type: the surface and text colour of the action"));
    auto types   = row(5.f);
    auto addType = [&types](char const* text, Button::Type type) {
        auto button = std::make_unique<Button>(text, [text] { log().debug("demo: button '{}' pressed", text); });
        button->type(type).size(Button::Size::Small);
        types->add(std::move(button));
    };
    addType("default", Button::Type::Default);
    addType("secondary", Button::Type::Secondary);
    addType("destructive", Button::Type::Destructive);
    addType("outline", Button::Type::Outline);
    addType("link", Button::Type::Link);
    body->add(std::move(types));

    body->add(caption("size: text scale and padding only"));
    auto sizes   = row(5.f);
    auto addSize = [&sizes](char const* text, Button::Size size) {
        auto button = std::make_unique<Button>(text, [text] { log().debug("demo: button '{}' pressed", text); });
        button->size(size);
        sizes->add(std::move(button));
    };
    addSize("xs", Button::Size::XSmall);
    addSize("sml", Button::Size::Small);
    addSize("default", Button::Size::Default);
    addSize("lg", Button::Size::Large);
    body->add(std::move(sizes));

    body->add(caption("disabled: dimmed, and a click does nothing"));
    auto disabled    = row(5.f);
    auto addDisabled = [&disabled](char const* text, Button::Type type) {
        auto button = std::make_unique<Button>(text);
        button->type(type);
        button->disabled(true);
        disabled->add(std::move(button));
    };
    addDisabled("default", Button::Type::Default);
    addDisabled("outline", Button::Type::Outline);
    addDisabled("link", Button::Type::Link);
    body->add(std::move(disabled));

    return page("Buttons", "type picks the surface, size picks the padding.", std::move(body), back);
}

// -------------------------------------------------------------------- input page

/// Field with a label, an input and a hint (the basic composition).
[[nodiscard]] std::unique_ptr<Field> basicField() {
    auto field = std::make_unique<Field>();
    field->label("Email");

    auto input = std::make_unique<Input>();
    input->placeholder("you@example.com");
    input->onChange([](std::string const& value) { log().debug("demo: email='{}'", value); });
    field->control(std::move(input));

    field->description("Shown under the input.");
    return field;
}

/// Same field with the disabled attribute on the field and the input.
[[nodiscard]] std::unique_ptr<Field> disabledField() {
    auto field = std::make_unique<Field>();
    field->label("Email");
    field->disabled(true);

    auto input = std::make_unique<Input>();
    input->placeholder("you@example.com");
    input->disabled(true);
    field->control(std::move(input));
    return field;
}

/// The validator: red while the check fails, normal as soon as it passes.
[[nodiscard]] std::unique_ptr<Field> validatedField() {
    auto field = std::make_unique<Field>();
    field->label("Email");

    auto input = std::make_unique<Input>();
    input->placeholder("you@example.com");
    input->validator([](std::string const& value) { return value.find('@') != std::string::npos; });
    field->control(std::move(input));

    field->description("Red until it contains @.");
    return field;
}

/// The required attribute: the label grows a marker.
[[nodiscard]] std::unique_ptr<Field> requiredField() {
    auto field = std::make_unique<Field>();
    field->label("Email");
    field->required(true);

    auto input = std::make_unique<Input>();
    input->placeholder("you@example.com");
    input->required(true);
    field->control(std::move(input));
    return field;
}

/// A badge inside the label, marking the field as recommended.
[[nodiscard]] std::unique_ptr<Field> badgeField() {
    auto field = std::make_unique<Field>();

    auto label = std::make_unique<FieldLabel>("Password");
    label->badge(std::make_unique<Badge>("Recommended", Badge::Variant::Secondary));
    field->label(std::move(label));

    auto input = std::make_unique<Input>();
    input->kind(Input::Kind::Password);
    input->placeholder("Password");
    field->control(std::move(input));
    return field;
}

/// A file input: the host opens the native dialog and the widget shows the name.
[[nodiscard]] std::unique_ptr<Field> fileField() {
    auto field = std::make_unique<Field>();
    field->label("Picture");

    auto input = std::make_unique<Input>();
    input->kind(Input::Kind::File);
    input->onChoose([](std::string const& path) { log().debug("demo: file chosen '{}'", path); });
    field->control(std::move(input));

    field->description("Select a picture to upload.");
    return field;
}

/// Inline search: a horizontal field with the input and a button in one row.
[[nodiscard]] std::unique_ptr<Field> searchField() {
    auto field = std::make_unique<Field>();
    field->orientation(Field::Orientation::Horizontal);
    field->label("Search");

    auto content = row(6.f);
    auto input   = std::make_unique<Input>();
    input->kind(Input::Kind::Search);
    input->placeholder("Search the components...");
    auto* raw = input.get();
    input->onSubmit([](std::string const& value) { log().debug("demo: search submitted '{}'", value); });
    content->add(std::move(input), 1.f); // the input takes the room the button leaves
    content->add(std::make_unique<Button>("Submit", [raw] {
        log().debug("demo: search button pressed, value='{}'", raw->value());
    }));
    field->control(std::move(content));
    return field;
}

[[nodiscard]] std::unique_ptr<Widget> inputPage(std::function<void()> back) {
    auto body = column(8.f);

    // Two columns keep the gallery inside the modal's height limit.
    auto grid = row(10.f);
    auto left = column(8.f);
    left->add(basicField());
    left->add(disabledField());
    left->add(badgeField());
    auto right = column(8.f);
    right->add(validatedField());
    right->add(requiredField());
    right->add(fileField());
    grid->add(std::move(left), 1.f);
    grid->add(std::move(right), 1.f);
    body->add(std::move(grid));

    body->add(searchField());
    return page("Input", "Label, control, hint; states are field attributes.", std::move(body), back);
}

// ------------------------------------------------------------------- select page

/// Select.Value starts empty and shows the placeholder until an option is picked.
[[nodiscard]] std::unique_ptr<Field> fruitField() {
    auto field = std::make_unique<Field>();
    field->label("Fruit");

    auto select = std::make_unique<Select>();
    select->placeholder("Choose a fruit");
    select->item("apple", "Apple");
    select->item("banana", "Banana");
    select->item("cherry", "Cherry");
    select->item("durian", "Durian (out of stock)", true);
    select->onValueChange([](std::string const& value) { log().debug("demo: fruit='{}'", value); });
    field->control(std::move(select));

    field->description("Esc or outside click.");
    return field;
}

/// A preset value: the trigger shows the label of the matching item.
[[nodiscard]] std::unique_ptr<Field> themeField() {
    auto field = std::make_unique<Field>();
    field->label("Theme");

    auto select = std::make_unique<Select>();
    select->value("dark");
    select->item("light", "Light");
    select->item("dark", "Dark");
    select->item("system", "System");
    field->control(std::move(select));
    return field;
}

/// The field's disabled attribute reaches the control through setControlState.
[[nodiscard]] std::unique_ptr<Field> disabledSelectField() {
    auto field = std::make_unique<Field>();
    field->label("Disabled");
    field->disabled(true);

    auto select = std::make_unique<Select>();
    select->placeholder("Not available");
    select->item("one", "One");
    field->control(std::move(select));
    return field;
}

/// Required is a field attribute; the select only carries the flag.
[[nodiscard]] std::unique_ptr<Field> requiredSelectField() {
    auto field = std::make_unique<Field>();
    field->label("Required");
    field->required(true);

    auto select = std::make_unique<Select>();
    select->placeholder("Pick one");
    select->item("one", "One");
    select->item("two", "Two");
    field->control(std::move(select));
    return field;
}

[[nodiscard]] std::unique_ptr<Widget> selectPage(std::function<void()> back) {
    auto grid = row(10.f);
    auto left = column(8.f);
    left->add(fruitField());
    left->add(disabledSelectField());
    auto right = column(8.f);
    right->add(themeField());
    right->add(requiredSelectField());
    grid->add(std::move(left), 1.f);
    grid->add(std::move(right), 1.f);

    auto body = column(8.f);
    body->add(std::move(grid));
    return page("Select", "One value from a list, opened over the form.", std::move(body), back);
}

// ------------------------------------------------------------------------- window

[[nodiscard]] std::unique_ptr<Widget> makeDemoWindow() {
    auto  router   = std::make_unique<Router>();
    auto* view     = router.get();
    auto  gotoPage = [view](std::size_t index) { view->show(index); };
    auto  back     = [view] { view->show(static_cast<std::size_t>(Page::Home)); };

    view->add(homePage(gotoPage));
    view->add(buttonsPage(back));
    view->add(inputPage(back));
    view->add(selectPage(back));
    return router;
}

void toggle() {
    if (modalVisible()) {
        closeModal();
        log().debug("demo: modal closing");
        return;
    }
    bool const shown = showModal(makeDemoWindow());
    log().debug("demo: modal shown={}", shown);
}

} // namespace

void install() {
    if (gKeyListener) {
        return;
    }
    auto& bus    = ll::event::EventBus::getInstance();
    auto& logger = BedrockUILib::getInstance().getSelf().getLogger();
    gKeyListener = bus.emplaceListener<ll::event::input::KeyInputEvent>([](ll::event::input::KeyInputEvent& event) {
        if (!event.isDown() || event.keyCode() != kToggleKeyCode) {
            return;
        }
        event.cancel();
        toggle();
    });
    // gJoinListener = bus.emplaceListener<ll::event::ClientJoinLevelEvent>(
    //     [](ll::event::ClientJoinLevelEvent&) { toggle(); }
    // );
    logger.info("demo installed: F6 opens the component gallery");
}

void uninstall() {
    auto& bus = ll::event::EventBus::getInstance();
    for (auto* listener : {&gKeyListener, &gJoinListener}) {
        if (*listener) {
            bus.removeListener(*listener);
            listener->reset();
        }
    }
    closeModal();
}

} // namespace bul::demo
