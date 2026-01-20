#include "custom_components.h"

using namespace ftxui;
using std::vector, std::array, std::string;

SortButtonBase::SortButtonBase(std::string label, int field_id, bool default_asc, int& current_field_id, bool& ascending)
    : label_(label), current_field_id_(current_field_id), ascending_(ascending) {
        field_id_ = field_id;
        default_asc_ = default_asc;
    };

Element SortButtonBase::OnRender() {
    auto sort_icon = (field_id_ != current_field_id_) ? "" : ascending_ ? "▲ " : "▼ ";

    Element element = text(sort_icon + label_) | bold | center;
    element |= size(WIDTH, GREATER_THAN, (int)label_.length() + 2);

    if (hovered_)  element |= inverted;
    if (Focused()) element |= color(ftxui::Color::BlueLight);

    return element | reflect(box_);
}

void SortButtonBase::on_click() {
    if (field_id_ != current_field_id_) {
        current_field_id_ = field_id_;
        ascending_ = default_asc_;
    }
    else {
        ascending_ = !ascending_;
    }
}

bool SortButtonBase::OnEvent(Event event) {
    if (!CaptureMouse(event)) return false;

    if (event.is_mouse()) return OnMouseEvent(event);

    hovered_ = false;
    if (event == Event::Character(' ') || event == Event::Return) {
      on_click();
      TakeFocus();
      return true;
    }

    return false;
}

bool SortButtonBase::OnMouseEvent(Event event) {
    hovered_ = box_.Contain(event.mouse().x, event.mouse().y);
    
    if (!CaptureMouse(event)) return false;

    if (!hovered_) return false;

    if (event.mouse().button == Mouse::Left &&
        event.mouse().motion == Mouse::Pressed) {
        on_click();
        return true;
    }

    return false;
    }

ftxui::Component SortButton(std::string label, int field_id, bool default_asc, int& current_field_id, bool& ascending) {
    return Make<SortButtonBase>(label, field_id, default_asc, current_field_id, ascending);
}