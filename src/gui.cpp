#include "gui.h"
#include "data.h"
#include "custom_components.h"
 
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;
using namespace std::views;
using std::vector, std::array, std::string;

const std::vector<Gui_Effect> gui_effects = std::ranges::to<vector<Gui_Effect>>(data::effects | transform([](Effect eff){return Gui_Effect(eff);}));

void run_gui(int drug_n, int max_ingredients, int max_rank) {
    using bool_r = unsigned char;

    // =================================== Data Model ===================================
    // this sets up all the states and interactive containers for the screen

    // Product Data
    Query_State current_state;

    // create the tree
    auto& tree_args         = current_state.tree_args;
    tree_args.drug_n        = drug_n;
    tree_args.ing_count     = max_ingredients;
    tree_args.rank          = max_rank;

    // Effect checkboxes
    vector<bool_r> sel_whitelist(data::size_effects, false); // char because bool vectors are not really bools
    vector<bool_r> sel_blacklist(data::size_effects, false);

    vector<Component> filter_checks;

    for (size_t i = 0; i < data::size_effects; ++i) {
        auto& eff = gui_effects[i];
        auto checkbox = FilterCheckbox(eff.name, (bool&)sel_whitelist[i], (bool&)sel_blacklist[i], eff.mult, eff.ui_color);
        filter_checks.push_back(checkbox);

        // load checkbox state from default mask (should usually be 0 but just in case)
        mask eff_mask = (mask)1 << i;
        if (eff_mask & current_state.filters.whitelist) sel_whitelist[i] = true;
        if (eff_mask & current_state.filters.blacklist) sel_blacklist[i] = true;
    }

    // sort checkboxes by multiplier
    auto get_mult = [](const Component& fcb){ return std::static_pointer_cast<FilterCheckBase>(fcb)->mult_; };
    std::sort(filter_checks.begin(), filter_checks.end(),
        [&](const Component& a, const Component& b) {return get_mult(a) > get_mult(b);});

    auto filters_box = Container::Vertical(filter_checks);
    
    // drop-downs
    const auto rank_labels = std::ranges::to<vector<string>>(data::rank_names);
    auto dd_rank = Dropdown(&rank_labels, &current_state.tree_args.rank);

    const auto drug_labels = std::ranges::to<vector<string>>(raw::base_products | transform([](raw::Base_Product bp){return bp.name;}));
    auto dd_drug = Dropdown(&drug_labels, &current_state.tree_args.drug_n);

    // Ing count slider
    const int slider_max = 20;
    auto v_slider = VerticalSlider(0, slider_max, current_state.tree_args.ing_count);
    
    // Product table
    auto product_table = static_pointer_cast<ProductTableBase>(ProductTable(current_state));    
    Component product_detail = product_table->get_product_detail();

    // =================================== Renderer ===================================
    // main render loop - captured in a lambda function.
    // this sets the final layout for the screen and handles updates of the data
    auto renderer = Renderer(
        Container::Vertical({
            Container::Horizontal({
                Container::Vertical({dd_drug, dd_rank,}),
                v_slider, 
                filters_box,
                product_table}),
            product_detail
        }),
    [&]() {
        // update filters
        current_state.filters.whitelist = 0;
        current_state.filters.blacklist = 0;
        for (size_t i = 0; i < data::size_effects; ++i) {        
            mask eff_mask = (mask)1 << i;
            if (sel_whitelist[i]) current_state.filters.whitelist |= eff_mask;
            if (sel_blacklist[i]) current_state.filters.blacklist |= eff_mask;
        }

        auto elem_query = vbox({
            text("Query Parameters") | bold | center,
            separator(),
            hbox({
                vbox({ 
                    text("Max Ingredients -> ") | bold | align_right,
                    separator(),
                    // dropdowns
                    vbox({
                        text(" Base Product") | xflex | bold, 
                        dd_drug -> Render() | size(WIDTH,  GREATER_THAN, 24),
                    }) | size(HEIGHT, GREATER_THAN, (int)drug_labels.size() + 5),
                    vbox({
                        text(" Max Rank") | xflex | bold, 
                        dd_rank -> Render()| size(WIDTH, GREATER_THAN, 20),
                    })
                }),
                separatorEmpty(),
                v_slider -> Render(),
            }) | yflex,
        });

        auto elem_filters = vbox({ 
            text("Filters") | bold | center,
            separator(),
            filters_box -> Render()
        });

        return vbox({
            hbox({
                elem_query   | border,
                elem_filters | border,
                product_table -> Render()
            }),
            product_detail->Render() | size(WIDTH, EQUAL, 108) // very lazy, will break on change
        });

    });

    // initialise screen and start render loop
    try {
        auto screen = ftxui::ScreenInteractive::Fullscreen();
        // auto screen = ftxui::ScreenInteractive::FitComponent();
        screen.Loop(renderer);
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << "\n";
    } catch (...) {
        std::cerr << "Unknown error occurred\n";
    }
}