#include "custom_components.h"

using namespace ftxui;
using namespace std::views;
using std::vector, std::string;

void open_url(const std::string& url) {
    #ifdef _WIN32
        std::string command = "start " + url;
        system(command.c_str());
    #elif __APPLE__
        std::string command = "open " + url;
        system(command.c_str());
    #elif __linux__
        std::string command = "xdg-open " + url;
        system(command.c_str());
    #else
        #error Unsupported platform
    #endif
}

auto make_button(string hash) {
    auto label  = "Hash: " + hash;
    auto url    = "https://schedule1.tools/mixer?mix=" + hash;

    return Button(label, [=](){open_url(url);}, ButtonOption::Ascii());
}

ProductDetailBase::ProductDetailBase(Product_Node*& detail_node, Product_Tree& tree_ref, int& result_count_ref) 
    : node_(detail_node), tree_(tree_ref), result_count_(result_count_ref) {
        update_details();
}

void ProductDetailBase::update_details() {
    current_node_ = node_;

    auto hash = string(node_ ? node_->get_product_hash(tree_) : "?");
    hash_button_ = Hoverable(make_button(hash), &button_hover_);

    if (!current_node_) {
        details_ = text("Nothing to Display") | border;
        return;
    }

    //details
    auto addiction = std::min(100, int(100*node_->product.get_addiction()));
    auto details = vector<std::pair<string, string>>({
        {"Ing Count: ",     std::to_string(node_->ingredient_count)},
        {"Profit: ",        std::to_string(node_->product.get_profit())},
        {"Sell Price: ",    std::to_string(node_->product.sell_price)},
        {"Cost: ",          std::to_string(node_->product.cost)},
        {"Addiction: ",     std::to_string(addiction) + "%"},
    });
    
    auto detail_elements = vector<Element>();
    for (const auto& detail :details) {
        detail_elements.push_back(text(detail.first) | bold);
        detail_elements.push_back(text(detail.second));
        detail_elements.push_back(separator());
    }

    // effects
    auto effect_elements =  vector<Element>();
    auto eff_inds = node_->product.get_effects_indices();
    for (auto i : eff_inds) {
        auto eff = gui_effects[i];
        effect_elements.push_back(eff.get_element());
        if (i != eff_inds.back()) effect_elements.push_back(text(", "));
    }

    //ingredients
    string ing_text = "";
    auto ing_list = node_->get_ingredients(tree_);
    for (auto& ing : ing_list) {
        auto name = raw::ingredients[ing->index].name;
        ing_text += name;
        if (&ing != &ing_list.back()) ing_text += " -> ";
    }

    details_ = vbox({ // Detail element
        separator(),
        hbox(detail_elements),
        separator(),
        hbox(effect_elements),
        separator(),
        paragraph(ing_text),
    });
}

Element ProductDetailBase::OnRender() {
    if (current_node_ != node_) update_details();

    // auto button = text("Hash: " + node_->get_product_hash(tree_)) | dim; // to-do: replace with clickable button render
    auto button = hash_button_->Render();

    return vbox({ // Detail element
        hbox({text("Top Result") | bold, text(" of " + std::to_string(result_count_)) , filler(), button_hover_ ? button | bold | underlined : button | dim}),
        this->details_
    }) | xflex | border;
}

bool ProductDetailBase::OnEvent(ftxui::Event event) {
    return this->hash_button_->OnEvent(event);
    return false;
}

Component ProductDetail(Product_Node*& detail_node, Product_Tree& tree, int& result_count_ref) {
    return Make<ProductDetailBase>(detail_node, tree, result_count_ref);
}