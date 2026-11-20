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
#include "bul/Spec.h"
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

enum class Page : std::size_t {
    Home     = 0,
    Buttons  = 1,
    Input    = 2,
    Select   = 3,
    Checkbox = 4,
    Radio    = 5,
    Text     = 6,
    Widget   = 7,
    Switch   = 8,
    Spec     = 9
};

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

    [[nodiscard]] Size measureContent(float availableWidth, Theme const& theme, Canvas const& canvas) const override {
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
    entry("Checkbox", Page::Checkbox);
    entry("Radio", Page::Radio);
    entry("Text", Page::Text);
    entry("Widget", Page::Widget);
    entry("Switch", Page::Switch);
    entry("Spec", Page::Spec);
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

// ---------------------------------------------------------------- checkbox page

[[nodiscard]] std::unique_ptr<Checkbox> makeCheckbox(std::string text, bool checked = false) {
    auto box = std::make_unique<Checkbox>();
    box->text(std::move(text));
    box->checked(checked);
    return box;
}

[[nodiscard]] std::unique_ptr<Widget> checkboxPage(std::function<void()> back) {
    auto left = column(8.f);
    {
        auto basic = makeCheckbox("Accept terms");
        basic->onCheckedChange([](bool value) { log().debug("demo: accepted={}", value); });
        left->add(std::move(basic));
        left->add(makeCheckbox("Remember me", true));
        auto mixed = makeCheckbox("Select all");
        mixed->indeterminate(true);
        left->add(std::move(mixed));
    }

    auto right = column(8.f);
    {
        auto field = std::make_unique<Field>();
        field->label("Notifications");
        field->invalid(true);
        field->control(makeCheckbox("Email me"));
        field->description("Field invalid is red.");
        right->add(std::move(field));
    }
    {
        auto field = std::make_unique<Field>();
        field->label("Disabled");
        auto boxes = row(8.f);
        auto off   = makeCheckbox("Off");
        off->disabled(true);
        auto on = makeCheckbox("On", true);
        on->disabled(true);
        boxes->add(std::move(off));
        boxes->add(std::move(on));
        field->control(std::move(boxes));
        right->add(std::move(field));
    }
    {
        auto field = std::make_unique<Field>();
        field->label("Terms");
        field->required(true);
        field->control(makeCheckbox("I agree"));
        right->add(std::move(field));
    }

    auto grid = row(10.f);
    grid->add(std::move(left), 1.f);
    grid->add(std::move(right), 1.f);

    auto body = column(8.f);
    body->add(std::move(grid));
    return page("Checkbox", "A checkbox with an inline label; three states.", std::move(body), back);
}

// -------------------------------------------------------------------- radio page

[[nodiscard]] std::unique_ptr<Field> planField() {
    auto field = std::make_unique<Field>();
    field->label("Plan");
    auto group = std::make_unique<RadioGroup>();
    group->add("starter", "Starter");
    group->add("pro", "Pro");
    group->add("team", "Team");
    group->onValueChange([](std::string const& value) { log().debug("demo: plan='{}'", value); });
    field->control(std::move(group));
    field->description("Arrow keys move.");
    return field;
}

[[nodiscard]] std::unique_ptr<Field> layoutRadioField() {
    auto field = std::make_unique<Field>();
    field->label("Layout");
    auto group = std::make_unique<RadioGroup>();
    group->direction(Stack::Direction::Horizontal);
    group->add("compact", "Compact");
    group->add("comfortable", "Comfortable");
    field->control(std::move(group));
    return field;
}

[[nodiscard]] std::unique_ptr<Field> invalidRadioField() {
    auto field = std::make_unique<Field>();
    field->label("Notifications");
    field->invalid(true);
    auto group = std::make_unique<RadioGroup>();
    group->add("daily", "Daily");
    group->add("weekly", "Weekly");
    field->control(std::move(group));
    field->description("Field invalid is red.");
    return field;
}

[[nodiscard]] std::unique_ptr<Field> disabledRadioField() {
    auto field = std::make_unique<Field>();
    field->label("Disabled");
    field->disabled(true);
    auto group = std::make_unique<RadioGroup>();
    group->add("on", "On");
    group->add("off", "Off");
    field->control(std::move(group));
    return field;
}

[[nodiscard]] std::unique_ptr<Field> requiredRadioField() {
    auto field = std::make_unique<Field>();
    field->label("Terms");
    field->required(true);
    auto group = std::make_unique<RadioGroup>();
    group->add("accept", "Accept");
    group->add("decline", "Decline");
    field->control(std::move(group));
    return field;
}

[[nodiscard]] std::unique_ptr<Widget> radioPage(std::function<void()> back) {
    auto grid = row(10.f);
    auto left = column(8.f);
    left->add(planField());
    left->add(layoutRadioField());
    auto right = column(8.f);
    right->add(invalidRadioField());
    right->add(disabledRadioField());
    right->add(requiredRadioField());
    grid->add(std::move(left), 1.f);
    grid->add(std::move(right), 1.f);

    auto body = column(8.f);
    body->add(std::move(grid));
    return page("Radio", "One option out of a group; the dot marks it.", std::move(body), back);
}

// --------------------------------------------------------------------- text page

[[nodiscard]] std::unique_ptr<Widget> textPage(std::function<void()> back) {
    auto left = column(6.f);
    left->add(caption("size"));
    {
        auto small = std::make_unique<Label>("Small caption", 0.85f);
        small->muted();
        left->add(std::move(small));
    }
    left->add(std::make_unique<Label>("Body text"));
    left->add(std::make_unique<Label>("Section title", 1.2f));
    left->add(std::make_unique<Label>("Display", 1.6f));

    auto right = column(6.f);
    right->add(caption("align"));
    right->add(std::make_unique<Label>("Left", 1.f, Align::Left));
    right->add(std::make_unique<Label>("Center", 1.f, Align::Center));
    right->add(std::make_unique<Label>("Right", 1.f, Align::Right));

    auto grid = row(10.f);
    grid->add(std::move(left), 1.f);
    grid->add(std::move(right), 1.f);

    auto colorBlock = column(6.f);
    colorBlock->add(caption("color"));
    {
        auto colorRow = row(10.f);
        auto accent   = std::make_unique<Label>("Accent");
        accent->color(Theme::neutral().accent);
        colorRow->add(std::move(accent));
        auto danger = std::make_unique<Label>("Invalid");
        danger->color(Theme::neutral().invalid);
        colorRow->add(std::move(danger));
        auto success = std::make_unique<Label>("Success");
        success->color(Rgba::fromRgb(0x46A758));
        colorRow->add(std::move(success));
        colorBlock->add(std::move(colorRow));
    }

    auto wrapRow = row(10.f);
    auto english = column(6.f);
    english->add(caption("wrap: spaces"));
    {
        auto sentence =
            std::make_unique<Label>("Word wrap breaks the text at spaces, so a long sentence stays inside its column.");
        sentence->wrap(true);
        sentence->maxWidth(120.f);
        english->add(std::move(sentence));
    }
    auto chinese = column(6.f);
    chinese->add(caption("wrap: CJK"));
    {
        auto sentence = std::make_unique<Label>("中文没有空格，换行会在任意两个字之间断开，避免整段溢出。");
        sentence->wrap(true);
        sentence->maxWidth(120.f);
        chinese->add(std::move(sentence));
    }
    wrapRow->add(std::move(english), 1.f);
    wrapRow->add(std::move(chinese), 1.f);

    auto limited = column(6.f);
    limited->add(caption("maxLines(2)"));
    {
        auto paragraph = std::make_unique<Label>(
            "A capped block shows two lines and then an ellipsis, so a list of cards keeps the same height."
        );
        paragraph->wrap(true);
        paragraph->maxWidth(250.f);
        paragraph->maxLines(2);
        limited->add(std::move(paragraph));
    }

    auto body = column(8.f);
    body->add(std::move(grid));
    body->add(std::move(colorBlock));
    body->add(std::move(wrapRow));
    body->add(std::move(limited));
    return page("Text", "Label: sizes, alignment and word wrap.", std::move(body), back);
}
// ------------------------------------------------------------------- widget page

[[nodiscard]] std::unique_ptr<Widget> widgetPage(std::function<void()> back) {
    auto body = column(8.f);

    body->add(caption("onClick: the label toggles its own text"));
    {
        auto  toggled = std::make_shared<bool>(false);
        auto  label   = std::make_unique<Label>("Click me");
        auto* raw     = label.get();
        label->onClick([raw, toggled] {
            *toggled = !*toggled;
            raw->text(*toggled ? "Clicked!" : "Click me");
            log().debug("demo: base onClick fired, now '{}'", raw->text());
        });
        body->add(std::move(label));
    }

    body->add(caption("onFocus / onBlur: focus the input"));
    {
        auto input = std::make_unique<Input>();
        input->placeholder("Focus me");
        input->onFocus([] { log().debug("demo: input focused"); });
        input->onBlur([] { log().debug("demo: input blurred"); });
        body->add(std::move(input));
    }

    body->add(caption("fixedSize: the input does not fill the row"));
    {
        auto input = std::make_unique<Input>();
        input->placeholder("Narrow input");
        input->fixedSize(160.f, 19.f);
        body->add(std::move(input));
    }

    return page("Widget", "The base class: size constraints and events.", std::move(body), back);
}
// ------------------------------------------------------------------- switch page

[[nodiscard]] std::unique_ptr<Switch> makeSwitch(std::string text, bool checked = false) {
    auto switchWidget = std::make_unique<Switch>();
    switchWidget->text(std::move(text));
    switchWidget->checked(checked);
    return switchWidget;
}

[[nodiscard]] std::unique_ptr<Widget> switchPage(std::function<void()> back) {
    auto left = column(8.f);
    {
        auto basic = makeSwitch("Wi-Fi");
        basic->onCheckedChange([](bool value) { log().debug("demo: wifi={}", value); });
        left->add(std::move(basic));
        left->add(makeSwitch("Bluetooth", true));
        auto disabled = row(8.f);
        auto off      = makeSwitch("Off");
        off->disabled(true);
        auto on = makeSwitch("On", true);
        on->disabled(true);
        disabled->add(std::move(off));
        disabled->add(std::move(on));
        left->add(std::move(disabled));
    }

    auto right = column(8.f);
    {
        auto field = std::make_unique<Field>();
        field->label("Notifications");
        field->invalid(true);
        field->control(makeSwitch("Email me"));
        field->description("Field invalid is red.");
        right->add(std::move(field));
    }
    {
        auto field = std::make_unique<Field>();
        field->label("Airplane mode");
        field->required(true);
        field->control(makeSwitch("On"));
        right->add(std::move(field));
    }

    auto grid = row(10.f);
    grid->add(std::move(left), 1.f);
    grid->add(std::move(right), 1.f);

    auto body = column(8.f);
    body->add(std::move(grid));
    return page("Switch", "A track with a thumb; the colour shows the state.", std::move(body), back);
}
// --------------------------------------------------------------------- spec page

[[nodiscard]] std::unique_ptr<Widget> specPage(std::function<void()> back) {
    auto tree = bul::spec::build(
        bul::spec::Stack{
            .direction = bul::spec::Stack::Direction::Vertical,
            .gap       = 8.f,
            .children  = {
                          bul::spec::node(
                    bul::spec::Label{
                        .text  = "This page is a struct tree.",
                        .font  = {.size = 1.05f},
                        .muted = true,
                    }
                ), bul::spec::node(
                    bul::spec::Field{
                        .label       = "Name",
                        .description = "Field + Input, built by the spec.",
                        .control     = bul::spec::node(bul::spec::Input{.placeholder = "Ada Lovelace"}),
                    }
                ), bul::spec::node(bul::spec::Switch{.text = "Notifications", .checked = true}),
                          bul::spec::node(bul::spec::Checkbox{.text = "Accept terms"}),
                          bul::spec::node(
                    bul::spec::Button{
                        .text    = "Built by a spec",
                        .onClick = [] { log().debug("demo: spec button pressed"); },
                    }
                ), },
    }
    );

    auto body = column(8.f);
    body->add(std::move(tree));
    return page("Spec", "A tree built from structs, not make_unique chains.", std::move(body), back);
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
    view->add(checkboxPage(back));
    view->add(radioPage(back));
    view->add(textPage(back));
    view->add(widgetPage(back));
    view->add(switchPage(back));
    view->add(specPage(back));
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
