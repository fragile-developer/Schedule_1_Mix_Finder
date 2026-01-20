#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include "ftxui/component/event.hpp"

#include "data.h"
#include "product_tree.h"
#include "gui.h"

class SortButtonBase : public ftxui::ComponentBase {
private:
    const std::string label_;
    int field_id_;
    bool default_asc_;
    int& current_field_id_;
    bool& ascending_;

    ftxui::Box box_;
    bool hovered_ = false;

    bool OnMouseEvent(ftxui::Event event);
    void on_click();
public:
    SortButtonBase(const std::string label, int id, bool default_asc, int& current_field, bool& current_ascending);

    ftxui::Element OnRender() override;
    bool OnEvent(ftxui::Event event) override;
    bool Focusable() const final { return true; }

};

ftxui::Component SortButton(std::string label, int field_id, bool default_asc, int& current_field_id, bool& ascending);

enum FilterState : unsigned int {
    NONE        = 0,
    INCLUDE     = 1,
    EXCLUDE     = 2,
    BOTH        = 3
};
class FilterCheckBase : public ftxui::ComponentBase {
private:
    std::string label_;
    bool& include_;
    bool& exclude_;
    ftxui::Color color_;

    ftxui::Box box_;
    bool hovered_ = false;

    bool OnMouseEvent(ftxui::Event event);
    void rotateState();
    void setState(FilterState state);
    FilterState getState() const;
public:
    FilterCheckBase(std::string label, bool& include, bool& exclude, double mult, ftxui::Color color);

    double mult_;

    ftxui::Element OnRender() override;
    bool OnEvent(ftxui::Event event) override;
    bool Focusable() const final { return true; }
};

ftxui::Component FilterCheckbox(std::string label, bool& include, bool& exclude, double mult, ftxui::Color color);

class VerticalSliderBase : public ftxui::ComponentBase {
private:
    const int min_;
    int   max_;
    int&  current_val_;
    int   hover_val_;
    bool  hovered_;
    bool  gauge_hovered_;

    ftxui::Box box_;
    ftxui::Box gauge_box_;
    bool OnMouseEvent(ftxui::Event event);
    void increment_max(int increment);

    static const auto color_focus_   = ftxui::Color::BlueLight;
    static const auto color_hover_   = ftxui::Color::LightSkyBlue1;
    static const auto color_default_ = ftxui::Color::Default;
public:
    VerticalSliderBase(int min, int max, int& current_value);

    ftxui::Element OnRender() override;
    bool OnEvent(ftxui::Event event) override;
    bool Focusable() const final { return true; }
};

ftxui::Component VerticalSlider(int min, int max, int& current_value);

class ProductDetailBase : public ftxui::ComponentBase {
private:
    Product_Tree&       tree_;          // current product tree
    Product_Node*&      node_;          // node selected in table
    int&                result_count_;  // current filtered data count
    Product_Node*       current_node_;
    ftxui::Element      details_;

    ftxui::Component    hash_button_;
    bool                button_hover_;

public:
    ProductDetailBase(Product_Node*& detail_node, Product_Tree& tree_ref, int& result_count_ref);
    void update_details();

    ftxui::Element OnRender() override;
    bool OnEvent(ftxui::Event event) override;
    bool Focusable() const final { return false; }
};

ftxui::Component ProductDetail(Product_Node*& detail_node, Product_Tree& tree, int& result_count_ref);

class ProductTableBase : public ftxui::ComponentBase {
private:
    
    // table stuff
    ftxui::Component button_container_;
    std::vector<ftxui::Component> sort_buttons_;
    std::vector<ftxui::Element> headings_;
    std::vector<std::vector<ftxui::Element>> rows_;
    int selected_row_;

    // data stuff
    Query_State& query_state_;
    Query_State  old_state_;
    Product_Tree tree_;
    std::vector<Product_Node*> data_;

    // navigation stuff
    Product_Node* selected_node_;
    int result_count_;
    ftxui::Box box_;
    bool  hovered_;
    int   hovered_row_;

    bool OnMouseEvent(ftxui::Event event);

    void update_headings();
    void update_rows();

    void update_query();
    void update_filter();
    void update_sort();

public:
    ProductTableBase(Query_State& query_state);

    
    void update();

    ftxui::Component get_product_detail(); 

    ftxui::Element OnRender() override;
    bool OnEvent(ftxui::Event event) override;
    bool Focusable() const final { return true; }
};

ftxui::Component ProductTable(Query_State& query_state);