#include "custom_components.h"


using namespace ftxui;
using std::vector, std::array, std::string;

// Possible ruler ticks "_⎽⎼⎻⎺‾ ⎯"

VerticalSliderBase::VerticalSliderBase(int min, int max, int& current_value) 
    : min_(min), current_val_(current_value) {
        max_ = max;
        hover_val_ = current_val_;
        hovered_ = false;
        gauge_hovered_ = false;
        if (current_val_ > max_) max_ = current_val_; // or clamp, or allow initialising out of bounds?
}

Element VerticalSliderBase::OnRender() {
    const bool focused = Focused();

    if (!hovered_ && !Focused()) hover_val_ = current_val_; // mutating on render.. probably bad practice

    auto to_percent = [&](int val){return std::clamp(float(val - min_) / float(max_ - min_), 0.0f, 1.0f);};
    
    float percent       = to_percent(current_val_);
    float hover_percent = to_percent(hover_val_);
    
    Color alt_color = color_default_;
    if (focused)  alt_color = color_focus_;
    if (hovered_ || current_val_ != hover_val_) alt_color = color_hover_;

    const auto gauge_height = gauge_box_.y_max - gauge_box_.y_min;
    auto val_height = std::min(gauge_height, int(hover_percent * (gauge_height)));
    bool is_min = hover_val_ == min_;

    return vbox({
        hbox({
            gaugeDirection(hover_percent, Direction::Up) | color(alt_color),
            vbox({ // sketechy way to put value marker at the correct height
                hover_val_ >= max_ ? emptyElement() : text(std::to_string(max_)),
                filler(),
                text(std::to_string(hover_val_)) | size(WIDTH, GREATER_THAN, 2) | color(alt_color),
                filler() | size(HEIGHT, EQUAL, val_height-!is_min),
                is_min ? emptyElement() : text(std::to_string(min_))
            })
        }) | yflex | reflect(gauge_box_),
    }) | reflect(box_);
}

bool VerticalSliderBase::OnEvent(Event event) {
    if (!CaptureMouse(event)) return false;
    
    if (event.is_mouse()) return OnMouseEvent(event);

    if (event == Event::Character(' ') || event == Event::Return) {
        if (this->Focused()) {
            current_val_ = hover_val_;
        }
        else {
            TakeFocus();
        }
        return true;
    }
    if (this->Focused()) {
        if (event == Event::ArrowUp) {
            hover_val_ = std::min(this->max_, hover_val_ + 1);
            return true;
        }
        if (event == Event::ArrowDown) {
            hover_val_ = std::max(this->min_, hover_val_ - 1);
            return true;
        }
        if (event == Event::PageUp) {
            increment_max(+4);
            return true;
        }
        if (event == Event::PageDown) {
            increment_max(-4);
            return true;
        }
    }

    return false;
}

bool VerticalSliderBase::OnMouseEvent(Event event) {

    hovered_ = box_.Contain(event.mouse().x, event.mouse().y);

    if (!CaptureMouse(event)) return false;

    if (!hovered_) return false;

    gauge_hovered_ = gauge_box_.Contain(event.mouse().x, event.mouse().y);

    auto unbound = float(gauge_box_.y_max - event.mouse().y); // y increases downward
    unbound /= float(gauge_box_.y_max - gauge_box_.y_min);
    unbound *= float(max_ - min_);
    hover_val_ = std::clamp(int(unbound), min_, max_);

    if (event.mouse().button == Mouse::Left &&
        event.mouse().motion == Mouse::Pressed) {
        current_val_ = hover_val_;
        return true;
    }
    if (event.mouse().button == Mouse::WheelUp) {
        increment_max(+4);
        return true;
    }
    if (event.mouse().button == Mouse::WheelDown) {
        increment_max(-4);
        return true;
    }

    return false;
    }

void VerticalSliderBase::increment_max(int increment) {
    this->max_       = std::clamp(this->max_ + increment, 4, 48);
    this->hover_val_ = std::clamp(this->hover_val_ + increment, 0, this->max_);
}

Component VerticalSlider(int min, int max, int& current_value) {
    return Make<VerticalSliderBase>(min, max, current_value);
}