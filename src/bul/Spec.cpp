#include "bul/Spec.h"

#include <utility>

namespace bul::spec {

std::unique_ptr<Widget> Label::buildContent() const {
    auto label = std::make_unique<bul::Label>(text, font.size, align);
    label->verticalAlign(verticalAlign);
    label->wrap(wrap);
    if (wrapWidth > 0.f) {
        label->maxWidth(wrapWidth);
    }
    if (maxLines > 0) {
        label->maxLines(maxLines);
    }
    if (muted) {
        label->muted();
    }
    if (font.color) {
        label->color(*font.color);
    }
    return label;
}

std::unique_ptr<Widget> Button::buildContent() const {
    auto button = std::make_unique<bul::Button>(text, onClick);
    button->type(type);
    button->size(size);
    button->disabled(disabled);
    return button;
}

std::unique_ptr<Widget> Badge::buildContent() const {
    auto badge = std::make_unique<bul::Badge>(text, variant);
    return badge;
}

std::unique_ptr<Widget> Input::buildContent() const {
    auto input = std::make_unique<bul::Input>(value);
    input->placeholder(placeholder);
    input->kind(kind);
    input->disabled(disabled);
    input->invalid(invalid);
    input->required(required);
    if (maxLength > 0) {
        input->maxLength(maxLength);
    }
    if (onChange) {
        input->onChange(onChange);
    }
    if (onSubmit) {
        input->onSubmit(onSubmit);
    }
    return input;
}

std::unique_ptr<Widget> Field::buildContent() const {
    auto field = std::make_unique<bul::Field>();
    if (!label.empty()) {
        field->label(label);
    }
    if (control) {
        field->control(control->build());
    }
    if (!description.empty()) {
        field->description(description);
    }
    field->orientation(orientation);
    if (gap >= 0.f) {
        field->gap(gap);
    }
    field->disabled(disabled);
    field->invalid(invalid);
    field->required(required);
    return field;
}

std::unique_ptr<Widget> Stack::buildContent() const {
    auto stack = std::make_unique<bul::Stack>(direction);
    if (gap >= 0.f) {
        stack->gap(gap);
    }
    for (auto const& child : children) {
        if (child) {
            stack->add(child->build());
        }
    }
    return stack;
}

std::unique_ptr<Widget> Select::buildContent() const {
    auto select = std::make_unique<bul::Select>();
    if (!placeholder.empty()) {
        select->placeholder(placeholder);
    }
    for (auto const& item : items) {
        select->item(item.value, item.label.empty() ? item.value : item.label, item.disabled);
    }
    if (!value.empty()) {
        select->value(value);
    }
    select->disabled(disabled);
    select->required(required);
    if (onValueChange) {
        select->onValueChange(onValueChange);
    }
    return select;
}

std::unique_ptr<Widget> Checkbox::buildContent() const {
    auto box = std::make_unique<bul::Checkbox>();
    if (!text.empty()) {
        box->text(text);
    }
    box->checked(checked);
    box->indeterminate(indeterminate);
    box->readOnly(readOnly);
    box->disabled(disabled);
    box->invalid(invalid);
    if (required) {
        box->required(true);
    }
    if (onCheckedChange) {
        box->onCheckedChange(onCheckedChange);
    }
    return box;
}

std::unique_ptr<Widget> Switch::buildContent() const {
    auto switchWidget = std::make_unique<bul::Switch>();
    if (!text.empty()) {
        switchWidget->text(text);
    }
    switchWidget->checked(checked);
    switchWidget->readOnly(readOnly);
    switchWidget->disabled(disabled);
    switchWidget->invalid(invalid);
    if (required) {
        switchWidget->required(true);
    }
    if (onCheckedChange) {
        switchWidget->onCheckedChange(onCheckedChange);
    }
    return switchWidget;
}

std::unique_ptr<Widget> RadioGroup::buildContent() const {
    auto group = std::make_unique<bul::RadioGroup>();
    group->direction(direction);
    if (gap >= 0.f) {
        group->gap(gap);
    }
    for (auto const& option : options) {
        group->add(option.value, option.text.empty() ? option.value : option.text, option.disabled);
    }
    if (!value.empty()) {
        group->value(value);
    }
    group->disabled(disabled);
    group->readOnly(readOnly);
    group->invalid(invalid);
    group->required(required);
    if (onValueChange) {
        group->onValueChange(onValueChange);
    }
    return group;
}

std::unique_ptr<Widget> Grid::buildContent() const {
    auto grid = std::make_unique<bul::Grid>(columns);
    if (gap >= 0.f) {
        grid->gap(gap);
    }
    for (auto const& child : children) {
        if (auto built = child->build()) {
            grid->add(std::move(built));
        }
    }
    return grid;
}

std::unique_ptr<Widget> Table::buildContent() const {
    auto table = std::make_unique<bul::Table>();
    for (auto const& track : columns) {
        table->column(track);
    }
    for (auto const& track : rows) {
        table->row(track);
    }
    table->gap(columnGap, rowGap);
    table->padding(padding);
    table->cellPadding(cellPadding);
    table->border(border);
    if (borderColor) {
        table->borderColor(*borderColor);
    }
    for (auto const& cell : cells) {
        if (cell.control) {
            if (auto built = cell.control->build()) {
                table->cell(cell.row, cell.column, std::move(built));
            }
        }
    }
    return table;
}

std::unique_ptr<Widget> ScrollView::buildContent() const {
    auto view = std::make_unique<bul::ScrollView>();
    view->viewport(viewport);
    view->scrollbar(scrollbar);
    if (content) {
        if (auto built = content->build()) {
            view->content(std::move(built));
        }
    }
    return view;
}
std::unique_ptr<Widget> FieldLabel::buildContent() const {
    auto label = std::make_unique<bul::FieldLabel>(text, scale);
    label->required(required);
    label->muted(muted);
    if (badge) {
        if (auto built = badge->build()) {
            label->badge(std::move(built));
        }
    }
    return label;
}

std::unique_ptr<Widget> FieldDescription::buildContent() const {
    return std::make_unique<bul::FieldDescription>(text, scale);
}

} // namespace bul::spec
