#include "bul/SpecBuilder.h"

#include <utility>

namespace bul::spec {

Builder::Builder() : mRoot(std::make_unique<bul::Stack>(bul::Stack::Direction::Vertical)) {
    bul::Stack* root = mRoot.get();
    mSinks.push_back([root](std::unique_ptr<Widget> widget, float stretch) { root->add(std::move(widget), stretch); });
}

void Builder::emit(std::unique_ptr<Widget> widget, float stretch) {
    if (!widget) {
        return;
    }
    std::string id;
    if (mPendingSet) {
        id          = mPending.id;
        widget      = detail::finish(std::move(widget), mPending);
        mPending    = Common{};
        mPendingSet = false;
    }
    if (!id.empty()) {
        mById.emplace_back(std::move(id), widget.get());
    }
    mLast = widget.get();
    mSinks.back()(std::move(widget), stretch);
}

void Builder::scoped(Sink sink, std::function<void()> const& body) {
    mSinks.push_back(std::move(sink));
    body();
    mSinks.pop_back();
}

Builder& Builder::with(Common const& common) {
    mPending    = common;
    mPendingSet = true;
    return *this;
}

Builder& Builder::id(std::string value) {
    mPending.id = std::move(value);
    mPendingSet = true;
    return *this;
}

Widget* Builder::find(std::string_view id) const {
    for (auto const& [key, widget] : mById) {
        if (key == id) {
            return widget;
        }
    }
    return nullptr;
}

Builder& Builder::row(std::function<void()> body, std::function<void(bul::Stack&)> configure) {
    auto stack = std::make_unique<bul::Stack>(bul::Stack::Direction::Horizontal);
    if (configure) {
        configure(*stack);
    }
    auto* raw = stack.get();
    emit(std::move(stack));
    scoped([raw](std::unique_ptr<Widget> child, float stretch) { raw->add(std::move(child), stretch); }, body);
    return *this;
}

Builder& Builder::column(std::function<void()> body, std::function<void(bul::Stack&)> configure) {
    auto stack = std::make_unique<bul::Stack>(bul::Stack::Direction::Vertical);
    if (configure) {
        configure(*stack);
    }
    auto* raw = stack.get();
    emit(std::move(stack));
    scoped([raw](std::unique_ptr<Widget> child, float stretch) { raw->add(std::move(child), stretch); }, body);
    return *this;
}

Builder& Builder::grid(std::size_t columns, std::function<void()> body, std::function<void(bul::Grid&)> configure) {
    auto grid = std::make_unique<bul::Grid>(columns);
    if (configure) {
        configure(*grid);
    }
    auto* raw = grid.get();
    emit(std::move(grid));
    scoped([raw](std::unique_ptr<Widget> child, float) { raw->add(std::move(child)); }, body);
    return *this;
}

Builder& Builder::scroll(float viewport, std::function<void()> body, std::function<void(bul::ScrollView&)> configure) {
    auto  content = std::make_unique<bul::Stack>(bul::Stack::Direction::Vertical);
    auto* raw     = content.get();
    scoped([raw](std::unique_ptr<Widget> child, float stretch) { raw->add(std::move(child), stretch); }, body);

    auto view = std::make_unique<bul::ScrollView>();
    view->viewport(viewport);
    view->content(std::move(content));
    if (configure) {
        configure(*view);
    }
    emit(std::move(view));
    return *this;
}

Builder& Builder::table(std::function<void(bul::Table&)> configure) {
    auto table = std::make_unique<bul::Table>();
    if (configure) {
        configure(*table);
    }
    emit(std::move(table));
    return *this;
}

Builder& Builder::field(
    std::string                      labelText,
    std::function<void()>            body,
    std::string                      description,
    std::function<void(bul::Field&)> configure
) {
    auto  control = std::make_unique<bul::Stack>(bul::Stack::Direction::Vertical);
    auto* raw     = control.get();
    scoped([raw](std::unique_ptr<Widget> child, float stretch) { raw->add(std::move(child), stretch); }, body);

    auto field = std::make_unique<bul::Field>();
    field->label(std::move(labelText));
    if (!description.empty()) {
        field->description(std::move(description));
    }
    field->control(std::move(control));
    if (configure) {
        configure(*field);
    }
    emit(std::move(field));
    return *this;
}

Builder& Builder::add(std::unique_ptr<Widget> widget, float stretch) {
    emit(std::move(widget), stretch);
    return *this;
}
Builder& Builder::label(std::string text, float size, bool muted, std::function<void(bul::Label&)> configure) {
    auto widget = std::make_unique<bul::Label>(std::move(text), size);
    if (muted) {
        widget->muted();
    }
    if (configure) {
        configure(*widget);
    }
    emit(std::move(widget));
    return *this;
}

Builder& Builder::button(std::string text, std::function<void()> onClick, std::function<void(bul::Button&)> configure) {
    auto widget = std::make_unique<bul::Button>(std::move(text), std::move(onClick));
    if (configure) {
        configure(*widget);
    }
    emit(std::move(widget));
    return *this;
}

Builder& Builder::badge(std::string text, std::function<void(bul::Badge&)> configure) {
    auto widget = std::make_unique<bul::Badge>(std::move(text));
    if (configure) {
        configure(*widget);
    }
    emit(std::move(widget));
    return *this;
}

Builder& Builder::input(std::string placeholder, std::string value, std::function<void(bul::Input&)> configure) {
    auto widget = std::make_unique<bul::Input>(std::move(value));
    if (!placeholder.empty()) {
        widget->placeholder(std::move(placeholder));
    }
    if (configure) {
        configure(*widget);
    }
    emit(std::move(widget));
    return *this;
}

Builder& Builder::checkbox(std::string text, bool checked, std::function<void(bul::Checkbox&)> configure) {
    auto widget = std::make_unique<bul::Checkbox>();
    widget->text(std::move(text));
    widget->checked(checked);
    if (configure) {
        configure(*widget);
    }
    emit(std::move(widget));
    return *this;
}

Builder& Builder::toggle(std::string text, bool checked, std::function<void(bul::Switch&)> configure) {
    auto widget = std::make_unique<bul::Switch>();
    widget->text(std::move(text));
    widget->checked(checked);
    if (configure) {
        configure(*widget);
    }
    emit(std::move(widget));
    return *this;
}

Builder& Builder::select(
    std::string                          placeholder,
    std::vector<bul::spec::Select::Item> items,
    std::function<void(bul::Select&)>    configure
) {
    auto widget = std::make_unique<bul::Select>();
    widget->placeholder(std::move(placeholder));
    for (auto const& entry : items) {
        widget->item(entry.value, entry.label.empty() ? entry.value : entry.label, entry.disabled);
    }
    if (configure) {
        configure(*widget);
    }
    emit(std::move(widget));
    return *this;
}

Builder& Builder::radio(
    std::vector<bul::spec::RadioGroup::Option> options,
    std::string                                value,
    std::function<void(bul::RadioGroup&)>      configure
) {
    auto group = std::make_unique<bul::RadioGroup>();
    for (auto const& option : options) {
        group->add(option.value, option.text.empty() ? option.value : option.text, option.disabled);
    }
    if (!value.empty()) {
        group->value(std::move(value));
    }
    if (configure) {
        configure(*group);
    }
    emit(std::move(group));
    return *this;
}

Builder& Builder::fieldLabel(std::string text, float scale, bool required, bool muted) {
    auto label = std::make_unique<bul::FieldLabel>(std::move(text), scale);
    label->required(required);
    label->muted(muted);
    emit(std::move(label));
    return *this;
}

Builder& Builder::fieldDescription(std::string text, float scale) {
    emit(std::make_unique<bul::FieldDescription>(std::move(text), scale));
    return *this;
}

Builder& Builder::margin(float value) {
    if (mLast) {
        mLast->margin(value);
    }
    return *this;
}

Builder& Builder::fixedSize(float width, float height) {
    if (mLast) {
        mLast->fixedSize(width, height);
    }
    return *this;
}

Builder& Builder::minSize(float width, float height) {
    if (mLast) {
        mLast->minSize(width, height);
    }
    return *this;
}

Builder& Builder::maxSize(float width, float height) {
    if (mLast) {
        mLast->maxSize(width, height);
    }
    return *this;
}

Builder& Builder::className(std::string value) {
    if (mLast) {
        mLast->className(std::move(value));
    }
    return *this;
}

Builder& Builder::style(Style value) {
    if (mLast) {
        mLast->style(std::move(value));
    }
    return *this;
}

Builder& Builder::onClick(std::function<void()> handler) {
    if (mLast) {
        mLast->onClick(std::move(handler));
    }
    return *this;
}

std::unique_ptr<Widget> Builder::build() { return take(); }
std::unique_ptr<Widget> Builder::take() { return std::move(mRoot); }

} // namespace bul::spec
