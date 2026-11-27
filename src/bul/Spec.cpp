#include "bul/Spec.h"

#include <utility>

namespace bul::spec {

// The field-to-setter mapping lives on the widget itself (`X::apply(node)`), so
// this file is only the thin bridge between a node and its widget: every widget
// owns the one function that configures it, and the spec layer calls it.

std::unique_ptr<Widget> Label::buildContent() const {
    auto widget = std::make_unique<bul::Label>("");
    widget->apply(*this);
    return widget;
}

std::unique_ptr<Widget> Button::buildContent() const {
    auto widget = std::make_unique<bul::Button>(text, onClick);
    widget->apply(*this);
    return widget;
}

std::unique_ptr<Widget> Badge::buildContent() const { return std::make_unique<bul::Badge>(text, variant); }

std::unique_ptr<Widget> Input::buildContent() const {
    auto widget = std::make_unique<bul::Input>(value);
    widget->apply(*this);
    return widget;
}

std::unique_ptr<Widget> Field::buildContent() const {
    auto widget = std::make_unique<bul::Field>();
    widget->apply(*this);
    return widget;
}

std::unique_ptr<Widget> Stack::buildContent() const {
    auto widget = std::make_unique<bul::Stack>(direction);
    widget->apply(*this);
    return widget;
}

std::unique_ptr<Widget> Select::buildContent() const {
    auto widget = std::make_unique<bul::Select>();
    widget->apply(*this);
    return widget;
}

std::unique_ptr<Widget> Checkbox::buildContent() const {
    auto widget = std::make_unique<bul::Checkbox>();
    widget->apply(*this);
    return widget;
}

std::unique_ptr<Widget> Switch::buildContent() const {
    auto widget = std::make_unique<bul::Switch>();
    widget->apply(*this);
    return widget;
}

std::unique_ptr<Widget> RadioGroup::buildContent() const {
    auto widget = std::make_unique<bul::RadioGroup>();
    widget->apply(*this);
    return widget;
}

std::unique_ptr<Widget> Grid::buildContent() const {
    auto widget = std::make_unique<bul::Grid>(columns);
    widget->apply(*this);
    return widget;
}

std::unique_ptr<Widget> Table::buildContent() const {
    auto widget = std::make_unique<bul::Table>();
    widget->apply(*this);
    return widget;
}

std::unique_ptr<Widget> ScrollView::buildContent() const {
    auto widget = std::make_unique<bul::ScrollView>();
    widget->apply(*this);
    return widget;
}

std::unique_ptr<Widget> FieldLabel::buildContent() const {
    auto widget = std::make_unique<bul::FieldLabel>(text, scale);
    widget->apply(*this);
    return widget;
}

std::unique_ptr<Widget> FieldDescription::buildContent() const {
    return std::make_unique<bul::FieldDescription>(text, scale);
}

} // namespace bul::spec
