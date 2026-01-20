#pragma once

#include "data.h"
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

enum Sort_Field : int {
    Ing_Count,
    Profit,
    Sell_Price,
    Cost,
};

struct Query_State {
    struct QS_Query {
        int drug_n      = 5;
        int ing_count   = 8;
        int rank        = data::size_ranks - 1;

        auto operator<=>(const QS_Query&) const = default;
        bool operator== (const QS_Query&) const = default;
    } tree_args;
    struct QS_Filter {
        mask whitelist  = 0;
        mask blacklist  = 0;

        auto operator<=>(const QS_Filter&) const = default;
        bool operator== (const QS_Filter&) const = default;
    } filters;
    struct QS_Sort {
        Sort_Field sort_field = Profit;
        bool       ascending  = false;

        auto operator<=>(const QS_Sort&) const = default;
        bool operator== (const QS_Sort&) const = default;
    } sorting;

    auto operator<=>(const Query_State&) const = default;
    bool operator== (const Query_State&) const = default;
};

struct Gui_Effect : Effect {
    ftxui::Color ui_color;

    Gui_Effect(const Effect& effect) : Effect(effect) {
        auto rgb  = RGBA(this->color);
        ui_color = ftxui::Color::RGB(rgb.r, rgb.g, rgb.b);
    }

    ftxui::Element get_element(bool abbr = false, bool colored = true) const {
        return ftxui::text(abbr ? abbreviation : name) | (colored ? ftxui::color(ui_color) : ftxui::nothing);
    }
};

extern const std::vector<Gui_Effect> gui_effects;

void run_gui(int drug_n, int max_ingredients, int max_rank);