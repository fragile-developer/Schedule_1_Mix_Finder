#include "custom_components.h"

using namespace ftxui;
using std::vector, std::array, std::string;

FilterState operator+(FilterState state, int delta) {
    int raw = static_cast<int>(state);
    raw = (raw + delta) % 3;
    return static_cast<FilterState>(raw);
}

FilterCheckBase::FilterCheckBase(std::string label, bool& include, bool& exclude, double mult, ftxui::Color color)
    : include_(include), exclude_(exclude) {
        label_ = label;
        color_ = color;
        mult_  = mult;
}

void FilterCheckBase::rotateState() {
    FilterState state = getState();
    state = state + 1;
    setState(state);
}

void FilterCheckBase::setState(FilterState state) {
    include_ = (state & 0b01) > 0;
    exclude_ = (state & 0b10) > 0;
}

FilterState FilterCheckBase::getState() const {
    return static_cast<FilterState>((include_) + (exclude_ << 1));
}

Element FilterCheckBase::OnRender() {
    // static const string prefixes[] = {"[ ] ", "[✓] ", "[x] ", "[?] "}; 
    static const string prefixes[] = {"· ", "✓ ", "x ", "? "}; 
    auto prefix = text(prefixes[getState()]);
    
    auto label  = text(label_) | color(color_);
    if (include_) {
        prefix |= color(Color::Green);
        label  |= bold;
    }
    if (exclude_) {
        prefix |= color(Color::Red);
        label  |= dim;
        label  |= strikethrough;
    }
    if (!include_ && !exclude_) {
        prefix |= dim;
    }

    if (hovered_ || Focused())  label |= inverted;

    auto element = hbox(prefix, label) ;
    return element | reflect(box_);
}

bool FilterCheckBase::OnEvent(Event event) {
    if (!CaptureMouse(event)) return false;

    if (event.is_mouse()) return OnMouseEvent(event);

    hovered_ = false;
    if (event == Event::Character(' ') || event == Event::Return) {
        rotateState();
        TakeFocus();
        return true;
    }

    return false;
}

bool FilterCheckBase::OnMouseEvent(Event event) {
    hovered_ = box_.Contain(event.mouse().x, event.mouse().y);
    
    if (!CaptureMouse(event)) return false;

    if (!hovered_) return false;

    if (event.mouse().motion == Mouse::Pressed) {
        if (event.mouse().button == Mouse::Left) {
            if (include_) setState(FilterState::NONE);
            else          setState(FilterState::INCLUDE);
            return true;
        }
        if (event.mouse().button == Mouse::Right) {
            if (exclude_) setState(FilterState::NONE);
            else          setState(FilterState::EXCLUDE);
            return true;
        }
        if (event.mouse().button == Mouse::Middle) {
            setState(FilterState::NONE);
            return true;
        }
    }
    return false;
}

Component FilterCheckbox(std::string label, bool& include, bool& exclude, double mult, ftxui::Color color) {
    return Make<FilterCheckBase>(label, include, exclude, mult, color);
}